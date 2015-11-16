#include "State.h"


namespace App
{

	State::State()
	{
		//todo: refactorovat a vyr�b�t nov� stavy z factory, kter� jim bude p�ed�vat d�lku used_inputs pole podle konfigurace v konstruktoru.
		used_inputs = vector<bool>(81);		//zat�m je zde velikost natvrdo
		fill(used_inputs.begin(), used_inputs.end(), false);
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