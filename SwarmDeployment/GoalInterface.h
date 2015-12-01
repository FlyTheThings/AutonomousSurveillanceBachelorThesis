#pragma once
#include <memory>
#include "Rectangle.h"

using namespace std;

namespace App
{

	class GoalInterface
	{
	public:
		virtual ~GoalInterface();
		virtual bool contains(shared_ptr<Point> location) = 0;
		virtual shared_ptr<Rectangle> getRectangle() = 0;
		virtual shared_ptr<Point> getRandomPointInside() = 0;
		//todo: ud�lat n�jak� getGoal, aby se v GoalGroup zjistilo, v jak�m c�li UAV je. nebo tak n�co. a mo�n� to ani nebude t�eba.
	};


}
