﻿#pragma once
#include "Point.h"
#include <memory>
#include "VCollide/Rectangle2D.h"

using namespace std;

namespace App
{

	class Rectangle
	{
	public:
		Rectangle(int x, int y, int width, int height);

		Rectangle(const Rectangle& other);
		int getX() const;
		int getY() const;
		int getHeight() const;
		int getWidth() const;
		void setX(int x);
		void setY(int y);
		void setHeight(int height);
		void setWidth(int width);
		bool contains(shared_ptr<Point> point);
		double getVolume() const;
		friend bool operator==(const Rectangle& lhs, const Rectangle& rhs);
		friend bool operator!=(const Rectangle& lhs, const Rectangle& rhs);
		friend size_t hash_value(const Rectangle& obj);
		shared_ptr<Point> getMiddle();
		mObject toJson() const;
		static shared_ptr<Rectangle> fromJson(mObject data);
		Rectangle2D toColDetectRectandle();
		double getDistance(shared_ptr<Point> point) const;

	protected:
		shared_ptr<Point> location;
		int m_height;
		int m_width;

	};

}