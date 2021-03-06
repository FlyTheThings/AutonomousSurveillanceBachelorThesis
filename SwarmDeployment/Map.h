﻿#pragma once
#include "Goal.h"
#include "Obstacle.h"
#include <vector>
#include <memory>
#include "GoalGroup.h"

using namespace std;
using namespace json_spirit;

namespace App
{
	class UavForRRT;

	class Map
	{
	public:
		Map();
		~Map();
		void addObstacle(shared_ptr<Obstacle> obstacle);
		void addGoal(shared_ptr<Goal> goal);
		size_t countGoals() const;
		vector<shared_ptr<Goal>> getGoals();
		vector<shared_ptr<Obstacle>> getObstacles();
		void addUavStart(shared_ptr<UavForRRT> start);
		vector<shared_ptr<UavForRRT>> getUavsStart();
		int countUavs() const;
		virtual shared_ptr<GoalGroup> getGoalGroup() const;
		mObject toJson() const;
		static shared_ptr<Map> fromJson(mValue json);
		void amplifyObstacles(int sizeIncrement);

	protected:
		vector<shared_ptr<Goal>> goals;
		vector<shared_ptr<Obstacle>> obstacles;
		vector<shared_ptr<UavForRRT>> uavsStart;
		shared_ptr<GoalGroup> goalGroup;
		vector<shared_ptr<Obstacle>> originalObstacles;
	};

}
