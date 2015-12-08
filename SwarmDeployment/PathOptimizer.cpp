#include "PathOptimizer.h"
#include "Random.h"
#include "Dubins/geom/geom.h"
#include "Configuration.h"
#include "PathHandler.h"

using namespace geom;

namespace App
{


	PathOptimizer::PathOptimizer(shared_ptr<DistanceResolver> distanceResolver, shared_ptr<Configuration> configuration, 
		shared_ptr<CarLikeMotionModel> motionModel, shared_ptr<CollisionDetector> collisionDetector, 
		shared_ptr<LoggerInterface> logger) :
		distanceResolver(distanceResolver), configuration(configuration), motionModel(motionModel), collisionDetector(collisionDetector), 
		logger(logger)
	{
	}


	PathOptimizer::~PathOptimizer()
	{
	}

	//optimalizuje cestu pomoc� Dubinsov�ch man�vr�
	vector<shared_ptr<State>> PathOptimizer::optimizePath(vector<shared_ptr<State>> path, shared_ptr<Map> map)
	{
		double pathLength = distanceResolver->getLengthOfPath(path);

		shared_ptr<LinkedState> endOfPath = path[path.size() - 1]; //�pln� posledn� prvek cel� cesty, c�l
		int stopLimit = 100;	//kolikr�t za sebou se nesm� aplikov�n� Dubinse zlep�it trajektorie, aby se algoritmus zastavil
		int notImprovedCount = 0;

		while (notImprovedCount < stopLimit)
		{
			auto startIndex = Random::index(path);	//indexy jsou kv�li jednodu���mu pohybu v poli
			auto endIndex = Random::index(path);
			while (startIndex == endIndex)
			{
				endIndex = Random::index(path);
			}
			if (startIndex > endIndex)
			{
				//swap
				auto temp = endIndex;
				endIndex = startIndex;
				startIndex = temp;
			}
			auto start = path[startIndex];
			auto endOriginal = path[endIndex];

			auto end = make_shared<LinkedState>(*endOriginal.get());	//kv�li prohazov�n� k��ej�c�ch se UAV vytvo��m kopii, kterou modifikuji. A� kdy� zjist�m, �e je nov� trajektorie v po��dku, ulo��m to do p�vodn�ho stavu

			auto pair = optimizePathBetween(start, end, map);
			bool isPathChanged = pair.second;
			auto trajectoryPart = pair.first;

			if (isPathChanged)
			{
				trajectoryPart[0]->setPrevious(start);	//navazuji za��tek nov� trajektorie na p�edchoz ��st trasy
				endOfPath = trajectoryPart[trajectoryPart.size() - 1];
				bool isEndOfPath = *endOriginal.get() == *endOfPath.get();	//zda je end koncem cel� cesty
				if (isEndOfPath)
				{
					endOfPath = end;	//posledn� prvek trajectoryPart
				}
				else
				{
					shared_ptr<LinkedState> afterEnd = path[endIndex + 1];
					afterEnd->setPrevious(end);

					//p�epo��t�n� �as� na �seku za optimalizovanou ��st�
					double previousTime = end->getTime();
					for (size_t i = endIndex + 1; i < path.size(); i++)
					{
						previousTime += configuration->getEndTime();
						path[i]->setTime(previousTime);
					}
				}
				path = PathHandler::getPath(endOfPath);
				if (distanceResolver->getLengthOfPath(path) < pathLength)
				{
					notImprovedCount = 0;
					pathLength = distanceResolver->getLengthOfPath(path);
				} else
				{
					notImprovedCount++;
				}
			} else
			{
				notImprovedCount++;
			}
		}
		return PathHandler::getPath(endOfPath);
	}

	//bool ��k�, zda se cesta zm�nila
	pair<vector<shared_ptr<State>>, bool> PathOptimizer::optimizePathBetween(shared_ptr<State> start, shared_ptr<State> end, shared_ptr<Map> map)
	{
		straightenCrossingTrajectories(start, end);	//pokud se k��� trajektorie, pak nemohu optimalizovat

		double maxSpeed = configuration->getDistanceOfNewNodes();	//tak� reprezentuje po�et pixel�, kter� v car like modelu uraz� uav za sekundu
		//p�edpo��t�m si d�lky v�ech dubins� p�edem
		unordered_map<Uav, pair<Dubins, bool>, UavHasher> dubinsTrajectories = unordered_map<Uav, pair<Dubins, bool>, UavHasher>();	//��k�, zda je dubins krat�� ne� p�vodn� trajektorie nebo ne
		bool areAllDubinsTrajectoriesLonger = true;
		for (auto uav : end->getUavs())
		{
			double length = distanceResolver->getLengthOfPath(start, end, uav);

			auto dubins = Dubins(start->getUav(uav)->getPointParticle()->toPosition(), uav->getPointParticle()->toPosition(), motionModel->getMinimalCurveRadius());
			double newLength = dubins.getLength();	//vrac� d�lku cel�ho man�vru
			bool isDubinsShorter = newLength < length;
			dubinsTrajectories[*uav.get()] = make_pair(dubins, isDubinsShorter);
			areAllDubinsTrajectoriesLonger = areAllDubinsTrajectoriesLonger && !isDubinsShorter;
		}

		logger->logDubinsPaths(dubinsTrajectories);

		if (areAllDubinsTrajectoriesLonger)
		{
			return make_pair(PathHandler::getPath(start, end), false); // p�vodn� cesta
		}

		//nalezen� nejdel��ho dubbinse ze v�ech, kter� jsou krat�� ne� p�vodn� trajektorie, podle n�j se bude diskretizovat
		int largestStepCount = 0;
		for (auto uav : end->getUavs())
		{
			auto pair = dubinsTrajectories[*uav.get()];
			auto dubins = pair.first;
			auto isDubinsShorter = pair.second;
			double newLength = dubins.getLength();	//vrac� d�lku cel�ho man�vru
			double totalTime = newLength / maxSpeed;	//doba cesty
			int stepCount = totalTime / configuration->getEndTime();	//po�et krok�, doba cel�ho man�vru / doba jednoho kroku

			if (isDubinsShorter && stepCount > largestStepCount)
			{
				largestStepCount = stepCount;
			}
		}

		if (largestStepCount == 0)	//tak mal� cesta, �e je men�� ne� krok simulace
		{
			return make_pair(PathHandler::getPath(start, end), false); // p�vodn� cesta
		}

		//zde provedu diskretizaci postupn� pro v�echna uav najednou, kus po kusu a p�itom budu kontrolovat v�echny podm�nky
		int stepCount = largestStepCount;
		vector<shared_ptr<LinkedState>> newTrajectory = vector<shared_ptr<LinkedState>>(stepCount);
		auto previousState = start;
		for (size_t i = 0; i < stepCount; i++)
		{
			auto newState = make_shared<LinkedState>(*previousState.get());
			newState->setPrevious(previousState);
			newState->incrementTime(configuration->getEndTime());
			double distanceCompleted = i * configuration->getDistanceOfNewNodes();	//ura�en� cesta v dubinsov� man�vru

			unordered_map<Uav, shared_ptr<CarLikeControl>, UavHasher> inputs = unordered_map<Uav, shared_ptr<CarLikeControl>, UavHasher>();
			//zde se pro ka�d� UAV vybere podle typu Dubinsova man�vru vhodn� input pro motion model
			for (auto uav : newState->getUavs())
			{
				auto pair = dubinsTrajectories[*uav.get()];
				auto dubins = pair.first;
				auto isDubinsShorter = pair.second;
				if (isDubinsShorter)
				{
					if (distanceCompleted + configuration->getDistanceOfNewNodes() < dubins.getLength())
					{
						auto newPosition = dubins.getPosition(distanceCompleted + configuration->getDistanceOfNewNodes());

						uav->getPointParticle()->getLocation()->setX(newPosition.getPoint().getX());
						uav->getPointParticle()->getLocation()->setY(newPosition.getPoint().getY());
						uav->getPointParticle()->getRotation()->setZ(newPosition.getAngle());
					}
					else
					{	//uav, kter� u� dorazilo do c�le, "po�k�" na ostatn�
						uav->getPointParticle()->setLocation(end->getUav(uav)->getPointParticle()->getLocation());
						uav->getPointParticle()->setRotation(end->getUav(uav)->getPointParticle()->getRotation());
					}
				} else
				{
					shared_ptr<LinkedState> currentOldState = end;
					while (currentOldState->getTime() > newState->getTime())
					{
						currentOldState = currentOldState->getPrevious();
					}
					//pokud je krat�� p�vodn� cesta pro dan� uav, bere se pro dan� uav p�vodn� cesta
					uav->getPointParticle()->setLocation(currentOldState->getUav(uav)->getPointParticle()->getLocation());
					uav->getPointParticle()->setRotation(currentOldState->getUav(uav)->getPointParticle()->getRotation());
				}

			}

			//validace
			if (!collisionDetector->isStateValid(start, end, map))
			{
				return make_pair(PathHandler::getPath(start, end), false); // p�vodn� cesta
			}

			previousState = newState;
			newTrajectory[i] = newState;
//			logger->logNewState(previousState, newState, true);
		}

		auto lastState = newTrajectory[newTrajectory.size() - 1];
		end->setPrevious(lastState);
		end->setTime(lastState->getTime() + configuration->getEndTime());
		newTrajectory.push_back(end);

		double newLength = distanceResolver->getLengthOfPath(newTrajectory[0], end);

		return make_pair(newTrajectory, true);
	}

	void PathOptimizer::setLogger(const shared_ptr<LoggerInterface> logger_interface)
	{
		logger = logger_interface;
	}

	void PathOptimizer::straightenCrossingTrajectories(shared_ptr<LinkedState> start, shared_ptr<LinkedState> end)
	{
		bool intersecting = collisionDetector->areTrajectoriesIntersecting(start, end);
		while (intersecting)
		{
			auto uavs = collisionDetector->getIntersectingUavs(start, end);
			//swap intersecting uavs
			end->swapUavs(uavs.first, uavs.second);
			intersecting = collisionDetector->areTrajectoriesIntersecting(start, end);
		}
	}

}
