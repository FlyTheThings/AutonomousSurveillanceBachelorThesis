#include "GoalGroup.h"

namespace App
{
		
	GoalGroup::GoalGroup()
	{
	}
	
	
	GoalGroup::~GoalGroup()
	{
	}

	void GoalGroup::addGoal(shared_ptr<Goal> goal)
	{
		goals.push_back(goal);
	}

	shared_ptr<Point> GoalGroup::getMiddle()
	{
		if (!rectangle)
		{
			initializeRectangle();
		}
		return rectangle->getMiddle();
	}

	bool GoalGroup::contains(shared_ptr<Point> location)
	{
		if (!rectangle)
		{
			initializeRectangle();
		}
		return rectangle->contains(location);
	}

	void GoalGroup::initializeRectangle()
	{
		//nejd��ve najdu 2 kraj� body ze v�ech obd�ln�k� (c�l�) (lev� doln� a prav� horn�), abych z n�j pak vytvo�il velk�, v�epokr�vaj�c� obd�ln�k.
		double leftLowerX = DBL_MAX;
		double leftLowerY = DBL_MAX;
		double rightUpperX = DBL_MIN;
		double rightUpperY = DBL_MIN;

		for (auto goal : goals)
		{
			if (goal->rectangle->getX() < leftLowerX)
			{
				leftLowerX = goal->rectangle->getX();
			}
			if (goal->rectangle->getY() < leftLowerY)
			{
				leftLowerY = goal->rectangle->getY();
			}
			if (goal->rectangle->getX() + goal->rectangle->getWidth() > rightUpperX)
			{
				rightUpperX = goal->rectangle->getX() + goal->rectangle->getWidth();
			}
			if (goal->rectangle->getY() + goal->rectangle->getHeight() > rightUpperY)
			{
				rightUpperY = goal->rectangle->getY() + goal->rectangle->getHeight();
			}
		}

		rectangle = make_shared<Rectangle>(leftLowerX, leftLowerY, rightUpperX - leftLowerX, rightUpperY - leftLowerY);
	}
}
