﻿#pragma once
#include "Point.h"
#include <memory>

namespace App
{

	class Rectangle
	{
	public:
		Rectangle(int x, int y, int width, int height);

		int getX() const;
		int getY() const;
		int getHeight() const;
		int getWidth() const;
		void setX(int x);
		void setY(int y);
		void setHeight(int height);
		void setWidth(int width);
		bool contains(std::shared_ptr<Point> point);
		double getVolume() const;
		friend bool operator==(const Rectangle& lhs, const Rectangle& rhs);
		friend bool operator!=(const Rectangle& lhs, const Rectangle& rhs);
		friend std::size_t hash_value(const Rectangle& obj);

	protected:
		std::shared_ptr<Point> location;
		int m_height;
		int m_width;

	};

}