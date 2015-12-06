#include "PathOptimizer.h"
#include "Random.h"

namespace App
{


	PathOptimizer::PathOptimizer(shared_ptr<DistanceResolver> distanceResolver) : 
		distanceResolver(distanceResolver)
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
			auto randomFirst = Random::element(path);
			auto randomSecond = Random::element(path);
			while (*randomFirst.get() == *randomSecond.get())
			{
				randomSecond = Random::element(path);
			}
			if (randomFirst->getIndex() > randomSecond->getIndex())
			{
				//swap
				auto temp = randomSecond;
				randomSecond = randomFirst;
				randomFirst = temp;
			}

			for (auto uav : randomSecond->getUavs())
			{

				double distance = 0;
				for (auto i = randomSecond; *i.get() != *randomFirst.get(); i = i->getPrevious())	//i jede od konce po prvek, jeho� p�edch�dce je za��tek
				{
					auto previous = i->getPrevious();
					distance += distanceResolver->getDistance(previous, i);
				}

				//sem p�ijde dubins
				double newDistance = 0;

				if (newDistance < distance)
				{
					//todo: ud�lat validace
					bool isNewPathValid = true;

					if (isNewPathValid)
					{

					}
					//zde se vyhod� del�� ��st cesty a m�sto n� se sem d� krat�� ��st cesty
				}
			}
		}
		return path;
	}


}
