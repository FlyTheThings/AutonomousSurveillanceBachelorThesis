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
#include <memory>
#include <string>

using namespace std;

namespace App
{

	Core::Core(shared_ptr<Configuration> configuration) :
		logger(make_shared<LoggerInterface>()), configuration(configuration)
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

		rrtPath(paths, configuration);
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

	void Core::rrtPath(vector<shared_ptr<Path>> guiding_paths, shared_ptr<Configuration> configuration)
	{
		int uavCount = configuration->getUavCount();
//		global params number_of_uavs empty_node nodes goals goal_map stop output

		cout << "Starting RRT-path...";

		// celkov� dr�ha v�ech cest dohromady, po�et nodes ve v�ech cest�ch
		// dohromady, evidentn� se ta prom�nn� nikde nepou��v�
		int gp_length = 0;
		for (int m = 0; m < guiding_paths.size(); m++)
		{
			gp_length += guiding_paths[m]->getSize();
		}
//		nodes = tree_init();	//dod�lat vlastn� inicializaci podle pot�eby
		auto current_index = vector<vector<int>>(uavCount);		// matice s d�lkami cest pro jednotliv� UAV.sloupec je cesta, ��dek je UAV

		for (int i = 0; i < current_index.size(); i++)
		{
			current_index[i] = vector<int>(guiding_paths.size());
			for (int j = 0; j < guiding_paths.size(); j++)
			{
				current_index[i][j] = guiding_paths[i]->getSize();
			}
		}

//		final_nodes = repmat(empty_node, 1, params.max_nodes);
//		goal_reached = NaN(1, number_of_uavs);
//		part_covered = [];
//
//		nodes_count = params.max_nodes;
//		tim = zeros(1, nodes_count - 1); %#ok % tahle prom�nn� se k ni�emu nepou��v�
//
//			i = 1; % po�et expandovan�ch nodes
//			m = 1; % po�et nalezen�ch cest
//			s = 2; % po�et pr�chod� cyklem ? prost� se to jen zv�t�� o 1 p�i ka�d�m pr�chodu, nikde se nepou��v�
//			tstart = tic; % tic je matlabovsk� timer http ://www.mathworks.com/help/matlab/ref/tic.html
//
//		while (m <= params.number_of_solutions || i < params.min_nodes) && i < params.max_nodes % numver_of_solutions je asi 10 000.
//			if stop
//				%        final_nodes(m) = new_node;
//		break
//			end
//			tv = tic; % tic je matlabovsk� timer
//			i = i + 1;
//		if all(~isnan(goal_reached)) % pokud jsou v�echna UAV uvnit� AoI a maj� se v r�mci AoI rozm�stit optim�ln�, rrt_path algoritmus u� skon�il
//			if params.pso_optimization  % pokud je vybr�na PSO fin�ln� ��st.jinak se pou�ije RRT fin�ln� ��st(mysl�m)
//				[final_node_index, particles] = pso([goals{ 1 }.x goals{ 1 }.x + goals{ 1 }.width; ...
//					goals{ 1 }.y goals{ 1 }.y + goals{ 1 }.height], ...
//					new_node);
//		final_nodes = particles(final_node_index(1), final_node_index(2));
//		nodes = nodes(1:(find(isnan([nodes.index]), 1) - 1));
//		nodes = [nodes particles(:, final_node_index(2))']; %#ok
//			return
//			else
//			end
//				end
//
//				%Random state
//				s_rand = random_state_guided(guiding_paths, current_index, goal_reached); % vr�t� pole n�hodn�ch bod�, jeden pro ka�dou kvadrokopt�ru
//
//				%Finding appropriate nearest neighbor
//				k = 0;
//		near_found = false;
//		while ~near_found
//			if k > params.near_count
//				i = i - 1;
//		disp('Not possible to find near node suitable for expansion');
//		break
//			end
//			[near_node] = nearest_neighbor(s_rand, nodes, k); % near_node v sob� obsahuje sou�adnice bl�zk�ch nodes pro v�echna UAV(tedy 4 r�zn� body, pokud m�m 4 UAV)
//			[near_node, new_node] = select_input(s_rand, near_node);    % zde je motion model, v new_node jsou op�t body pro v�echna UAV dosa�iteln� podle motion modelu.
//			% Vypad� to, �e near_node je ve funkci select_input zm�n�n� kv�li
//			% kontrole p�ek�ek
//			nodes(near_node.index) = near_node; % prom�nn� nodes je pole, kam se ukl�d� strom prohled�v�n� u RRT - Path
//			if all(near_node.used_inputs) || ...
//				any(isnan(new_node.loc(1, :))) || any(isnan(near_node.loc(1, :)))
//				k = k + 1;
//		check_expandability();
//		continue
//			end
//			near_found = true;
//		end
//			if any(isnan(new_node.loc(1, :)))
//				check_expandability();
//		disp('NaN in new node');
//		final_nodes(m) = nodes(i - 1);
//		break
//			end
//
//			new_node.index = i;
//		new_node.time_added = toc(tstart);
//		if params.debug
//			fprintf('[debug] Added node index: %d\n', new_node.index);
//		end
//
//			if mod(i, 200) == 0
//				fprintf('RRT size: %d\n', i);
//		end
//
//			nodes(i) = new_node;
//		s = s + 1;
//
//		current_index = guiding_point_reached(new_node, guiding_paths, current_index); % zde se ulo�� do current_index, kolik nodes zb�v� dan�mu UAV do c�le
//			goal_reached = check_near_goal(new_node.loc);
//		if all(~isnan(goal_reached)) % pokud je nalezen c�l
//			output.goal_reached = goal_reached;
//		final_nodes(m) = new_node;
//		fprintf('%d viable paths found so far.\n', m);
//		part_covered(1, end + 1) = goal_map.get_area_covered(); %#ok
//			goal_map.reset();
//		m = m + 1;
//		end
//			output.distance_of_new_nodes(1, i) = params.distance_of_new_nodes;
//		%     dt = toc(tv);
//		%     output.time_to_find_new_node(1, i) = dt;
//		%     output.iterations(1, i) = i;
//		%     try
//			%         output.time_running(1, i) = output.time_running(1, i - 1) + dt;
//		%     catch
//			%         output.time_running(1, i) = dt;
//		%     end
//			%     if output.time_running(1, i) > params.max_iteration_time
//			%         final_nodes(m) = new_node;
//		%         return
//			%     end
//			%     output.goal_reached = goal_reached;
//
//		%Visualize
//			if params.visualize == true && mod(i, params.draw_freq) == 0
//				visualize_growth(near_node.loc, new_node.loc, s_rand);
//		end
//
//			end
//			final_nodes(m) = nodes(i);
//		goal_reached = check_near_goal(new_node.loc);
//		output.goal_reached = goal_reached;
//		final_nodes = final_nodes(1:(find(isnan([final_nodes.index]), 1) - 1));
//		if any(isnan([nodes.index]))
//			nodes = nodes(1:(find(isnan([nodes.index]), 1) - 1));
//		end
//			output.nodes = nodes;
		cout << "RRT-Path finished";
	}
}
