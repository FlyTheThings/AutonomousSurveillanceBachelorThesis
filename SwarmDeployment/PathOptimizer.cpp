#include "PathOptimizer.h"
#include "Random.h"

namespace App
{


	PathOptimizer::PathOptimizer()
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
			//sem p�ijde dubbins
			auto randomFirst = Random::element(path);
		}
		return path;
	}


}
