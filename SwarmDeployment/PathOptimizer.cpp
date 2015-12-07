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
	vector<shared_ptr<State>> PathOptimizer::optimizePath(vector<shared_ptr<State>> path)
	{
		int stopLimit = 100;	//kolikr�t za sebou se nesm� aplikov�n� Dubinse zlep�it trajektorie, aby se algoritmus zastavil
		int notImprovedCount = 0;

		while (notImprovedCount < stopLimit)
		{
			auto start = Random::element(path);
			auto endOriginal = Random::element(path);
			while (*start.get() == *endOriginal.get())
			{
				endOriginal = Random::element(path);
			}
			if (start->getIndex() > endOriginal->getIndex())
			{
				//swap
				auto temp = endOriginal;
				endOriginal = start;
				start = temp;
			}

			auto end = make_shared<State>(*endOriginal.get());	//kv�li prohazov�n� k��ej�c�ch se UAV vytvo��m kopii, kterou modifikuji. A� kdy� zjist�m, �e je nov� trajektorie v po��dku, ulo��m to do p�vodn�ho stavu

			optimizePathBetween(start, end);
		}
		return path;
	}

	vector<shared_ptr<State>> PathOptimizer::optimizePathBetween(shared_ptr<State> start, shared_ptr<State> end)
	{
		straightenCrossingTrajectories(start, end);	//pokud se k��� trajektorie, pak nemohu optimalizovat

		double maxSpeed = configuration->getDistanceOfNewNodes();	//tak� reprezentuje po�et pixel�, kter� v car like modelu uraz� uav za sekundu
		//p�edpo��t�m si d�lky v�ech dubins� p�edem
		unordered_map<Uav, pair<Dubins, bool>, UavHasher> dubinsTrajectories = unordered_map<Uav, pair<Dubins, bool>, UavHasher>();	//��k�, zda je dubins krat�� ne� p�vodn� trajektorie nebo ne
		bool areAllDubinsTrajectoriesLonger = true;
		for (auto uav : end->getUavs())
		{
			double length = 0;
			for (auto i = end; *i.get() != *start.get(); i = i->getPrevious())	//i jede od konce po prvek, jeho� p�edch�dce je za��tek
			{
				auto previous = i->getPrevious();
				length += distanceResolver->getDistance(previous, i);	//todo: vymyslet, jak naj�t v�d�lenost po��tanou po kru�nici, a ne po v�sledn�ch p��mk�ch
			}

			auto dubins = Dubins(start->getUav(uav)->getPointParticle()->toPosition(), uav->getPointParticle()->toPosition(), motionModel->getMinimalCurveRadius());
			double newLength = dubins.getLength();	//vrac� d�lku cel�ho man�vru
			bool isDubinsShorter = newLength < length;
			dubinsTrajectories[*uav.get()] = make_pair(dubins, isDubinsShorter);
			areAllDubinsTrajectoriesLonger = areAllDubinsTrajectoriesLonger && !isDubinsShorter;
		}

		logger->logDubinsPaths(dubinsTrajectories);

		if (areAllDubinsTrajectoriesLonger)
		{
			return PathHandler::getPath(start, end);
		}

		//nalezen� nejdel��ho dubbinse ze v�ech, kter� jsou krat�� ne� p�vodn� trajektorie, podle n�j se bude diskretizovat
		int largestStepCount;
		for (auto uav : end->getUavs())
		{

			auto pair = dubinsTrajectories[*uav.get()];
			auto dubins = pair.first;
			auto isDubinsShorter = pair.second;
			double newLength = dubins.getLength();	//vrac� d�lku cel�ho man�vru
			int totalTime = newLength / maxSpeed;	//doba cesty
			int stepCount = totalTime / configuration->getEndTime();	//po�et krok�, doba cel�ho man�vru / doba jednoho kroku

			if (isDubinsShorter && stepCount > largestStepCount)
			{
				largestStepCount = stepCount;
			}
		}

		//zde provedu diskretizaci postupn� pro v�echna uav najednou, kus po kusu a p�itom budu kontrolovat v�echny podm�nky
		int stepCount = largestStepCount;
		vector<shared_ptr<State>> newTrajectory = vector<shared_ptr<State>>(stepCount);
		auto previousState = start;
		for (size_t i = 0; i < stepCount; i++)
		{
			auto newState = make_shared<State>(*previousState.get());
			double distanceCompleted = i * configuration->getDistanceOfNewNodes();	//ura�en� cesta v dubinsov� man�vru

			unordered_map<Uav, shared_ptr<CarLikeControl>, UavHasher> inputs = unordered_map<Uav, shared_ptr<CarLikeControl>, UavHasher>();
			//zde se pro ka�d� UAV vybere podle typu Dubinsova man�vru vhodn� input pro motion model
			for (auto uav : newState->getUavs())
			{
				auto dubins = dubinsTrajectories[*uav.get()].first;

				auto newPosition = dubins.getPosition(distanceCompleted + configuration->getDistanceOfNewNodes());

				uav->getPointParticle()->getLocation()->setX(newPosition.getPoint().getX());
				uav->getPointParticle()->getLocation()->setY(newPosition.getPoint().getY());
				uav->getPointParticle()->getRotation()->setZ(newPosition.getAngle());
			}
			newState->setPrevious(previousState);

			bool isValid = true;

			if (!isValid)
			{
				return PathHandler::getPath(start, end);
			}

			previousState = newState;
			//todo: zm�nit hodnoty a nastavit previous
			newTrajectory[i] = newState;
			logger->logNewState(previousState, newState, true);

		}

		return newTrajectory;
	}

	void PathOptimizer::setLogger(const shared_ptr<LoggerInterface> logger_interface)
	{
		logger = logger_interface;
	}

	void PathOptimizer::straightenCrossingTrajectories(shared_ptr<State> start, shared_ptr<State> end)
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
