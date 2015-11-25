#pragma once
#include <memory>
#include "Goal.h"

using namespace App;

namespace App
{
		
	class GoalGroup
	{
	public:
		GoalGroup();
		virtual ~GoalGroup();
		void addGoal(shared_ptr<Goal> goal);
		shared_ptr<Point> getMiddle();
		bool contains(shared_ptr<Point> location);

	protected:
		vector<shared_ptr<Goal>> goals;
		shared_ptr<Rectangle> rectangle;	//rectangle containing all goals
		void initializeRectangle();	//proto�e jdou c�le p�id�vat dynamicky, vytvo��m rectangle lazy, kdy p�edpokl�d�m, �e se nic nebude vytv��et. Vy�e�ilo by se to builder paternem, ale naah.
	};
	
}
