#include "GoalGroup.h"
#include "Random.h"

namespace App
{
		
	GoalGroup::GoalGroup() : Goal(0, 0, 0, 0)	//inicializuji a� lazy. Todo: mo�n� p�ed�lat, aby se goalGroup vytvo�il builderPaternem, pak budu moct eager loadovat
	{
		rectangle.reset();
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

	shared_ptr<Rectangle> GoalGroup::getRectangle()
	{
		if (!rectangle)
		{
			initializeRectangle();
		}
		return rectangle;
	}

	shared_ptr<Point> GoalGroup::getRandomPointInside()
	{
		auto randomGoal = Random::element(goals);
		return randomGoal->getRandomPointInside();
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
			auto rectangle = goal->getRectangle();
			if (rectangle->getX() < leftLowerX)
			{
				leftLowerX = rectangle->getX();
			}
			if (rectangle->getY() < leftLowerY)
			{
				leftLowerY = rectangle->getY();
			}
			if (rectangle->getX() + rectangle->getWidth() > rightUpperX)
			{
				rightUpperX = rectangle->getX() + rectangle->getWidth();
			}
			if (rectangle->getY() + rectangle->getHeight() > rightUpperY)
			{
				rightUpperY = rectangle->getY() + rectangle->getHeight();
			}
		}

		rectangle = make_shared<Rectangle>(leftLowerX, leftLowerY, rightUpperX - leftLowerX, rightUpperY - leftLowerY);
	}
}
