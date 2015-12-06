#include "State.h"


namespace App
{
	int State::lastIndex = 0;

	State::State(int inputCount) : index(lastIndex++)
	{
		//todo: refactorovat a vyr�b�t nov� stavy z factory, kter� jim bude p�ed�vat d�lku used_inputs pole podle konfigurace v konstruktoru.
		used_inputs = vector<bool>(inputCount);		//zat�m je zde velikost natvrdo
		fill(used_inputs.begin(), used_inputs.end(), false);
	}

	State::State(const State& other) : index(index + 1), used_inputs(used_inputs)
	{
		for (auto uav : other.uavs)
		{
			uavs.push_back(make_shared<Uav>(*uav.get()));
		}
		if (previous)	//kontrola, zda je shred_pointer pr�zdn�
		{
			previous = make_shared<State>(*other.previous.get());
		}
		for (auto prev_input : other.prev_inputs)
		{
			prev_inputs[prev_input.first] = make_shared<Point>(*prev_input.second.get());
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

	shared_ptr<Uav> State::getUav(shared_ptr<Uav> uav)
	{
		for(auto other : uavs)
		{
			if (*other.get() == *uav.get())
			{
				return other;
			}
		}
		throw "No equal node found for node " + to_string(uav->getId());
	}

	bool State::areUavsInGoals()
	{
		bool allInGoals = true;
		for (auto uav : uavs)
		{
			allInGoals = allInGoals && uav->isGoalReached();
		}
		return allInGoals;
	}

	double State::getDistanceOfNewNodes() const
	{
		return distanceOfNewNodes;
	}

	void State::setDistanceOfNewNodes(const double distance_of_new_nodes)
	{
		distanceOfNewNodes = distance_of_new_nodes;
	}

	int State::getIndex() const
	{
		return index;
	}

	shared_ptr<State> State::getPrevious() const
	{
		return previous;
	}

	void State::setPrevious(const shared_ptr<State> state)
	{
		previous = state;
	}

	std::ostream& operator<<(std::ostream& os, const State& obj)
	{
		os << "index: " << obj.index << endl;
		os << " uavs: ";
		for (auto a : obj.uavs)
		{
			os << *a.get() << endl;
		}
		os << " used_inputs: ";
		for (auto a : obj.used_inputs)
		{
			os << a;
		}
		if (obj.previous)
		{
			os << " prev: " << *obj.previous.get() << endl;
		} else
		{
			os << " prev: empty" << endl;
		}
		os << " prev_inputs: ";
		for (auto a : obj.prev_inputs)
		{
			os << "id " << a.first << ": " << *a.second.get() << endl;
		}
		return os;
	}

}
