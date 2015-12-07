#pragma once
#include "PointParticle.h"
#include <vector>
#include <string>
#include "Uav.h"
#include "CarLikeControl.h"

using namespace std;

namespace App
{

	class State
	{
	public:
		State(int inputCount);
		State(const State& other);
		virtual ~State();
		vector<bool> used_inputs; // na za��tku pole false, o d�lce number_of_inputs
		unordered_map<Uav, shared_ptr<CarLikeControl>, UavHasher> prev_inputs;	//vstupy, kter� vedly do t�to node
		bool areAllInputsUsed();
		friend std::ostream& operator<<(std::ostream& os, const State& obj);
		shared_ptr<Uav> getUav(shared_ptr<Uav> uav);	//used to acquire uav with same id as uav in argument, even if uav locations differ. It uses == to compare
		bool areUavsInGoals();
		virtual double getDistanceOfNewNodes() const;
		virtual void setDistanceOfNewNodes(const double distance_of_new_nodes);
		virtual int getIndex() const;
		virtual shared_ptr<State> getPrevious() const;
		virtual void setPrevious(const shared_ptr<State> state);
		friend bool operator==(const State& lhs, const State& rhs);
		friend bool operator!=(const State& lhs, const State& rhs);
		virtual vector<shared_ptr<Uav>> getUavs() const;
		virtual void setUavs(const vector<shared_ptr<Uav>> shared_ptrs);
		virtual void incrementTime(double increment);
		virtual double getTime() const;
		virtual void swapUavs(shared_ptr<Uav> first, shared_ptr<Uav> second);

	protected:
		vector<shared_ptr<Uav>> uavs; //spojen� prom�nn�ch loc a rot z Node objektu z matlabu. nejsp� node bude jin� pro rrt path a pro diskretizaci na nalezen guiding path
		double distanceOfNewNodes;	//proto�e se m�n�, ukl�d�m ji sem
		shared_ptr<State> previous;
		int index;
		static int lastIndex;
		double time;	//�as, ve kter�m jsou UAV v dan�ch pozic�ch. Pokud chci kontrolovat kolize, pot�ebuji v�d�t, v jak�m �ase se UAV na dan�m m�st� nach�z�.
	};

}
