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
			newPath[i] = make_shared<State>(path[i]);
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
	vector<shared_ptr<State>> PathHandler::straightenCrossingTrajectories(vector<shared_ptr<State>> path)
	{
		for (size_t i = 1; i < path.size(); i++)
		{
			for (auto uav : path[i]->getBaseUavs())
			{
				auto start = path[i - 1]->getBaseUav(uav);
				auto end = uav;
				for (size_t j = 1; j < path.size(); j++)
				{
					for (auto another : path[j]->getBaseUavs())
					{
						auto anotherStart = path[j - 1]->getBaseUav(another);
						auto anotherEnd = another;

						//aby se nehledala kolize u toho sam�ho uav
						if (*start.get() == *anotherStart.get() && *end.get() == *anotherEnd.get())
						{
							continue;
						}
						bool intersecting = collisionDetector->areLineSegmentsIntersecting(start, end, anotherStart, anotherEnd);
						//swap intersecting uavs in all states after end (including end)
						if (intersecting)
						{
							//jedna cesta je v�dy del�� ne� jin�, proto�e o�et�en� na kolize mezi sousedn�mi stavy je ji� v rrt-path
							vector<shared_ptr<PointParticle>> pathToEnd1;	//cesty k���c�ch se uav do konce. cesty se zkop�ruj� sem, pak se vlo�� �ekac� stavy a pak se odzadu vlo�� cesty odsud do p�vodn�ch cest
							vector<shared_ptr<PointParticle>> pathToEnd2;

							for (size_t k = i; k < path.size(); k++)
							{
								pathToEnd1.push_back(path[k]->getBaseUav(uav)->getPointParticle());
							}

							for (size_t k = j; k < path.size(); k++)
							{
								pathToEnd2.push_back(path[k]->getBaseUav(another)->getPointParticle());
							}

							//nyn� vlo��m �ekac� stavy, abych mohl prohodit cesty
							int waitingStatesCount = abs(int(i - j));
							vector<shared_ptr<State>> waitingStates;
							int startIndex = min(i, j);	//aby se za�ala prohazovat cesta od prvku s kolizemi, kter� je nejbl�e c�li


							UavPath preservePointParticle = i > j ? UavPath::First : UavPath::Second;	//ur�uje, zda je del�� prvn� nebo druh� ��st, tedy, jak� si m� "zachovat bod", kter� by se jinak vymazal
							shared_ptr<PointParticle> preservedPointParticle;
							if (preservePointParticle == UavPath::First)
							{
								preservedPointParticle = path[i - 1]->getBaseUav(uav)->getPointParticle();
							}
							else {
								preservedPointParticle = path[j - 1]->getBaseUav(another)->getPointParticle();
							}


							auto waitingState = path[startIndex - 1];
							for (size_t k = 0; k < waitingStatesCount; k++)
							{
								waitingStates.push_back(make_shared<State>(*waitingState.get()));
							}
							path.insert(path.begin() + startIndex - 1, waitingStates.begin(), waitingStates.end());

							//odzadu zapisuji do pole prohozen� cesty
							//vlastn� scope to m� jen, proto�e mi p�ijde �ist��, �e ka�d� cyklus m� vlastn� prom�nnou index
							{
								int index = pathToEnd2.size() - 1;
								for (size_t k = path.size() - 1; k >= i; k--)
								{
									//pokud je jedna cesta krat�� ne� druh� (co� je v�dy), tak se mus� p�ed novou ��st cesty (tedy pro index < 0) vlo�it dal�� �ekac� stav
									if (index < 0)
									{
										path[k]->getBaseUav(uav)->setPointParticle(path[k - 1]->getBaseUav(uav)->getPointParticle());
									} else
									{
										path[k]->getBaseUav(uav)->setPointParticle(pathToEnd2[index]);
									}
									index--;
								}

								if (preservePointParticle == UavPath::First)
								{
									path[i - 1]->getBaseUav(uav)->setPointParticle(preservedPointParticle);
								}
							}
							{
								int index = pathToEnd1.size() - 1;
								for (size_t k = path.size() - 1; k >= j; k--)
								{
									//pokud je jedna cesta krat�� ne� druh� (co� je v�dy), tak se mus� p�ed novou ��st cesty (tedy pro index < 0) vlo�it dal�� �ekac� stav
									if (index < 0)
									{
										path[k]->getBaseUav(another)->setPointParticle(path[k - 1]->getBaseUav(another)->getPointParticle());
									}
									else
									{
										path[k]->getBaseUav(another)->setPointParticle(pathToEnd1[index]);
									}
									index--;
								}

								if (preservePointParticle == UavPath::Second)
								{
									path[j - 1]->getBaseUav(another)->setPointParticle(preservedPointParticle);
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
		return path;
	}

}