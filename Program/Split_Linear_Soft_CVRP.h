#ifndef SPLIT_LINEAR_SOFT_CVRP_H
#define SPLIT_LINEAR_SOFT_CVRP_H

#include "Split.h"

// Original linear soft-capacity CVRP solver from Thibaut Vidal's
// Split-Library, commit 0005ac156a0d632cb214c5504ddd1e409bd340c9.
// Upstream license: GNU GPL v3.
// Integrated into this extended data model on September 20th, 2026.
class Split_Linear_Soft: public Split
{
private:
	vector<double> potential;
	vector<int> pred;
	vector<double> sumDistance;
	vector<double> sumLoad;

	inline double propagate(int i, int j)
	{
		return potential[i] + sumDistance[j] - sumDistance[i + 1]
			+ myData->cli[i + 1].dreturn + myData->cli[j].dreturn
			+ myData->penaltyLoad * max<double>(sumLoad[j] - sumLoad[i] - myData->vehCapacity, 0.);
	}

	// Tests if i dominates j as a predecessor for all nodes x >= j+1.
	inline bool dominates(int i, int j)
	{
		return potential[j] + myData->cli[j + 1].dreturn
			> potential[i] + myData->cli[i + 1].dreturn
			+ sumDistance[j + 1] - sumDistance[i + 1]
			+ myData->penaltyLoad * (sumLoad[j] - sumLoad[i]) - 0.0001;
	}

	// Tests if j dominates i as a predecessor for all nodes x >= j+1.
	inline bool dominatesRight(int i, int j)
	{
		return potential[j] + myData->cli[j + 1].dreturn
			< potential[i] + myData->cli[i + 1].dreturn
			+ sumDistance[j + 1] - sumDistance[i + 1] + 0.0001;
	}

public:
	Split_Linear_Soft(Pb_Data* myData) : Split(myData) {}

	int solve();
};

#endif
