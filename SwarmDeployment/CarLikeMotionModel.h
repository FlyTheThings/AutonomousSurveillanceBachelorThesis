#pragma once
#include <memory>
#include "PointParticle.h"
#include "CarLikeControl.h"
#include "MotionModel.h"

using namespace std;

namespace App
{
	class Configuration;

	class CarLikeMotionModel : public MotionModel
	{
	public:
		CarLikeMotionModel(shared_ptr<Configuration> configuration);
		virtual ~CarLikeMotionModel();
		void calculateState(shared_ptr<PointParticle> state, shared_ptr<CarLikeControl> control) override;
		double getCurveRadius(shared_ptr<CarLikeControl> control) override;
		double getMinimalCurveRadius() override;	//calculates radius from max value from configuration
	protected:
		shared_ptr<Configuration> configuration;
		double getTotalRotation(shared_ptr<CarLikeControl> control);
	};

}