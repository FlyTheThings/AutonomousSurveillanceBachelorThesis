#pragma once
#include "PointParticle.h"
#include <vector>
#include <string>
#include "UavForRRT.h"
#include "CarLikeControl.h"
#include "StateInterface.h"

using namespace std;

namespace App
{

	class LinkedState : public StateInterface
	{
	public:
		LinkedState(int inputCount);
		LinkedState(const LinkedState& other);
		virtual ~LinkedState();
		vector<bool> used_inputs; // na za��tku pole false, o d�lce number_of_inputs
		bool areAllInputsUsed();
		friend std::ostream& operator<<(std::ostream& os, const LinkedState& obj);
		virtual shared_ptr<UavInterface> getBaseUav(shared_ptr<UavInterface> uav) const override;	//used to acquire uav with same id as uav in argument, even if uav locations differ. It uses == to compare
		bool areUavsInGoals() const;
		virtual double getDistanceOfNewNodes() const;
		virtual void setDistanceOfNewNodes(const double distance_of_new_nodes);
		virtual int getIndex() const;
		virtual shared_ptr<LinkedState> getPrevious() const;
		virtual void setPrevious(const shared_ptr<LinkedState> state);
		friend bool operator==(const LinkedState& lhs, const LinkedState& rhs);
		friend bool operator!=(const LinkedState& lhs, const LinkedState& rhs);
		virtual vector<shared_ptr<UavInterface>> getBaseUavs() const override;
		virtual void setUavs(const vector<shared_ptr<UavForRRT>> shared_ptrs);
		virtual void incrementTime(double increment);
		virtual double getTime() const;
		virtual vector<shared_ptr<UavForRRT>> getUavsForRRT();
	protected:
		vector<shared_ptr<UavForRRT>> uavs; //spojen� prom�nn�ch loc a rot z Node objektu z matlabu. nejsp� node bude jin� pro rrt path a pro diskretizaci na nalezen guiding path
		double distanceOfNewNodes;	//proto�e se m�n�, ukl�d�m ji sem
		shared_ptr<LinkedState> previous;
		int index;
		static int lastIndex;
		double time;	//�as, ve kter�m jsou UAV v dan�ch pozic�ch. Pokud chci kontrolovat kolize, pot�ebuji v�d�t, v jak�m �ase se UAV na dan�m m�st� nach�z�.
	};

}
