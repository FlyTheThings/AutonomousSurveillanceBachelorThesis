#include "MapProcessor.h"
#include "VCollide/ColDetect.h"
#include "memory"
#include <algorithm>

using namespace std;

namespace App
{

	MapProcessor::MapProcessor(shared_ptr<LoggerInterface> logger) : logger(logger)
	{
	}

	MapProcessor::~MapProcessor()
	{
	}

	shared_ptr<MapGraph> MapProcessor::mapToNodes(shared_ptr<Map> map, int cellSize, int worldWidth, int worldHeigh, double uavSize)
	{
		//firstly we have to get map as 2D matrix, grid
		auto mapGrid = getMapGrid(map, cellSize, worldWidth, worldHeigh, uavSize);	//map object and parameters to 2D matrix of enums (grid)
		logger->logMapGrid(mapGrid);
		//now we get nodes from this grid
		auto nodes = gridToNodes(mapGrid, cellSize);

		countDistancesToObstacles(nodes);

		logger->logMapNodes(nodes);


		//now we determine starting and ending node.
		//Start node is node, where starts UAV in middle
		//End node is node in middle of each goal recrangle
		//Todo: vymyslet, zda zde natvrdo pou��vat pro nalezen� st�edu obd�ln�ky �i ne
		//todo: vymyslet, jak pou��t node s podobn�m n�klonem, pokud nejsou uav p�esn� na hran�ch �tverc�. mo�n� br�t je�t� okoln� nodes, pokud na nich nen� p�ek�ka?
		shared_ptr<Node> startNode = getStartNode(nodes, map, cellSize);
		auto endNodes = getEndNodes(nodes, map, cellSize);


		//�prava, aby mapa odpov�dala matlabovsk� p�edloze pro p�esn� porovn�v�n�
		//nastaveno pro mapu 3
		bool modifyByHand = false;
		if (modifyByHand)
		{
			for (auto node : nodes)
			{
				if (node->contains(75, 75, cellSize / 2))	//nalezen� node, ve kter� je st�ed
				{
					startNode = node;
					break;
				}
			}

			for (size_t i = 0; i < map->getGoals().size(); i++)
			{

				for (auto node : nodes)
				{
					if (node->contains(725, 775, cellSize / 2))	//nalezen� node, ve kter� je st�ed
					{
						endNodes[i] = node;
						break;
					}
				}
			}
		}


		shared_ptr<MapGraph> graph = make_shared<MapGraph>(nodes, startNode, endNodes);
		return graph;
	}

	vector<vector<Grid>> MapProcessor::getMapGrid(shared_ptr<Map> map, int cellSize, int worldWidth, int worldHeigh, double uavSize)
	{
		int gridRow = 0;
		int rows = ceil(double(worldWidth) / double(cellSize));	//todo: zkontrolovat, zda nemus�m p�i��st 1, podle zaokrouhlov�n�
		int columns = ceil(double(worldHeigh) / double(cellSize));	//todo: zkontrolovat, zda nemus�m p�i��st 1, podle zaokrouhlov�n�
		auto grid = vector<vector<Grid>>(rows);
		for (int i = cellSize; i <= worldWidth; i += cellSize)
		{
			grid[gridRow] = vector<Grid>(columns);
			int gridColumn = 0;
			for (int j = cellSize; j <= worldHeigh; j += cellSize)
			{
				grid[gridRow][gridColumn] = analyzeCell(map, Point(i - cellSize, j - cellSize), Point(i, j), uavSize);	//ternary operator checking borders of map
				gridColumn++;
			}
			gridRow++;
		}
		return grid;
	}

	Grid MapProcessor::analyzeCell(shared_ptr<Map> map, Point leftBottom, Point rightUpper, double uavSize)
	{
		ColDetect colDetect;
		Rectangle2D cell = Rectangle2D(leftBottom.getX(), leftBottom.getY(), rightUpper.getX() - leftBottom.getX(), rightUpper.getY() - leftBottom.getY());

		for (auto uavStart : map->getUavsStart())
		{
			if (colDetect.coldetect(
				Rectangle2D(uavStart->getPointParticle()->getLocation()->getX() - uavSize / 2, uavStart->getPointParticle()->getLocation()->getY() - uavSize / 2, uavSize, uavSize), cell))
			{
				return Grid::UAV;
			}
		}

		for (auto obstacle : map->getObstacles())
		{
			if (colDetect.coldetect(
				Rectangle2D(obstacle->rectangle->getX(), obstacle->rectangle->getY(), obstacle->rectangle->getWidth(), obstacle->rectangle->getHeight()), cell))
			{
				return Grid::Obstacle;
			}
		}

		for (auto goal : map->getGoals())
		{
			if (colDetect.coldetect(
				Rectangle2D(goal->rectangle->getX(), goal->rectangle->getY(), goal->rectangle->getWidth(), goal->rectangle->getHeight()), cell))
			{
				return Grid::Goal;
			}
		}

		return Grid::Free;
	}

	vector<shared_ptr<Node>> MapProcessor::gridToNodes(vector<vector<Grid>> mapGrid, int cellSize)
	{
		auto nodes = vector<shared_ptr<Node>>(mapGrid.size() * mapGrid[0].size());
		int index = 0;

		for (size_t i = 0; i < mapGrid.size(); i++)
		{
			auto row = mapGrid[i];
			for (size_t j = 0; j < row.size(); j++)
			{
				Grid grid = row[j];
				int x = i * cellSize + cellSize / 2;
				int y = j * cellSize + cellSize / 2;
				nodes[index] = make_shared<Node>(make_shared<Point>(x, y), grid);

				index++;
			}
		}

		double cost_neighbor = 10; // cena node, pokud je n�kde vedle n� p�ek�ka
		double cost_diagonal = 5; // cena node, pokud je diagon�ln� k n� p�ek�ka
		index = 0;	//index dan� node

		//adding of neighbors, when all nodes are added
		for (size_t i = 0; i < mapGrid.size(); i++)
		{
			auto row = mapGrid[i];
			for (size_t j = 0; j < row.size(); j++)
			{
				auto node = nodes[index];
				for (int p = -1; p <= 1; p++)
				{
					for (int q = -1; q <= 1; q++)
					{
						bool isNeighbor = p != 0 || q != 0;
						bool isOutOfMap = (i + p) < 0 || (j  + q) < 0 //kontrola 1. ��dku a 1. sloupce
							|| (i + p) >= mapGrid.size() || (j + q) >= row.size(); //kontrola posledn�ho ��dku a posledn�ho sloupce
						if (
							isNeighbor && !isOutOfMap
						) {
							auto neighbor = nodes[(i + p) * mapGrid.size() + (j + q)];
							if (neighbor)	//kontrola empty pointeru
							{
								if (neighbor->getGridType() == Grid::Obstacle)
								{
									if (p == 0 || q == 0)	//p��m� soused
									{
										node->increaseCost(cost_neighbor);
									}
									else	//soused na diagon�le
									{
										node->increaseCost(cost_diagonal);
									}
								}
								//sousedy p�id�v�m v�dy, kv�li ur�ov�n� vzd�le nosti od p�ek�ek. u a staru si sousedy filtruju a expanduji jen do soused� bez p�ek�ek
								bool isDiagonal = p != 0 && q != 0;
								node->addNeighbor(neighbor, isDiagonal);
							}
						}
					}
				}
				index++;
			}
		}

		return nodes;
	}

	shared_ptr<Node> MapProcessor::getStartNode(vector<shared_ptr<Node>> nodes, shared_ptr<Map> map, int cellSize)
	{
		shared_ptr<Node> startNode;
		int uavCount = map->countUavs();
//		shared_ptr<Point> middleUav = map->getUavsStart()[uavCount / 2]->getPointParticle()->getLocation();
//		for (auto node : nodes)
//		{
//			if (node->contains(middleUav->getX(), middleUav->getY(), cellSize / 2))	//nalezen� node, ve kter� je st�ed
//			{
//				startNode = node;
//				break;
//			}
//		}


		//pokud za��naj� UAV na za��tku jinak nato�en� ne� je sm�r vedoc� cesty, mus� se slo�it� ot��et, proto vyberu ze v�ech nodes, na kter�ch le�� n�jak� uav, node, kter� le�� ve sm�ru pr�m�rn�ho oto�en� UAV
		//sm�r u��m podle �hlu, kter� dan� node sv�r� s pr�m�rn�m bodem mezi v�emi UAV.
		vector<shared_ptr<Node>> startingNodes = vector<shared_ptr<Node>>();
		for (auto node : nodes)
		{
			//todo: zjistit, pro� p�i map� 6 a aStarCellSize = 9 se objevuj� empty prvky
			if(node)
			{
				for (auto uav : map->getUavsStart())
				{
					if (node->contains(uav->getPointParticle()->getLocation()->getX(), uav->getPointParticle()->getLocation()->getY(), cellSize / 2))	//nalezen� node, ve kter� je st�ed
					{
						startingNodes.push_back(node);
						break;
					}
				}
			}
		}

		double averageRotation = 0;
		shared_ptr<Point> averageLocation = make_shared<Point>(0, 0);
		for (auto uav : map->getUavsStart())
		{
			averageRotation += uav->getPointParticle()->getRotation()->getZ();	//u 2D je pouze rotace okolo osy Z
			averageLocation->changeX(uav->getPointParticle()->getLocation()->getX());
			averageLocation->changeY(uav->getPointParticle()->getLocation()->getY());
		}

		averageRotation /= uavCount;
		averageLocation->setX(averageLocation->getX() / uavCount);
		averageLocation->setY(averageLocation->getY() / uavCount);

		double similarRotation = DBL_MAX;
		for(auto possibleStartNode : startingNodes)
		{
			double angle = atan2(possibleStartNode->getPoint()->getY() - averageLocation->getY(), possibleStartNode->getPoint()->getX() - averageLocation->getX());
			if (abs(angle - averageRotation) < similarRotation)
			{
				similarRotation = abs(angle - averageRotation);
				startNode = possibleStartNode;
			}
		}

		return startNode;
	}

	vector<shared_ptr<Node>> MapProcessor::getEndNodes(vector<shared_ptr<Node>> nodes, shared_ptr<Map> map, int cellSize)
	{
		auto endNodes = vector<shared_ptr<Node>>(map->getGoals().size());
		for (size_t i = 0; i < map->getGoals().size(); i++)
		{
			auto goal = map->getGoals()[i]->rectangle;
			int middleX = goal->getX() + goal->getWidth() / 2;
			int middleY = goal->getY() + goal->getHeight() / 2;


			for (auto node : nodes)
			{
				if (node->contains(middleX, middleY, cellSize / 2))	//nalezen� node, ve kter� je st�ed
				{
					endNodes[i] = node;
					break;
				}
			}
		}
		return endNodes;
	}

	void MapProcessor::countDistancesToObstacles(vector<shared_ptr<Node>> nodes)
	{
		//nejd��ve za�nu od nodes s p�ek�kami. ty expanduji (pouze direct neighbors) a expandovan�m nod�m ulo��m vzd�lenost 1 a ulo��m sousedy do seznamu. P�vodn� nody ze seznamu odeb�r�m.
		//pak opakuji, zvy�uji vzd�lenost. neohodnocenou node pozn�m podle n�jak� defaultn� hodnoty.
		//pokud je ji� node ohodnocen�, ale na�el jsem ji s v�t�� vzd�lenost� od jin� p�ek�ky, p�ep�u v�t�� vzd�lenost men�� vzd�lenost�.
		//max. tolik expanz�, kolik je ���ka mapy. v�ce nen� pot�eba. 
		//nev�m, jestli expandovat zvl᚝ ka�dou p�ek�ku nebo najednou

		//todo: vyzkou�et, jak je to s push_back a pln� neobsazen�m vektorem		
		vector<shared_ptr<Node>> openedNodes = vector<shared_ptr<Node>>();
		for (auto node : nodes)
		{
			if (node->getGridType() == Grid::Obstacle)
			{
				openedNodes.push_back(node);
			}
		}

		while (!openedNodes.empty())
		{
			shared_ptr<Node> node;
			do
			{
				node = openedNodes[0];
				openedNodes.erase(remove(openedNodes.begin(), openedNodes.end(), node));	//erase remove idiom. for removing. really, c++? really?
			} while (!node && !openedNodes.empty());	//kontrola null pointeru a nepr�zdn�ho pole
			if (!node)	//pole se vypr�zdnilo a tak je node null
			{
				break;
			}

			if (node->getGridType() == Grid::Obstacle)
			{
				node->setDistanceToObstacle(0);
			}
			int neighborsDistance = node->getDistanceToObstacle() + 1;	//distance incremented by one, because neighbors have n+1 distance from nearest obstacle, when node has distance n.
			auto neighbors = node->getDirectNeighbors();
			for (auto neighbor : neighbors)
			{

				if (neighbor->getDistanceToObstacle() > neighborsDistance)
				{
					neighbor->setDistanceToObstacle(neighborsDistance);
					openedNodes.push_back(neighbor);
				}
			}
		}
	}
}
