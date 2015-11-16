#include "State.h"


namespace App
{

	State::State()
	{
		//todo: refactorovat a vyr�b�t nov� stavy z factory, kter� jim bude p�ed�vat d�lku used_inputs pole podle konfigurace v konstruktoru.
		used_inputs = vector<bool>(81);		//zat�m je zde velikost natvrdo
		fill(used_inputs.begin(), used_inputs.end(), false);
	}

	State::State(const State& other) : index(index), used_inputs(used_inputs)
	{
		for (auto uav : other.uavs)
		{
			uavs.push_back(make_shared<PointParticle>(*uav.get()));
		}
		if (prev)	//kontrola, zda je shred_pointer pr�zdn�
		{
			prev = make_shared<State>(*other.prev.get());
		}
		for (auto prev_input : other.prev_inputs)
		{
			prev_inputs.push_back(make_shared<Point>(*prev_input.get()));
		}
	}

	State::~State()
	{
	}

	bool State::areAllInputsUsed()
	{
		bool areAllInputsUsed = true;
		for (auto inputUsed : used_inputs)
		{
			areAllInputsUsed = areAllInputsUsed && inputUsed;
		}
		return areAllInputsUsed;
	}

	shared_ptr<State> State::clone() const
	{
		auto newObject = make_shared<State>();
		for (auto prev_input : prev_inputs)
		{
			newObject->prev_inputs = prev_inputs;
		}
		newObject->uavs = uavs;
		newObject->index = index;
		newObject->prev = prev;
		newObject->used_inputs = used_inputs;
		return newObject;
	}
}