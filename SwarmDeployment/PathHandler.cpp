#include "PathHandler.h"

namespace App
{

	PathHandler::PathHandler(shared_ptr<CollisionDetector> collisionDetector) : 
		collisionDetector(collisionDetector)
	{
	}


	PathHandler::~PathHandler()
	{
	}

	vector<shared_ptr<LinkedState>> PathHandler::getPath(shared_ptr<LinkedState> end)
	{
		vector<shared_ptr<LinkedState>> path = vector<shared_ptr<LinkedState>>();
		auto iterNode = end;
		do
		{
			path.push_back(iterNode);
			iterNode = iterNode->getPrevious();
		} while (iterNode->getPrevious());

		path.push_back(iterNode);

		reverse(path.begin(), path.end());	//abych m�l cestu od za��tku do konce
		return path;
	}

	vector<shared_ptr<State>> PathHandler::createStatePath(vector<shared_ptr<LinkedState>> path)
	{
		vector<shared_ptr<State>> newPath = vector<shared_ptr<State>>(path.size());
		for (size_t i = 0; i < path.size(); i++)
		{
			newPath[i] = make_shared<State>(*path[i].get());
		}

		return newPath;
	}

	vector<shared_ptr<LinkedState>> PathHandler::getPath(shared_ptr<LinkedState> start, shared_ptr<LinkedState> end)
	{
		vector<shared_ptr<LinkedState>> path = vector<shared_ptr<LinkedState>>();
		auto iterNode = end;
		do
		{
			path.push_back(iterNode);
			iterNode = iterNode->getPrevious();
		} while (*iterNode.get() != *start.get());

		path.push_back(iterNode);

		reverse(path.begin(), path.end());	//abych m�l cestu od za��tku do konce
		return path;
	}

	//narovn� v�echny trajektorie p�edt�m, ne� se sput� optimalizace Dubinsem
	void PathHandler::straightenCrossingTrajectories(vector<shared_ptr<State>> path)
	{
		for (size_t i = 1; i < path.size(); i++)
		{
			for (auto uav : path[i]->getUavs())
			{
				auto start = path[i - 1]->getUav(uav);
				auto end = uav;
				for (size_t j = 1; j < path.size(); j++)
				{
					for (auto another : path[j]->getUavs())
					{
						auto anotherStart = path[j - 1]->getUav(another);
						auto anotherEnd = another;

						bool intersecting = collisionDetector->areLineSegmentsIntersecting(start, end, anotherStart, anotherEnd);
						//swap intersecting uavs in all states after end (including end)
						if (intersecting)
						{
							//jedna cesta je v�dy del�� ne� jin�, proto�e o�et�en� na kolize mezi sousedn�mi stavy je ji� v rrt-path
							vector<shared_ptr<PointParticle>> pathToEnd1;	//cesty k���c�ch se uav do konce. cesty se zkop�ruj� sem, pak se vlo�� �ekac� stavy a pak se odzadu vlo�� cesty odsud do p�vodn�ch cest
							vector<shared_ptr<PointParticle>> pathToEnd2;

							for (size_t k = i; k < path.size(); k++)
							{
								pathToEnd1.push_back(path[k]->getUav(uav)->getPointParticle());
							}

							for (size_t k = j; k < path.size(); k++)
							{
								pathToEnd2.push_back(path[k]->getUav(another)->getPointParticle());
							}

							//nyn� vlo��m �ekac� stavy, abych mohl prohodit cesty
							int waitingStatesCount = abs(int(i - j));
							vector<shared_ptr<State>> waitingStates;
							int startIndex = min(i, j);	//aby se za�ala prohazovat cesta od prvku s kolizemi, kter� je nejbl�e c�li

							auto waitingState = path[startIndex - 1];
							for (size_t k = 0; k < waitingStatesCount; k++)
							{
								waitingStates.push_back(make_shared<State>(*waitingState.get()));
							}
							path.insert(path.begin() + startIndex - 1, waitingStates.begin(), waitingStates.end());


							//odzadu zapisuji do pole prohozen� cesty
							{
								int index = pathToEnd2.size() - 1;
								for (size_t k = path.size() - 1; k >= i; k++)
								{
									path[k]->getUav(uav)->setPointParticle(pathToEnd2[index]);
									index--;
								}
							}
							{
								int index = pathToEnd1.size() - 1;
								for (size_t k = path.size() - 1; k >= j; k++)
								{
									path[k]->getUav(another)->setPointParticle(pathToEnd1[index]);
									index--;
								}

							}

							//m�sto v�m�ny id vym�n�m ��sti cest, proto�e budu m�nit r�zn� dlouh� �seky (r�zn� dlouh� na po�et stav�)
//							for (size_t k = startIndex; k < path.size(); k++)
//							{
//								auto toBeSwapped = path[k];
//								toBeSwapped->swapUavs(uav, another);
//							}
						}
					}
				}

			}
		}
	}

}