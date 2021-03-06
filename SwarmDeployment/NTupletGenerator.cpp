#include "NTupletGenerator.h"

namespace App
{


	NTupletGenerator::NTupletGenerator()
	{
	}

	NTupletGenerator::~NTupletGenerator()
	{
	}

	vector<unordered_map<App::UavForRRT, CarLikeControl, UavHasher>> NTupletGenerator::generateNTuplet(vector<CarLikeControl> usedChars, vector<shared_ptr<UavForRRT>> tupletKeys)
	{
		return generateNTuplet(usedChars, tupletKeys, tupletKeys.size() - 1);	//index posledn�ho prvku
	}

	vector<unordered_map<UavForRRT, CarLikeControl, UavHasher>> NTupletGenerator::generateNTuplet(vector<CarLikeControl> usedChars, vector<shared_ptr<UavForRRT>> tupletKeys, int index)
	{

		auto list = vector<unordered_map<UavForRRT, CarLikeControl, UavHasher>>();	//todo: pop�em��let, jak to refactorovat, aby se mohlo pracovat s fixn� velikost�  pole
																									//		vector<vector<T>> list = vector<vector<T>>(pow(usedChars.size(), tupletClass));
		if (index < 0)
		{
			list.push_back(unordered_map<UavForRRT, CarLikeControl, UavHasher>());
		}
		else
		{
			vector<unordered_map<UavForRRT, CarLikeControl, UavHasher>> tuplet = generateNTuplet(usedChars, tupletKeys, index - 1);
			for (auto character : usedChars)
			{
				for (auto row : tuplet)
				{
					row[*tupletKeys[index].get()] = character;
					list.push_back(row);
				}
			}
		}

		return list;
	}

}
