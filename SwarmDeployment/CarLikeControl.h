#pragma once
#include <ostream>
#include <json_spirit_v4.08/json_spirit/json_spirit_reader.h>

using namespace json_spirit;

namespace App
{

	class CarLikeControl
	{
	public:
		CarLikeControl(double step, double turn);
		virtual ~CarLikeControl();
		virtual double getStep() const;
		virtual double getTurn() const;
		friend std::ostream& operator<<(std::ostream& os, const CarLikeControl& obj);
		CarLikeControl(const CarLikeControl& other);
		virtual void setStep(const double step);
		mObject toJson() const;

	protected:
		double step;	//krok vp�ed
		double turn;	//oto�ka
	};

}

