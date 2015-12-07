#pragma once
#include <memory>
#include "State.h"

using namespace std;

namespace App
{
	class Configuration;

	class CollisionDetector
	{
	public:
		CollisionDetector(shared_ptr<Configuration> configuration);
		virtual ~CollisionDetector();
		bool areTrajectoriesIntersecting(shared_ptr<State> start, shared_ptr<State> end);
		bool areLineSegmentsIntersecting(shared_ptr<Point> p1, shared_ptr<Point> p2, shared_ptr<Point> p3, shared_ptr<Point> p4);
		bool LinePointIntersection(shared_ptr<Point> q, shared_ptr<Point> p1, shared_ptr<Point> p2);
		shared_ptr<Point> lineLineIntersection(shared_ptr<Point> p1, shared_ptr<Point> p2, shared_ptr<Point> p3, shared_ptr<Point> p4);
		pair<shared_ptr<Uav>, shared_ptr<Uav>> getIntersectingUavs(shared_ptr<State> start, shared_ptr<State> end);
		bool checkRelativeLocalization(shared_ptr<State> node);
		bool insideWorldBounds(shared_ptr<Point> point, int worldWidth, int worldHeight);
		bool insideWorldBounds(vector<shared_ptr<Uav>> points, int worldWidth, int worldHeight);
		bool isStateValid(shared_ptr<State> oldState, shared_ptr<State> newState, shared_ptr<Map> map);
		bool check_obstacle_vcollide_single(shared_ptr<State> new_node, shared_ptr<Map> map);

	protected:
		shared_ptr<Configuration> configuration;
	};

}