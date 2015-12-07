#include "PathOptimizer.h"
#include "Random.h"
#include "Dubins/geom/geom.h"
#include "Configuration.h"

using namespace geom;

namespace App
{


	PathOptimizer::PathOptimizer(shared_ptr<DistanceResolver> distanceResolver, shared_ptr<Configuration> configuration, shared_ptr<CarLikeMotionModel> motionModel, shared_ptr<CollisionDetector> collisionDetector) :
		distanceResolver(distanceResolver), configuration(configuration), motionModel(motionModel), collisionDetector(collisionDetector)
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
		double maxSpeed = configuration->getDistanceOfNewNodes();	//tak� reprezentuje po�et pixel�, kter� v car like modelu uraz� uav za sekundu

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

			straightenCrossingTrajectories(start, end);	//pokud se k��� trajektorie, pak nemohu optimalizovat

			//p�edpo��t�m si d�lky v�ech dubins� p�edem
			unordered_map<Uav, pair<Dubins, bool>, UavHasher> dubinsTrajectories = unordered_map<Uav, pair<Dubins, bool>, UavHasher>();	//��k�, zda je dubins krat�� ne� p�vodn� trajektorie nebo ne
			bool areAllDubinsTrajectoriesLonger = true;
			for (auto uav : endOriginal->getUavs())
			{
				double length = 0;
				for (auto i = endOriginal; *i.get() != *start.get(); i = i->getPrevious())	//i jede od konce po prvek, jeho� p�edch�dce je za��tek
				{
					auto previous = i->getPrevious();
					length += distanceResolver->getDistance(previous, i);	//todo: vymyslet, jak naj�t v�d�lenost po��tanou po kru�nici, a ne po v�sledn�ch p��mk�ch
				}

				auto dubins = Dubins(uav->getPointParticle()->toPosition(), start->getUav(uav)->getPointParticle()->toPosition(), motionModel->getMinimalCurveRadius());
				double newLength = dubins.getLength();	//vrac� d�lku cel�ho man�vru
				bool isDubinsShorter = newLength < length;
				dubinsTrajectories[*uav.get()] = make_pair(dubins, isDubinsShorter);
				areAllDubinsTrajectoriesLonger = areAllDubinsTrajectoriesLonger && !isDubinsShorter;
			}

			if (areAllDubinsTrajectoriesLonger)
			{
				continue;
			}

			//nalezen� nejdel��ho dubbinse ze v�ech, kter� jsou krat�� ne� p�vodn� trajektorie, podle n�j se bude diskretizovat
			pair<Uav, int> largestStepCount = pair<Uav, int>();
			for (auto uav : end->getUavs())
			{

				auto pair = dubinsTrajectories[*uav.get()];
				auto dubins = pair.first;
				auto isDubinsShorter = pair.second;
				double newLength = dubins.getLength();	//vrac� d�lku cel�ho man�vru

				int totalTime = newLength / maxSpeed;	//doba cesty
				int stepCount = totalTime / configuration->getEndTime();	//po�et krok�, doba cel�ho man�vru / doba jednoho kroku

				if (isDubinsShorter && stepCount > largestStepCount.second)
				{
					largestStepCount = make_pair(*uav.get(), stepCount);
				}
			}


			//zde provedu diskretizaci postupn� pro v�echna uav najednou, kus po kusu a p�itom budu kontrolovat v�echny podm�nky
			int stepCount = largestStepCount.second;
			vector<shared_ptr<State>> newTrajectory = vector<shared_ptr<State>>(stepCount);
			auto previousState = start;
			for (size_t i = 0; i < stepCount; i++)
			{
				auto newState = make_shared<State>(*previousState.get());
				//todo: zm�nit hodnoty a nastavit previous
				newTrajectory[i] = newState;
			}
		}
		return path;
	}

	void PathOptimizer::straightenCrossingTrajectories(shared_ptr<State> start, shared_ptr<State> end)
	{
		bool intersecting = collisionDetector->areTrajectoriesIntersecting(start, end);
		while (intersecting)
		{
			auto uavs = collisionDetector->getIntersectingUavs(start, end);
			//swap intersecting uavs
			int tempId = end->getUav(uavs.first)->getId();
			end->getUav(uavs.first)->setId(end->getUav(uavs.second)->getId());
			end->getUav(uavs.second)->setId(tempId);
			intersecting = collisionDetector->areTrajectoriesIntersecting(start, end);
		}
	}

}
