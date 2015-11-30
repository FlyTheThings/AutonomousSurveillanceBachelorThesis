#pragma once
#include "Rectangle.h"

using namespace std;

namespace App
{

	class Goal
	{

	public:
		Goal(int x, int y, int width, int height);
		virtual ~Goal();
		virtual bool contains(shared_ptr<Point> location);
		friend bool operator==(const Goal& lhs, const Goal& rhs);
		friend bool operator!=(const Goal& lhs, const Goal& rhs);
		virtual shared_ptr<Point> getRandomPointInside();
		friend size_t hash_value(const Goal& obj);
		virtual shared_ptr<Rectangle> getRectangle();	//nen� const, proto�e jeho potomek se zavol�n�m getteru lazy inicializuje. po vytvo�en� builderu bude op�t moct b�t lazy

	protected:
		shared_ptr<Rectangle> rectangle;
	};

	class GoalHasher
	{
	public:
		size_t operator() (Goal const& key) const
		{
			return hash_value(key);
		}
	};

}