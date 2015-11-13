#pragma once
#include "PointParticle.h"
#include <vector>

using namespace std;

namespace App
{

	class State
	{
	public:
		State();
		virtual ~State();

		int index;	//todo: zjistit, k �emu to m� b�t, pou��v� se to v rrt_path. je to jako id, pou��v� se pro z�sk�n� v�ech inicializovan�ch nodes, co� j� nepot�ebuju

		vector<shared_ptr<App::PointParticle>> uavs; //spojen� prom�nn�ch loc a rot z Node objektu z matlabu. nejsp� node bude jin� pro rrt path a pro diskretizaci na nalezen guiding path
		vector<bool> used_inputs; // na za��tku pole false, o d�lce number_of_inputs
		bool areAllInputsUsed();
		shared_ptr<State> prev;
		vector<double> prev_inputs;	//zat�m v�bec nev�m, k �emu se tohle pou��v� prost� to jen pot�ebuju p�edat
	};

}

