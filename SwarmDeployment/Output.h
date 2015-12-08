#pragma once
#include "vector"
#include <memory>
#include "Node.h"
#include "State.h"

using namespace std;

namespace App
{
	class Output
	{
	public:
		Output();
		virtual ~Output();
		vector<shared_ptr<LinkedState>> nodes;
		int start_goal_distance_euclidean;	//distance between start and goal. 
		int start_goal_distance_a_star;	//todo: tyto dva integery p�ed�lat na pole doubl�, pole dlouh� jako po�et c�l�
		bool goals_reached;	//empty pointer, pokud uav nedorazilo. vektor dlouh� jako po�et uav
		//todo: p�ed�lat na mapy
		vector<double> distancesToGoal;		//ke ka�d� UAV d�lka cesty k c�li
		vector<shared_ptr<LinkedState>> finalNodes;
	};

}

