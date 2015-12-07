#include "DistanceResolver.h"
#include "Configuration.h"

namespace App
{

	DistanceResolver::DistanceResolver(shared_ptr<Configuration> configuration) : 
		configuration(configuration)
	{
	}

	DistanceResolver::~DistanceResolver()
	{
	}

	double DistanceResolver::getDistance(shared_ptr<State> first, unordered_map<Uav, shared_ptr<Point>, UavHasher> second)
	{
		NNMethod nn_method = configuration->getNearestNeighborMethod();
		double totalDistance = 0;
		unordered_map<Uav, double, UavHasher> distances = unordered_map<Uav, double, UavHasher>(first->getUavs().size());

		for (auto uav : first->getUavs())
		{
			auto randomState = second[*uav.get()];
			distances[*uav.get()] = uav->getPointParticle()->getLocation()->getDistance(randomState);
		}

		switch (nn_method)
		{
		case NNMethod::Total:
			for (auto dist : distances)
			{
				auto distance = dist.second;
				totalDistance += distance;	//no function for sum, so I must do it by hand
			}
			break;
		case NNMethod::Max:
			totalDistance = (*max_element(distances.begin(), distances.end())).second;	//tohle vrac� iter�tor, kter� mus�m dereferencovat, abych z�skal ��slo. fuck you, C++
			break;
		case NNMethod::Min:
			totalDistance = (*min_element(distances.begin(), distances.end())).second;	//tohle vrac� iter�tor, kter� mus�m dereferencovat, abych z�skal ��slo. fuck you, C++
			break;
		}
		return totalDistance;
	}

	double DistanceResolver::getDistance(shared_ptr<State> first, shared_ptr<State> second)
	{
		auto secondMap = unordered_map<Uav, shared_ptr<Point>, UavHasher>();
		for (auto uav : second->getUavs())
		{
			secondMap[*uav.get()] = uav->getPointParticle()->getLocation();
		}
		return getDistance(first, secondMap);
	}

	double DistanceResolver::getDistance(shared_ptr<State> first, shared_ptr<State> second, shared_ptr<Uav> uav)
	{
		return first->getUav(uav)->getPointParticle()->getLocation()->getDistance(second->getUav(uav)->getPointParticle()->getLocation());
	}

	double DistanceResolver::getLengthOfPath(shared_ptr<State> start, shared_ptr<State> end)
	{
		double length = 0;
		for (auto i = end; i->getTime() != start->getTime(); i = i->getPrevious())	//i jede od konce po prvek, jeho� p�edch�dce je za��tek
		{
			auto previous = i->getPrevious();
			length += getDistance(previous, i);	//todo: vymyslet, jak naj�t v�d�lenost po��tanou po kru�nici, a ne po v�sledn�ch p��mk�ch
		}
		return length;
	}

	double DistanceResolver::getLengthOfPath(vector<shared_ptr<State>> path)
	{
		double length = 0;
		for (size_t i = 1; i < path.size(); i++)
		{
			auto state = path[i];
			length += getDistance(state->getPrevious(), state);
		}
		return length;
	}

	double DistanceResolver::getLengthOfPath(shared_ptr<State> start, shared_ptr<State> end, shared_ptr<Uav> uav)
	{
		double length = 0;
		for (auto i = end; i->getTime() != start->getTime(); i = i->getPrevious())	//i jede od konce po prvek, jeho� p�edch�dce je za��tek
		{
			auto previous = i->getPrevious();
			length += getDistance(previous, i, uav);	//todo: vymyslet, jak naj�t v�d�lenost po��tanou po kru�nici, a ne po v�sledn�ch p��mk�ch
		}
		return length;
	}

}
