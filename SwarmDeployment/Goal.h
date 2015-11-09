#pragma once
#include "Rectangle.h"

namespace App
{

	class Goal
	{

	public:
		Goal(int x, int y, int width, int height);
		virtual ~Goal();
		std::shared_ptr<Rectangle> rectangle;

	};

}