#include "Core.h"
#include "Map.h"
#include "Configuration.h"
#include "MapFactory.h"
#include "Path.h"
#include "GuidingPathFactory.h"
#include "MapProcessor.h"
#include <iostream>
#include <cstdio>
#include <ctime>
#include <cmath>
#include <memory>
#include <string>
#include "Output.h"
#include "State.h"
#include "Random.h"
#include "UavGroup.h"
#include "VCollide/Triangle3D.h"
#include "VCollide/ColDetect.h"
#include <chrono>
#include <thread>
#include "Enums.h"
#include "Uav.h"
#include <valarray>
#include <algorithm>

#define PI 3.14159265358979323846

using namespace std;

namespace App
{

	Core::Core(shared_ptr<Configuration> configuration) :
		logger(make_shared<LoggerInterface>()), configuration(configuration), 
		stateFactory(make_shared<StateFactory>(configuration))
	{
		setLogger(make_shared<LoggerInterface>());	//I will use LoggerInterface as NilObject for Logger, because I am too lazy to write NilObject Class.

		MapFactory mapFactory;
		maps = mapFactory.createMaps(configuration->getUavCount());
	}


	Core::~Core()
	{
	}

	void Core::run()
	{
		file = ofstream("myLogging.txt");
		
		clock_t start;
		double duration;

		start = clock();

		shared_ptr<Map> map = maps.at(configuration->getMapNumber());
		logger->logSelectedMap(map, configuration->getWorldWidth(), configuration->getWorldHeight());
		MapProcessor mapProcessor = MapProcessor(logger);
		auto nodes = mapProcessor.mapToNodes(map, configuration->getAStarCellSize(), configuration->getWorldWidth(), configuration->getWorldHeight(), configuration->getUavSize());
		GuidingPathFactory pathFactory = GuidingPathFactory(logger);
		auto paths = pathFactory.createGuidingPaths(nodes->getAllNodes(), nodes->getStartNode(), nodes->getEndNodes());

		duration = (clock() - start) / double(CLOCKS_PER_SEC);

		cout << to_string(duration) << "seconds to discretize map and find path" << endl;


//		try
//		{
			rrtPath(paths, configuration, map);
			file.close();
//		} catch(const rrtPathError& error)
//		{
			file.close();
//			throw error;
//		}
//		testGui();

	}

	void Core::testGui()
	{
		for (size_t i = 0; i < 200; i++)
		{
			this_thread::sleep_for(chrono::milliseconds(500));
			this->logger->logText(to_string(i));
		}
	}

	void Core::setLogger(shared_ptr<LoggerInterface> logger)
	{
		this->logger = logger;
	}

	void Core::logConfigurationChange()
	{
		auto map = maps.at(configuration->getMapNumber());
		logger->logSelectedMap(map, configuration->getWorldWidth(), configuration->getWorldHeight());
	}

	void Core::rrtPath(vector<shared_ptr<Path>> guiding_paths, shared_ptr<Configuration> configuration, shared_ptr<Map> map)
	{
//		throw 22;
//		throw runtime_error("No valid input found.");

		int uavCount = configuration->getUavCount();
		int rrt_min_nodes = configuration->getRrtMinNodes();
		int rrt_max_nodes = configuration->getRrtMaxNodes();
		int number_of_solutions = configuration->getNumberOfSolutions();
		int near_count = configuration->getNearCount();
		bool debug = configuration->getDebug();
		int distance_of_new_nodes = configuration->getDistanceOfNewNodes();
		int guiding_near_dist = configuration->getGuidingNearDist();

		cout << "Starting RRT-path...";

		vector<shared_ptr<State>> nodes = vector<shared_ptr<State>>();
		auto initialState = stateFactory->createState();
		initialState->uavs = map->getUavsStart();
		nodes.push_back(initialState);

		for (auto uav : initialState->uavs)
		{
			for (auto guidingPath : guiding_paths)
			{
				uav->current_indexes->set(guidingPath, guidingPath->get(0));
			}
		}

		auto final_nodes = vector<shared_ptr<State>>(rrt_max_nodes);


		int nodes_count = rrt_max_nodes;	//todo: zjistit, co s tou prom�nnou m�m d�lat
		shared_ptr<State> newState;
		shared_ptr<State> nearState = initialState;
		auto output = make_shared<Output>();

		int i = 0; // po�et expandovan�ch nodes, hned na za��tku se zv��� o jedna
		int m = 0; // po�et nalezen�ch cest
		int s = 2; // po�et pr�chod� cyklem ? prost� se to jen zv�t�� o 1 p�i ka�d�m pr�chodu, nikde se nepou��v�

		while ((m <= number_of_solutions || i < rrt_min_nodes) && i < rrt_max_nodes) // number_of_solutions je asi 10 000.
		{
			if (configuration->getStop())
			{
				break;
			}
			i++;	// initial node je 0. prvek, proto vkl�d�m od 1

//			%Random state
			unordered_map<Uav, shared_ptr<Point>, UavHasher> s_rand = random_state_guided(guiding_paths, map, nearState); // vr�t� pole n�hodn�ch bod�, jeden pro ka�dou kvadrokopt�ru

			//Finding appropriate nearest neighbor
			int k = 1;	//po��tadlo nepou�iteln�ch nodes, proto�e se pou��v� v nearest_neighbor size() - k, za��n�m od 1, znamen� to, �e se pou�ije k-t� nejbli��� node
			bool near_found = false;

			auto isNewUavPosition = false;
			//opakov�n�, dokud nenajdu vyhovuj�c� �e�en�, po��taj� se pr�chody cyklem kv�li uv�znut�
			while (!near_found)
			{
				if (k > near_count)
				{
//					i--;	//todo: zjistit, pro� se sni�uje i o 1
//					throw "Not possible to find near node suitable for expansion";
					cout << "Not possible to find near node suitable for expansion";
				}
				nearState = nearest_neighbor(s_rand, nodes, k);
				newState = select_input(s_rand, nearState, map);
				// Vypad� to, �e near_node je ve funkci select_input zm�n�n� kv�li kontrole p�ek�ek

				bool allInputsUsed = nearState->areAllInputsUsed();

				auto isNearUavPosition = false;
				for (auto uavPosition : nearState->uavs)
				{
					isNearUavPosition = isNearUavPosition || uavPosition != false;
				}

				isNewUavPosition = newState != false;	//pointer je empty , pokud se pro UAV nena�la vhodn� dal�� pozice

				//po��tadlo uv�znut�. UAV uv�zlo, pokud je tento if true
				if (allInputsUsed || !isNewUavPosition || !isNearUavPosition)	//kontrola empty new_node
				{
					k++;
					check_expandability(nodes);
				} else
				{
					near_found = true;
				}
			}

			if (!isNewUavPosition)	//spust� se v p��pad�, �e se nedorazilo do c�le a nena�la se ��dn� cesta
			{
				check_expandability(nodes);
				logger->logText("NaN in new node");
				break;
			}

			newState->index = i;
			if (debug)
			{
				logger->logText("[debug] Added node index: " + to_string(newState->index));
			}

			if (i % 200 == 0)
			{
				char buffer[20];
				sprintf(buffer, "RRT size: %d\n", i);
				logger->logText(buffer);
			}

			nodes.push_back(newState);
			s++;

			guiding_point_reached(newState, guiding_paths, guiding_near_dist); // zde se ulo�� do current_index, kolik nodes zb�v� dan�mu UAV do c�le
			check_near_goal(newState->uavs, map);

			bool isGoalReached = true;
			for (auto uav : newState->uavs)
			{
				isGoalReached = isGoalReached && uav->isGoalReached();
			}

			output->distance_of_new_nodes = vector<int>(nodes.size());
			if (isGoalReached) // pokud je nalezen c�l
			{
				output->goal_reached = vector<shared_ptr<Goal>>();
				for(auto uav : newState->uavs)
				{
					output->goal_reached.push_back(uav->getReachedGoal());
				}
				final_nodes[m] = newState;	//rekurz� se ze stavu d� z�skat cel� cesta
				char buffer[1024];
				sprintf(buffer, "%d viable paths found so far.\n", m);
				logger->logText(buffer);
				m++;
			}
			output->distance_of_new_nodes[i] = distance_of_new_nodes;

			if (i % configuration->getDrawPeriod() == 0)
			{
				logger->logNewState(nearState, newState);
				logger->logRandomStates(s_rand);
			}
		}
		
		final_nodes[m] = nodes[nodes.size() - 1];	//posledn� prvek
		check_near_goal(newState->uavs, map);
		output->goal_reached = vector<shared_ptr<Goal>>();
		for (auto uav : newState->uavs)
		{
			output->goal_reached.push_back(uav->getReachedGoal());
		}
		//todo: o�et�it nodes a final_nodes proti nullpointer�m a vyh�zet null nody
		output->nodes = nodes;
		logger->logText("RRT-Path finished");
	}

	unordered_map<Uav, shared_ptr<Point>, UavHasher> Core::random_state_guided(vector<shared_ptr<Path>> guiding_paths, shared_ptr<Map> map, shared_ptr<State> state)
	{
		double guided_sampling_prob = configuration->getGuidedSamplingPropability();
		int worldWidth = configuration->getWorldWidth();
		int worldHeight = configuration->getWorldHeight();
		int number_of_uavs = map->getUavsStart().size();
		unordered_map<Uav, shared_ptr<Point>, UavHasher> randomStates;

		valarray<double> propabilities = valarray<double>(guiding_paths.size());	//tohle nakonec v�bec nen� pou�ito, proto�e se cesty ur�ily p�esn�. 
		//todo: vy�e�it probl�m s t�m, �e krat�� cesta je prozkoum�na d��ve

		int sum = 0; // sum = celkov� d�lka v�ech vedouc�ch cest

		// del�� cesta m� v�t�� prpst.proto, aby algoritmus asymptiticky pokryl ka�dou cestu stejn� hust�. na del�� cestu tedy p�ipadne v�ce bod�.
		for (size_t i = 0; i < guiding_paths.size(); i++)
		{
			int pathSize = guiding_paths[i]->getSize(); // ��m del�� cesta, t�m v�t�� pravd�podobnost, �e se tou cestou vyd�, aspo� douf�m
			propabilities[i] = pathSize;		// propabilities je norm�ln�, 1D pole. nech�pu, pro� tady pou��v� notaci pro 2D pole
			sum += pathSize;
		}
		
		propabilities /= sum;

		double random = Random::fromZeroToOne();
		if (random > guided_sampling_prob) //vyb�r� se n�hodn� vzorek
		{
			int index = 0;
			for (auto uav : state->uavs)
			{
				if (uav->isGoalReached())
				{//todo: s t�mhle n�co ud�lat, a nep�istupovat k poli takhle teple p�es indexy
					randomStates[*uav.get()] = random_state_goal(uav->getReachedGoal(), map);	//pokud je n-t� UAV v c�li, vybere se n�hodn� bod z c�lov� plochy, kam UAV dorazilo
				}
				else
				{
					randomStates[*uav.get()] = random_state(0, 0, worldWidth, worldHeight, map); // pokud n-t� UAV nen� v c�li, vybere se n�hodn� bod z cel� mapy
				}
			}
		} else
		{
			//rozd�lit kvadrokopt�ry na skupinky. Jedna skupina pro ka�d� AoI. Rozd�lit skupiny podle plochy, kterou jednotkliv� AoI zab�raj�
			valarray<double> ratios = valarray<double>(map->getGoals().size()); //pom�ry jednotliv�ch ploch ku celkov� plo�e. Dlouh� jako po�et c�l�, tedy po�et guiding paths
			double totalVolume = 0;
			for (size_t i = 0; i < map->getGoals().size(); i++)
			{
				double volume = map->getGoals()[i]->rectangle->getVolume();
				ratios[i] = volume;
				totalVolume += volume;
			}

			ratios /= totalVolume;	//valarray umo��uje vektorov� operace, ka�d� prvek je v rozsahu od 0 do 1

			vector<shared_ptr<UavGroup>> uavGroups = vector<shared_ptr<UavGroup>>(map->getGoals().size());

			//p�erozd�lov�n� kvadrokopt�r podle pom�ru inspirov�no t�mto https://github.com/sebastianbergmann/money/blob/master/src/Money.php#L261

			int uavsInGroups = 0;	//po��t�, kolik uav je rozvr�en�ch do skupin, kv�li zaokrouhlov�n�
			for (size_t i = 0; i < uavGroups.size(); i++)
			{
				int uavsCountInGroup = floor(number_of_uavs * ratios[i]);	//round down
				auto uavs = vector<shared_ptr<Uav>>(uavsCountInGroup);
				for (size_t j = 0; j < uavsCountInGroup; j++)
				{
					uavs[j] = state->uavs[uavsInGroups + j];	//todo: tuhle ��st asi zrefaktorovat. A n�kde m�t objekty reprezentuj�c� uav, s jeho polohou, apod.
				}
				uavGroups[i] = make_shared<UavGroup>(uavs, guiding_paths[i]);
				uavsInGroups += uavsCountInGroup;
			}
			//rozh�zet do skupin nep�i�azen� uav, kter� zbyla kv�li zaokrouhlov�n� dol�
			int remaining = map->getUavsStart().size() - uavsInGroups;
			for (size_t i = 0; i < remaining; i++)	//zbytku je v�dycky stejn� nebo m�n� ne� po�tu skupin
			{
				uavGroups[i]->addUav(state->uavs[uavsInGroups + i]);
			}

			for (auto group : uavGroups)
			{
				//te� je v groupCurrentIndexes current_index pro ka�d� UAV pro danou path z dan� group
				auto center = group->getBestNode();
				logger->logRandomStatesCenter(center->getPoint());
				for (auto uav : group->getUavs())
				{
					if (uav->isGoalReached())
					{
						randomStates[*uav.get()] = random_state_goal(uav->getReachedGoal(), map);
					}
					else
					{
						randomStates[*uav.get()] = random_state_polar(center->getPoint(), map, 0, configuration->getSamplingRadius());
					}
				}
			}
		}

		//p�eskl�d�n� randomStates podle ID UAV.
		vector<int> uavIds = vector<int>(randomStates.size());
		unordered_map<Uav, shared_ptr<Point>, UavHasher> randomStatesSorted;
		int index = 0;
		for (auto pair : randomStates)
		{
			auto uav = pair.first;
			uavIds[index] = uav.getId();
			index++;
		}
		sort(uavIds.begin(), uavIds.end());
		for(int uavId : uavIds)
		{
			//nalezen� uav s dan�m id
			for (auto pair : randomStates)
			{
				auto uav = pair.first;
				if (uav.getId() == uavId)
				{
					randomStatesSorted[uav] = randomStates[uav];
					break;
				}
			}
		}

		return randomStatesSorted;
	}

	shared_ptr<State> Core::nearest_neighbor(unordered_map<Uav, shared_ptr<Point>, UavHasher> s_rand, vector<shared_ptr<State>> nodes, int count)
	{
		int max_nodes = configuration->getRrtMaxNodes();
		int debug = configuration->getDebug();
		NNMethod nn_method = configuration->getNearestNeighborMethod();

		vector<shared_ptr<State>> near_arr = vector<shared_ptr<State>>();
		shared_ptr<State> near_node;
		vector<double> stateDistances = vector<double>(nodes.size());	//celkov� vzd�lenost pro dan� State, ukl�d�m tam hamilt_dist, zat�m pouze pro debug, nikde se nepou��v�

		int s = 1;
		double current_best = DBL_MAX;
		
		for (int j = 0; j < nodes.size(); j++)
		{
			// Distance of next node in the tree
			shared_ptr<State> tmp_node = nodes[j];

			if (tmp_node->areAllInputsUsed())
			{
				char buffer[1024];
				sprintf(buffer, "Node %d is unexpandable\n", tmp_node->index);
				logger->logText(buffer);
				continue;
			}
			
			double hamilt_dist = 0;
			vector<double> distances = vector<double>(tmp_node->uavs.size());

			for (size_t i = 0; i < tmp_node->uavs.size(); i++)
			{
				auto uav = tmp_node->uavs[i];
				auto randomState = s_rand[*uav.get()];
				distances[i] = uav->getPointParticle()->getLocation()->getDistanceSquared(randomState);
			}

			switch (nn_method)
			{
			case NNMethod::Total:
				for(auto dist : distances)
				{
					hamilt_dist += dist;	//no function for sum, so I must do it by hand
				}
				break;
			case NNMethod::Max:
				hamilt_dist = *max_element(distances.begin(), distances.end());	//tohle vrac� iter�tor, kter� mus�m dereferencovat, abych z�skal ��slo. fuck you, C++
				break;
			case NNMethod::Min:
				hamilt_dist = *min_element(distances.begin(), distances.end());	//tohle vrac� iter�tor, kter� mus�m dereferencovat, abych z�skal ��slo. fuck you, C++
				break;
			}

			stateDistances[j] = hamilt_dist;
			
			//Check if tested node is nearer than the current nearest
			if (hamilt_dist < current_best)
			{
				near_node = tmp_node;
				near_arr.push_back(near_node);
				current_best = hamilt_dist;
				if (debug)
				{
					double distance;
					for (auto uav : tmp_node->uavs)
					{
						auto randomState = s_rand[*uav.get()];	
						distance = uav->getPointParticle()->getLocation()->getDistanceSquared(randomState);
					}
					char buffer[1024];
					sprintf(buffer, "[debug] near node #%d found, distance to goal state: %f\n", s, distance);
					logger->logText(buffer);
				}
				s++;
			}			
		}
			
		if (near_arr.size() >= count)	//todo: zjistit, zda je spr�vn� nerovnost a zda nem� b�t >=
		{
			near_node = near_arr[near_arr.size() - count];	//indexuje se od 0, proto count mus� za��nat od 1
			double distance;
			for (auto uav : near_node->uavs)
			{
				auto randomState = s_rand[*uav.get()];
				distance = uav->getPointParticle()->getLocation()->getDistanceSquared(randomState);
			}
			if (debug && count > 0)
			{
				char buffer[1024];
				sprintf(buffer, "[debug] near node #%d chosen, %d discarded, near node index %d, distance to goal state: %f\n", near_arr.size() - count, count, near_node->index, distance);
				logger->logText(buffer);
			}
		}

		if (!near_node)
		{
			throw "No suitable near node found";
		}

		return near_node;
	}

	shared_ptr<State> Core::select_input(unordered_map<Uav, shared_ptr<Point>, UavHasher> s_rand, shared_ptr<State> near_node, shared_ptr<Map> map)
	{
//		file << "Near node: " << *near_node.get() << endl;
//		file << "s_rand";
//		for (auto a : s_rand)
//		{
//			file << *a.get() << endl;
//		}

		int input_samples_dist = configuration->getInputSamplesDist();
		int input_samples_phi = configuration->getInputSamplesPhi();
		int distance_of_new_nodes = configuration->getDistanceOfNewNodes();
		double max_turn = configuration->getMaxTurn();
		bool relative_localization = true;	//zat�m natvrdo, proto�e nev�m, jak se m� chovat druh� mo�nost
		int uavCount = near_node->uavs.size();
		int inputCount = configuration->getInputCount();
		vector<shared_ptr<Point>> oneUavInputs = vector<shared_ptr<Point>>();
		shared_ptr<State> new_node;

		for (size_t k = 0; k < input_samples_dist; k++)
		{
			for (size_t m = 0; m < input_samples_phi; m++)
			{
				double x = distance_of_new_nodes / pow(1.5,k);
				double y = -max_turn + 2 * m * max_turn / (input_samples_phi - 1);
				shared_ptr<Point> point = make_shared<Point>(x, y);
				oneUavInputs.push_back(point);
			}
		}

		//po�et v�ech mo�n�ch "kombinac�" je variace s opakov�n�m (n-tuple anglicky). 
		//inputs jsou vstupy do modelu
		vector<unordered_map<Uav, shared_ptr<Point>, UavHasher>> inputs = generateNTuplet<shared_ptr<Point>>(oneUavInputs, near_node->uavs, uavCount - 1);	//po�et v�ech kombinac� je po�et v�ech mo�n�ch vstup� jednoho UAV ^ po�et UAV
																												//translations jsou v�stupy z modelu
		vector<unordered_map<Uav, shared_ptr<Point>, UavHasher>> translations = vector<unordered_map<Uav, shared_ptr<Point>, UavHasher>>(inputCount);	//po�et v�ech kombinac� je po�et v�ech mo�n�ch vstup� jednoho UAV ^ po�et UAV
		vector<shared_ptr<State>> tempStates = vector<shared_ptr<State>>(inputCount);	//po�et v�ech kombinac� je po�et v�ech mo�n�ch vstup� jednoho UAV ^ po�et UAV

		for (size_t i = 0; i < inputs.size(); i++)
		{
			auto input = inputs[i];
			auto tempState = car_like_motion_model(near_node, input);	//this method changes near_node
			tempStates[i] = tempState;
			for (auto uav : tempState->uavs)
			{
				double x = uav->getPointParticle()->getLocation()->getX() - near_node->getUav(uav)->getPointParticle()->getLocation()->getX();
				double y = uav->getPointParticle()->getLocation()->getY() - near_node->getUav(uav)->getPointParticle()->getLocation()->getY();
				translations[i][*uav.get()] = make_shared<Point>(x ,y);
			}
		}

		vector<double> d = vector<double>(inputCount);
		//todo: mo�n� zrefactorovat a schovat do jednoho cyklu, kter� je v��e
		//todo: kde to p�jde, pu��t range-based loop, iteraci m�sto klasick�ho foru
		//Distance to s_rand when using different inputs
		for (size_t i = 0; i < d.size(); i++)
		{
			auto tempState = tempStates[i];
			d[i] = 0;
			for (auto uav : tempState->uavs)
			{
				d[i] += s_rand[*uav.get()]->getDistance(uav->getPointParticle()->getLocation());
			}
		}

		
		// Find vector with minimal distance to s_rand and return it
		if (relative_localization)
		{
			int m = 0;
			while (m < inputCount)
			{
				m++;
				if (near_node->areAllInputsUsed())
				{
//					throw "No valid input left";
					break;	//v�jimka se nem� vyhazovat
				}

				//todo: pokud mo�no, refactorovat, a� nemus�m minimum hledat ru�n�
				int index = 0;	//kl�� nejmen�� hodnoty vzd�lenosti v poli d
				double minValue = DBL_MAX;
				for (size_t i = 0; i < d.size(); i++)
				{
					if (d[i] < minValue)
					{
						minValue = d[i];
						index = i;
					}
				}
				auto tempState = tempStates[index];

				if (!check_localization_sep(tempState) || trajectory_intersection(near_node, tempState) || near_node->used_inputs[index])
				{
					d[index] = DBL_MAX; //jde o to vy�adit tuto hodnotu z hled�n� minima
					continue;
				}

				if (!check_world_bounds(tempState->uavs, configuration->getWorldWidth(), configuration->getWorldHeight()))
				{
					d[index] = DBL_MAX; //jde o to vy�adit tuto hodnotu z hled�n� minima
					continue;
				}

				check_obstacle_vcollide_single(near_node, translations, index, map);	//this fills field used_inputs by true and thus new state looks like already used

				if (near_node->used_inputs[index])
				{
					d[index] = DBL_MAX; //jde o to vy�adit tuto hodnotu z hled�n� minima
					continue;
				} else
				{
					new_node = stateFactory->createState();
					new_node->uavs = tempState->uavs;
					new_node->prev = near_node;
					new_node->prev_inputs = tempState->prev_inputs;
					near_node->used_inputs[index] = true;
					break;
				}
			}
		} else
		{
			//todo: rozhodnout, zda to tady chci, nebo zda bude natvrdo zapnut� relativn� lokalizace
			//v�bec nech�pu, co tady d�l� m. Je �pln� out odf scope, z�st�v� na posledn� hodnot� for cyklu.
//		    if ~near_node.used_inputs(m,1)
//		        index = find(d(:) == min(d(:)),1);
//		        tmp_node.loc = tmp_nodes(index(1,n)).loc;
//		        tmp_node.rot = tmp_nodes(index(1,n)).rot;
//		        new_node = tmp_node;
//		        new_node.prev = near_node.index;
//		        new_node.prev_inputs(:,:) = tmp_nodes(index(1,n).prev_inputs(:,:));
//		        near_node.used_inputs(index,1) = true;
//		    end
		}		
		
		return new_node;
	}

	int Core::check_expandability(vector<shared_ptr<State>> nodes)
	{
		int unexpandable_count = 0;
		for (auto node : nodes)
		{
			if (node->areAllInputsUsed())
			{
				unexpandable_count++;
			}
		}

		if (unexpandable_count > 0) {
			char buffer[1024];
			sprintf(buffer, "Not expandable nodes: %d/%d\n", unexpandable_count, nodes.size());
			logger->logText(buffer);
		}
		return unexpandable_count;
	}

	//detects narrow passage
	void Core::guiding_point_reached(shared_ptr<State> state, vector<shared_ptr<Path>> guiding_paths, int guiding_near_dist)
	{
		for (auto guiding_path : guiding_paths) {
			for (auto node : guiding_path->getNodes())	//todo: mo�n� to p�ed�lat a iterovat obr�cen�, abych jel odzadu a ud�lal break, kdy� naraz�m na currentPoint u uav, abych nemusel kontrolovat isFirstCloserToEnd
			{
				for (auto uav : state->uavs)
				{
					bool reached = false;
					if (uav->getPointParticle()->getLocation()->getDistanceSquared(node->getPoint()) < pow(guiding_near_dist, 2))
					{
						reached = true;
						//Narrow passage detection
						detect_narrow_passage(node);
					}
					auto uavCurrentPoint = uav->current_indexes->get(guiding_path);
					if (reached && guiding_path->hasNext(uavCurrentPoint) && guiding_path->isFirstCloserOrSameToEnd(node, uavCurrentPoint)) //o�et�en�, aby se UAV nevracela, a by nep�etekl index u guidingPath
					{
						uav->current_indexes->set(guiding_path, guiding_path->getNext(uavCurrentPoint));
						break;
					}
				}
			}
		}

	}

	void Core::check_near_goal(vector<shared_ptr<Uav>> uavs, shared_ptr<Map> map)
	{
		for (auto goal : map->getGoals())
		{
			for (auto uav : uavs)
			{
				if (goal->is_near(uav->getPointParticle()->getLocation()))
				{
					uav->setReachedGoal(goal);
				}
			}
		}
	}

	void Core::detect_narrow_passage(shared_ptr<Node> node)
	{
		//todo: vymyslet, jak to ud�lat, abych odsud nastavil parametry, ale nesahal na celou konfiguraci. ud�lat asi n�jak� command na d�len� np_divisorem

//		global params defaults count
		if (node->getCost() > 1) {
//			params.distance_of_new_nodes = defaults.distance_of_new_nodes / params.np_divisor;
//			params.max_turn = defaults.max_turn * params.np_divisor;
//			params.guiding_near_dist = defaults.guiding_near_dist / params.np_divisor;
//			count = 0;
//			// params.sampling_radius = defaults.sampling_radius / 3;
		} else {
//			count = count + 1;
//		
//			//Far from obstacle
//				if count > params.exit_np_threshold
//					params.distance_of_new_nodes = defaults.distance_of_new_nodes;
//					params.max_turn = defaults.max_turn;
//					params.guiding_near_dist = defaults.guiding_near_dist;
//				count = 0;
//				end
//		// params.sampling_radius = defaults.sampling_radius;
		}

	}

	shared_ptr<Point> Core::random_state_goal(shared_ptr<Goal> goal, shared_ptr<Map> map)
	{
		return random_state(goal->rectangle, map);
	}

	shared_ptr<Point> Core::random_state(shared_ptr<Rectangle> rectangle, shared_ptr<Map> map)
	{
		return random_state(rectangle->getX(), rectangle->getY(), rectangle->getX() + rectangle->getWidth(), rectangle->getY() + rectangle->getHeight(), map);
	}

	shared_ptr<Point> Core::random_state(int x1, int y1, int x2, int y2, shared_ptr<Map> map)
	{
		shared_ptr<Point> random_state;
		do
		{
			int x = Random::inRange(x1, x2);
			int y = Random::inRange(y1, y2);
			random_state = make_shared<Point>(x, y);
		} while (check_inside_obstacle(random_state, map));
		return random_state;
	}

	//returns true if point is inside of obstacle
	bool Core::check_inside_obstacle(shared_ptr<Point> point, shared_ptr<Map> map)
	{
		bool collision = false;

		for (auto obstacle : map->getObstacles())
		{
			collision = collision || obstacle->rectangle->contains(point);
		}
		return collision;
	}

	shared_ptr<Point> Core::random_state_polar(shared_ptr<Point> center, shared_ptr<Map> map, double radius_min, double radius_max)
	{
		shared_ptr<Point> randomState;
		do
		{
			double phi = Random::inRange(0, 2 * PI);
			double r = radius_min + Random::inRange(radius_min, radius_max);
			double x = center->getX() + r*cos(phi);
			double y = center->getY() + r*sin(phi);
			randomState = make_shared<Point>(x, y);
		} while (check_inside_obstacle(randomState, map) && check_world_bounds(randomState, configuration->getWorldWidth(), configuration->getWorldHeight()));
		return randomState;
	}

	bool Core::check_world_bounds(shared_ptr<Point> point, int worldWidth, int worldHeight)
	{
		bool inBounds = false;

		if (point->getX() < worldWidth && point->getX() > 0 && point->getY() < worldHeight && point->getY() > 0)
		{
			inBounds = true;
		}
		return inBounds;

	}

	bool Core::check_world_bounds(vector<shared_ptr<Uav>> points, int worldWidth, int worldHeight)
	{
		bool inBounds = false;
		for (auto point : points)
		{
			inBounds = inBounds || check_world_bounds(point->getPointParticle()->getLocation(), worldWidth, worldHeight);
		}
		return inBounds;
	}

	//only modifies node by inputs
	shared_ptr<State> Core::car_like_motion_model(shared_ptr<State> node, unordered_map<Uav, shared_ptr<Point>, UavHasher> inputs)
	{
		auto newNode = make_shared<State>(*node.get());	//copy constructor, deep copy

		double uav_size = configuration->getUavSize();
		// Simulation step length
		double time_step = configuration->getTimeStep();
		// Simulation length
		double end_time = configuration->getEndTime();
		int number_of_uavs = node->uavs.size();
		//		global number_of_uavs params empty_trajectory
		
		//model parameters
		//front to rear axle distance
		double L = uav_size;
		
		//prepare trajectory array
		//repmat is time demanding
		auto trajectory = vector<shared_ptr<State>>();
		
		//main simulation loop
		//todo: v�ude, kde pou��v�m push_back se pod�vat, zda by ne�lo na za��tku naalokovat pole, aby se nemusela dynamicky m�nit velikost

		for (auto uav : newNode->uavs)
		{
			auto uavPointParticle = uav->getPointParticle();
			double dPhi = (inputs[*uav.get()]->getX() / L) * tan(inputs[*uav.get()]->getY());	//dPhi se nem�n� v r�mci vnit�n�ho cyklu, tak�e sta�� spo��tat jen jednou

			for (double i = time_step; i < end_time; i += time_step)
			{
				//calculate derivatives from inputs
				double dx = inputs[*uav.get()]->getX() * cos(uavPointParticle->getRotation()->getZ());	//pokud jsme ve 2D, pak jedin� mo�n� rotace je rotace okolo osy Z
				double dy = inputs[*uav.get()]->getX() * sin(uavPointParticle->getRotation()->getZ());	//input nen� klasick� bod se sou�adnicemi X, Y, ale objekt se dv�ma ��sly, odpov�daj�c�mi dv�ma vstup�m do car_like modelu

				//calculate current state variables
				uavPointParticle->getLocation()->changeX(dx * time_step);
				uavPointParticle->getLocation()->changeY(dy * time_step);
				uavPointParticle->getRotation()->changeZ(dPhi * time_step);
			}
			newNode->prev_inputs = inputs;
			trajectory.push_back(newNode);
		}

		return newNode;
	}

	bool Core::check_localization_sep(shared_ptr<State> node)
	{
		int number_of_uavs = node->uavs.size();
		double relative_distance_min = configuration->getRelativeDistanceMin();
		double relative_distance_max = configuration->getRelativeDistanceMax();
		bool check_fov = configuration->getCheckFov();
		// Neighbor must be in certain angle on / off
		double localization_angle = configuration->getLocalizationAngle();
		int required_neighbors = configuration->getRequiredNeighbors();
		bool allow_swarm_splitting = configuration->getAllowSwarmSplitting();

		// Initialize default values
		vector<int> neighbors = vector<int>(number_of_uavs);
		fill(neighbors.begin(), neighbors.end(), 0);	//inicializace
		
		// Single UAV needs no localization
		if (number_of_uavs == 1)
		{
			return true;
		}
		
		// Check minimal distance between UAVs
		for (size_t i = 0; i < number_of_uavs - 1; i++)	//todo: zkontrolovat indexy, zda spr�vn� sed� a neut�kaj� o 1
		{
			for (size_t j = i + 1; j < number_of_uavs; j++)
			{
				double uavIx = node->uavs[i]->getPointParticle()->getLocation()->getX();
				double uavIy = node->uavs[i]->getPointParticle()->getLocation()->getY();
				double uavJx = node->uavs[j]->getPointParticle()->getLocation()->getX();
				double uavJy = node->uavs[j]->getPointParticle()->getLocation()->getY();

				if (sqrt(pow(uavIx - uavJx, 2) + pow(uavIy - uavJy, 2)) <= relative_distance_min)
				{
					return false;
				}
			}
		}

		// Check maximal distance between UAVs
		for (size_t i = 0; i < number_of_uavs - 1; i++)	//todo: zkontrolovat indexy, zda spr�vn� sed� a neut�kaj� o 1
		{
			for (size_t j = i + 1; j < number_of_uavs; j++)
			{
				double uavIx = node->uavs[i]->getPointParticle()->getLocation()->getX();
				double uavIy = node->uavs[i]->getPointParticle()->getLocation()->getY();
				double uavIphi = node->uavs[i]->getPointParticle()->getRotation()->getZ();
				double uavJx = node->uavs[j]->getPointParticle()->getLocation()->getX();
				double uavJy = node->uavs[j]->getPointParticle()->getLocation()->getY();
				double uavJphi = node->uavs[i]->getPointParticle()->getRotation()->getZ();

				if (sqrt(pow(uavIx - uavJx, 2) + pow(uavIy - uavJy, 2)) < relative_distance_max && (!check_fov || abs(uavIphi - uavJphi) < localization_angle / 2))
				{
					neighbors[i]++;
					neighbors[j]++;					
				}
			}
		}

		bool allUavsHaveNeighbors = false;
		bool oneOrMoreNeighbors = false;
		for (auto neighbor : neighbors)
		{
			allUavsHaveNeighbors = allUavsHaveNeighbors || neighbor >= required_neighbors;
			oneOrMoreNeighbors = oneOrMoreNeighbors || neighbor >= 1;
		}

		// Check whether each UAV has required number of neighbors
		// Pair formations
		if (allow_swarm_splitting)
		{
			return allUavsHaveNeighbors;
		} else
		// Whole swarm
		{
			int twoOrMoreNeighbors = 0;
			if (oneOrMoreNeighbors)
			{
				char buffer[1024];
				sprintf(buffer, "Neigbors: %d %d %d %d \n", neighbors[0], neighbors[1], neighbors[2], neighbors[3]);
				logger->logText(buffer);
				for (auto neighbor : neighbors)
				{
					neighbor > 1 ? twoOrMoreNeighbors++ : NULL;
				}
				return twoOrMoreNeighbors >= number_of_uavs - 2;
			}

		}
		return false;
	}

	bool Core::trajectory_intersection(shared_ptr<State> near_node, shared_ptr<State> tmp_node)
	{
		
		int number_of_uavs = near_node->uavs.size();
		for (size_t i = 0; i < number_of_uavs; i++)
		{
			for (size_t j = 0; j < number_of_uavs; j++)
			{
				if (i != j && line_segments_intersection(
					near_node->uavs[i]->getPointParticle()->getLocation(),
					tmp_node->uavs[i]->getPointParticle()->getLocation(),
					near_node->uavs[j]->getPointParticle()->getLocation(),
					tmp_node->uavs[j]->getPointParticle()->getLocation()))
				{
					return true;
				}
			}
		}
		return false;
	}

	void Core::check_obstacle_vcollide_single(shared_ptr<State> near_node, vector<unordered_map<Uav, shared_ptr<Point>, UavHasher>> translation, int index, shared_ptr<Map> map)
	{
		double uav_size = configuration->getUavSize();
				
		double zero_trans[] = { 0,0,0, 1,0,0, 0,1,0, 0,0,1 };
				
		int k = index;
		for (auto uav : near_node->uavs)
		{
			double x = uav->getPointParticle()->getLocation()->getX();
			double y = uav->getPointParticle()->getLocation()->getY();
			double x1 = x - uav_size / 2;
			double y1 = y - uav_size / 2;
			double z1 = 1;
			double x2 = x + uav_size / 2;
			double y2 = y - uav_size / 2;
			double z2 = 1;
			double x3 = x;
			double y3 = y + uav_size / 2;
			double z3 = 1;
			Triangle3D tri_uav = Triangle3D(Point3D(x1, y1, z1), Point3D(x2, y2, z2), Point3D(x3, y3, z3));

			for (auto obs : map->getObstacles())
			{
				Point3D p1 = Point3D(obs->rectangle->getX(), obs->rectangle->getY(), 1);
				Point3D p2 = Point3D(obs->rectangle->getX() + obs->rectangle->getWidth(), obs->rectangle->getY(), 1);
				Point3D p3 = Point3D(obs->rectangle->getX() + obs->rectangle->getWidth(), obs->rectangle->getY() + obs->rectangle->getHeight(), 1);
				Point3D p4 = Point3D(obs->rectangle->getX(), obs->rectangle->getY() + obs->rectangle->getHeight(), 1);

				Triangle3D tri1_obs = Triangle3D(p1, p2, p3);
				Triangle3D tri2_obs = Triangle3D(p1, p4, p3);


				double trans[] = { translation[k][*uav.get()]->getX(), translation[k][*uav.get()]->getY(), 0, 1, 0, 0, 0, 1, 0, 0, 0, 1 };
				bool col = ColDetect::coldetect(tri_uav, tri1_obs, trans, zero_trans);
				col = col || ColDetect::coldetect(tri_uav, tri2_obs, trans, zero_trans);
				if (col)
				{
					for (size_t l = 0; l < translation.size(); l++)
					{
						if (translation[l][*uav.get()]->getY() == translation[k][*uav.get()]->getY() && translation[l][*uav.get()]->getX() == translation[k][*uav.get()]->getX())
						{
							near_node->used_inputs[l] = true;
						}
					}
					near_node->used_inputs[k] = true;
				}
			}
		}
	}

	bool Core::line_segments_intersection(shared_ptr<Point> p1, shared_ptr<Point> p2, shared_ptr<Point> p3, shared_ptr<Point> p4)
	{
		auto p = line_line_intersection(p1, p2, p3, p4);
		
		if (isfinite(p->getX()) && isfinite(p->getY()))
		{
			if (line_point_intersection(p, p1, p2) &&
				line_point_intersection(p, p3, p4))
			{
				return true;
			}

		}
		return false;
	}

	bool Core::line_point_intersection(shared_ptr<Point> q, shared_ptr<Point> p1, shared_ptr<Point> p2)
	{
		double tolerance = 1e-10;

		double x = q->getX();
		double y = q->getY();
		double x1 = p1->getX();
		double y1 = p1->getY();
		double x2 = p2->getX();
		double y2 = p2->getY();

		double value = (pow(x - x1, 2) + pow(y - y1, 2)) + (pow(x - x2, 2) + pow(y - y2, 2)) - (pow(x1 - x2, 2) + pow(y1 - y2, 2));

		return value <= tolerance;
	}

	shared_ptr<Point> Core::line_line_intersection(shared_ptr<Point> p1, shared_ptr<Point> p2, shared_ptr<Point> p3, shared_ptr<Point> p4)
	{
		double x1 = p1->getX();
		double y1 = p1->getY();
		double x2 = p2->getX();
		double y2 = p2->getY();
		double x3 = p3->getX();
		double y3 = p3->getY();
		double x4 = p4->getX();
		double y4 = p4->getY();

		double px = ((x1*y2 - y1*x2)*(x3 - x4) - (x1 - x2)*(x3*y4 - y3*x4)) / 
			((x1 - x2)*(y3 - y4) - (y1 - y2)*(x3 - x4));
		double py = ((x1*y2 - y1*x2)*(y3 - y4) - (y1 - y2)*(x3*y4 - y3*x4)) / 
			((x1 - x2)*(y3 - y4) - (y1 - y2)*(x3 - x4));

		return make_shared<Point>(px, py);
	}

	// Zde je java implementace, ze kter� vych�z�m https ://cs.wikipedia.org/wiki/Variace_(algoritmus)
	template<typename T> vector<vector<T>> Core::generateNTuplet(vector<T> usedChars, int tupletClass)
	{
		vector<vector<T>> list = vector<vector<T>>();	//todo: pop�em��let, jak to refactorovat, aby se mohlo pracovat s fixn� velikost�  pole
//		vector<vector<T>> list = vector<vector<T>>(pow(usedChars.size(), tupletClass));

		if(tupletClass == 0)
		{
			list.push_back(vector<T>());
		} else
		{
			vector<vector<T>> tuplet = generateNTuplet(usedChars, tupletClass - 1);
			for (T character : usedChars)
			{
				for (auto row : tuplet)
				{
					row.push_back(character);
					list.push_back(row);
				}
			}
		}
		return list;
	}

	template <typename T>
	vector<unordered_map<Uav, T, UavHasher>> Core::generateNTuplet(vector<T> usedChars, vector<shared_ptr<Uav>> tupletKeys, int index)
	{
		vector<unordered_map<Uav, T, UavHasher>> list = vector<unordered_map<Uav, T, UavHasher>>();	//todo: pop�em��let, jak to refactorovat, aby se mohlo pracovat s fixn� velikost�  pole
														//		vector<vector<T>> list = vector<vector<T>>(pow(usedChars.size(), tupletClass));
		if (index < 0)
		{
			list.push_back(unordered_map<Uav, T, UavHasher>());
		}
		else
		{
			vector<unordered_map<Uav, T, UavHasher>> tuplet = generateNTuplet(usedChars, tupletKeys, index - 1);
			for (T character : usedChars)
			{
				for (auto row : tuplet)
				{
					row[*tupletKeys[index].get()] = character;
					list.push_back(row);
				}
			}
		}
		return list;
	}

}
