#include "PathOptimizer.h"
#include "Random.h"
#include "Dubins/geom/geom.h"
#include "Configuration.h"
#include "PathHandler.h"

using namespace geom;

namespace App
{


	PathOptimizer::PathOptimizer(shared_ptr<DistanceResolver> distanceResolver, shared_ptr<Configuration> configuration, 
		shared_ptr<MotionModel> motionModel, shared_ptr<CollisionDetector> collisionDetector, 
		shared_ptr<LoggerInterface> logger) :
		distanceResolver(distanceResolver), configuration(configuration), motionModel(motionModel), collisionDetector(collisionDetector), 
		logger(logger)
	{
	}


	PathOptimizer::~PathOptimizer()
	{
	}

	//optimalizuje cestu pomoc� Dubinsov�ch man�vr�
	vector<shared_ptr<State>> PathOptimizer::optimizePathByDubins(vector<shared_ptr<State>> path, shared_ptr<Map> map)
	{
		double pathLength = distanceResolver->getLengthOfPath(path);

		shared_ptr<State> endOfPath = path[path.size() - 1]; //�pln� posledn� prvek cel� cesty, c�l
		int stopLimit = 500;	//kolikr�t za sebou se nesm� aplikov�n� Dubinse zlep�it trajektorie, aby se algoritmus zastavil
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

			auto pair = optimizePathPart(startIndex, endIndex, map, path);
			auto trajectoryPart = pair.first;
			bool isPathChanged = pair.second;

			if (isPathChanged)
			{
				vector<shared_ptr<State>> pathFirstPart;		//��st p�ed dubinsem
				vector<shared_ptr<State>> pathMiddlePart = trajectoryPart;	//��st nahrazen� dubinsem
				vector<shared_ptr<State>> pathLastPart;		//��st po dubinsovi
				
				if (startIndex > 0)	//p�ed vyseknutou ��st� je dal�� ��st
				{
					pathFirstPart = vector<shared_ptr<State>>(path.begin(), path.begin() + startIndex);	//subvector, vykousnut� ��sti vektoru, prvek na pozici startIndex u� tam nen�
				}

				if (endIndex < path.size() - 1)	//po vyseknut� ��sti je je�t� dal�� ��st
				{
					pathLastPart = vector<shared_ptr<State>>(path.begin() + endIndex + 1, path.end());	//subvector, vykousnut� ��sti vektoru
				}

				//spojen� 3 ��st� cesty
				auto newPath = pathFirstPart;
				newPath.insert(newPath.end(), pathMiddlePart.begin(), pathMiddlePart.end());
				newPath.insert(newPath.end(), pathLastPart.begin(), pathLastPart.end());

				if (distanceResolver->getLengthOfPath(newPath) < pathLength)
				{
					notImprovedCount = 0;
					pathLength = distanceResolver->getLengthOfPath(newPath);
					path = newPath;
				} else
				{
					notImprovedCount++;
				}
			} else
			{
				notImprovedCount++;
			}
		}
		return path;
	}

	//bool ��k�, zda se cesta zm�nila
	pair<vector<shared_ptr<State>>, bool> PathOptimizer::optimizePathPart(int startIndex, int endIndex, shared_ptr<Map> map, vector<shared_ptr<State>> path)
	{
		vector<shared_ptr<State>>::const_iterator startIterator = path.begin() + startIndex;	//iter�tory pou�ity pouze k vykousnut� ��sti cesty, kterou budu m�nit
		vector<shared_ptr<State>>::const_iterator endIterator = path.begin() + endIndex + 1;
		vector<shared_ptr<State>> pathPart(startIterator, endIterator);
		auto start = pathPart[0];
		auto end = pathPart[pathPart.size() - 1];

		double maxSpeed = configuration->getDistanceOfNewNodes();	//tak� reprezentuje po�et pixel�, kter� v car like modelu uraz� uav za sekundu
		//p�edpo��t�m si d�lky v�ech dubins� p�edem
		unordered_map<Uav, pair<Dubins, bool>, UavHasher> dubinsTrajectories = unordered_map<Uav, pair<Dubins, bool>, UavHasher>();	//��k�, zda je dubins krat�� ne� p�vodn� trajektorie nebo ne
		bool areAllDubinsTrajectoriesLonger = true;
		for (auto uav : end->getUavs())
		{
			double length = distanceResolver->getLengthOfPath(pathPart, uav);

			auto dubins = Dubins(start->getUav(uav)->getPointParticle()->toPosition(), uav->getPointParticle()->toPosition(), motionModel->getMinimalCurveRadius());
			double newLength = dubins.getLength();	//vrac� d�lku cel�ho man�vru
			bool isDubinsShorter = newLength < length;
			dubinsTrajectories[*uav.get()] = make_pair(dubins, isDubinsShorter);
			areAllDubinsTrajectoriesLonger = areAllDubinsTrajectoriesLonger && !isDubinsShorter;
		}

		logger->logDubinsPaths(dubinsTrajectories);

		if (areAllDubinsTrajectoriesLonger)
		{
			return make_pair(pathPart, false); // p�vodn� cesta
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
			int stepCount = totalTime / configuration->getTimeStep();	//po�et krok�, doba cel�ho man�vru / doba jednoho kroku

			if (isDubinsShorter && stepCount > largestStepCount)
			{
				largestStepCount = stepCount;
			}
		}

		if (largestStepCount == 0)	//tak mal� cesta, �e je men�� ne� krok simulace
		{
			return make_pair(pathPart, false); // p�vodn� cesta
		}

		//zde provedu diskretizaci postupn� pro v�echna uav najednou, kus po kusu a p�itom budu kontrolovat v�echny podm�nky
		int stepCount = largestStepCount;
		vector<shared_ptr<State>> newTrajectory = vector<shared_ptr<State>>(stepCount + 1);
		auto previousState = start;
		for (size_t i = 0; i <= stepCount; i++)
		{
			auto newState = make_shared<State>(*previousState.get());
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
					auto pointParticle = uav->getPointParticle();
					if (distanceCompleted < dubins.getLength())
					{

						//TODO: zde ulo�it podle pozice v Dubinsov� man�vru vstup
						switch (dubins.getCurrentManeuver(distanceCompleted))
						{
						case ManeuverPart::L:
							uav->setPreviousInput(make_shared<CarLikeControl>(configuration->getDistanceOfNewNodes(), configuration->getMaxTurn()));
							break;
						case ManeuverPart::S:
							uav->setPreviousInput(make_shared<CarLikeControl>(configuration->getDistanceOfNewNodes(), 0));
							break;
						case ManeuverPart::R: 
							uav->setPreviousInput(make_shared<CarLikeControl>(configuration->getDistanceOfNewNodes(), - configuration->getMaxTurn()));
							break;
						}

						auto newPosition = dubins.getPosition(distanceCompleted);

						pointParticle->getLocation()->setX(newPosition.getPoint().getX());
						pointParticle->getLocation()->setY(newPosition.getPoint().getY());
						pointParticle->getRotation()->setZ(newPosition.getAngle());
					}
					else
					{	//uav, kter� u� dorazilo do c�le, "po�k�" na ostatn�
						pointParticle->setLocation(end->getUav(uav)->getPointParticle()->getLocation());
						pointParticle->setRotation(end->getUav(uav)->getPointParticle()->getRotation());
					}
				} else
				{
					//pokud je krat�� p�vodn� cesta pro dan� uav, bere se pro dan� uav p�vodn� cesta, pokud je p�vodn� cesta krat�� na po�et krok�, uav "�ek�" v c�li
					shared_ptr<State> currentOldState;
					if (i >= pathPart.size())
					{
						currentOldState = pathPart[pathPart.size() - 1];
					} else
					{
						currentOldState = pathPart[i];
					}
					uav->getPointParticle()->setLocation(currentOldState->getUav(uav)->getPointParticle()->getLocation());
					uav->getPointParticle()->setRotation(currentOldState->getUav(uav)->getPointParticle()->getRotation());
				}

			}

			//validace
			if (!collisionDetector->isStateValid(previousState, newState, map))
			{
				return make_pair(pathPart, false); // p�vodn� cesta
			}

			previousState = newState;
			newTrajectory[i] = newState;
//			logger->logNewState(previousState, newState, true);
		}

		auto lastState = newTrajectory[newTrajectory.size() - 1];
		newTrajectory.push_back(end);

		return make_pair(newTrajectory, true);
	}

	void PathOptimizer::setLogger(const shared_ptr<LoggerInterface> logger_interface)
	{
		logger = logger_interface;
	}

	vector<shared_ptr<State>> PathOptimizer::removeDuplicitStates(vector<shared_ptr<State>> path)
	{
		vector<shared_ptr<State>> shorterPath = vector<shared_ptr<State>>();
		bool first = true;
		shared_ptr<State> previous;
		for (auto state : path)
		{
			//p�esko�en� prvn�ho
			if (first)
			{
				first = false;
				previous = state;
				shorterPath.push_back(state);
				continue;
			}

			//porovn�n� stejn�ch vzd�lenosti sousedn�ch stav�
			bool areStatesSame = true;
			for (auto uav : state->getUavs())
			{
				auto loc = uav->getPointParticle()->getLocation();
				auto previousLoc = previous->getUav(uav)->getPointParticle()->getLocation();
				if (loc->getDistance(previousLoc) > 0.01)
				{
					areStatesSame = false;
				}
			}

			if (!areStatesSame)
			{
				shorterPath.push_back(state);
			}

			previous = state;
		}
		return shorterPath;
	}
}
