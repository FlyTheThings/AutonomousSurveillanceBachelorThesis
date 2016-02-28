#include "CarLikeControl.h"


namespace App
{
	CarLikeControl::CarLikeControl()
	{
		step = 0;
		turn = 0;
	}

	CarLikeControl::CarLikeControl(double step, double turn) : 
		step(step), turn(turn)
	{
	}

	CarLikeControl::~CarLikeControl()
	{
	}

	double CarLikeControl::getStep() const
	{
		return step;
	}

	double CarLikeControl::getTurn() const
	{
		return turn;
	}

	CarLikeControl::CarLikeControl(const CarLikeControl& other): 
		step(other.step), turn(other.turn)
	{
	}

	void CarLikeControl::setStep(const double step)
	{
		this->step = step;
	}

	mObject CarLikeControl::toJson() const
	{
		mObject object;
		object["step"] = this->step;
		object["turn"] = this->turn;
		return object;
	}

	CarLikeControl CarLikeControl::fromJson(mValue data)
	{
		auto step = data.get_obj().at("step").get_real();
		auto turn = data.get_obj().at("turn").get_real();
		return CarLikeControl(step, turn);
	}

	std::ostream& operator<<(std::ostream& os, const CarLikeControl& obj)
	{
		return os
			<< "step: " << obj.step
			<< " turn: " << obj.turn;
	}
}