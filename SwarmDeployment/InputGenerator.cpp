#include "InputGenerator.h"
#include <string>

namespace App {

	InputGenerator::InputGenerator(int input_samples_dist, int input_samples_phi) : 
		input_samples_dist(input_samples_dist), input_samples_phi(input_samples_phi)
	{
	}

	InputGenerator::~InputGenerator()
	{
	}

	vector<unordered_map<UavForRRT, CarLikeControl, UavHasher>> InputGenerator::generateAllInputs(int distance_of_new_nodes, double max_turn, vector<shared_ptr<UavForRRT>> uavs, bool zeroInputEnabled)
	{
		if (uavs.size() == 0)
		{
			throw "Uavs size is 0. No inputs are generated";
		}

		string stringRepresentation = argumentsToString(distance_of_new_nodes, max_turn, uavs, zeroInputEnabled);
		if (isInCache(stringRepresentation))
		{
			return cache[stringRepresentation];
		}

		auto oneUavInputs = generateOneUavInputs(distance_of_new_nodes, max_turn, zeroInputEnabled);
		auto inputs = generator.generateNTuplet(oneUavInputs, uavs);		//po�et v�ech kombinac� je po�et v�ech mo�n�ch vstup� jednoho UAV ^ po�et UAV

		if (zeroInputEnabled)	//odebr�n� nulov�ch vstup� pro v�echna UAV najednou, aby se v�dy alespo� jedno pohnulo, je to v�dy prvn� prvek
		{
			inputs.erase(inputs.begin());
		}

		cache[stringRepresentation] = inputs;
		return inputs;
	}

	vector<CarLikeControl> InputGenerator::generateOneUavInputs(int distance_of_new_nodes, double max_turn, bool zeroInputEnabled)
	{
		vector<CarLikeControl> oneUavInputs = vector<CarLikeControl>();

		if (zeroInputEnabled)
		{
			CarLikeControl zeroInput = CarLikeControl(0, 0);
			oneUavInputs.push_back(zeroInput);
		}

		for (size_t k = 0; k < input_samples_dist; k++)
		{
			for (size_t m = 0; m < input_samples_phi; m++)
			{
				double step = distance_of_new_nodes / pow(1.5, k);
				double turn = -max_turn + 2 * m * max_turn / (input_samples_phi - 1);
				CarLikeControl point = CarLikeControl(step, turn);
				oneUavInputs.push_back(point);
			}
		}

		return oneUavInputs;
	}

	bool InputGenerator::isInCache(string stringRepresentation)
	{
		return cache.find(stringRepresentation) != cache.end();
	}

	string InputGenerator::argumentsToString(int distance_of_new_nodes, double max_turn, vector<shared_ptr<UavForRRT>> uavs, bool zeroInputEnabled)
	{
		string string = "d:" + to_string(distance_of_new_nodes) + "m:" + to_string(max_turn) + "z:" + to_string(zeroInputEnabled);
		for (auto uav : uavs)
		{
			string += "id:" + to_string(uav->getId());
		}
		return string;
	}

}
