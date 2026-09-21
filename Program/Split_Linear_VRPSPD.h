#ifndef SPLIT_LINEAR_VRPSPD_H
#define SPLIT_LINEAR_VRPSPD_H

#include "Split.h"
#include "Trivial_Deque.h"

class Split_Linear_VRPSPD : public Split
{
private:
	vector<double> potential;
	vector<int> pred;
	vector<double> sumDistance;
	vector<double> sumLoad;
	vector<double> sumPickup;

	inline double c(int i, int j)
	{
		return sumDistance[j] - sumDistance[i + 1] + myData->cli[i + 1].dreturn + myData->cli[j].dreturn;
	}

	inline bool dominated(int i, int j)
	{
		return potential[i] + myData->cli[i + 1].dreturn + (sumDistance[j + 1] - sumDistance[i + 1]) >= potential[j] + myData->cli[j + 1].dreturn - tolerance;
	}

	inline bool dominated_h(int i, int j)
	{
		return sumLoad[j] - sumLoad[i] + sumPickup[i] <= sumPickup[j] + tolerance;
	}

	inline bool infeasible_h(int i, int j, int k)
	{
		return sumPickup[j] - sumPickup[i] + sumLoad[k] - sumLoad[j] > myData->vehCapacity + tolerance;
	}

public:
	Split_Linear_VRPSPD(Pb_Data *myData) : Split(myData) {}

	int solve();
};

#endif
