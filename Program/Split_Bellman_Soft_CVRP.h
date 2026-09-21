#ifndef SPLIT_BELLMAN_SOFT_CVRP_H
#define SPLIT_BELLMAN_SOFT_CVRP_H

#include "Split.h"

// Original soft-capacity CVRP Bellman solver from Thibaut Vidal's
// Split-Library, commit 0005ac156a0d632cb214c5504ddd1e409bd340c9.
// Upstream license: GNU GPL v3.
// Integrated into this extended data model on September 20th, 2026.
class Split_Bellman_Soft: public Split
{
private:
	vector<double> potential;
	vector<int> pred;

public:
	Split_Bellman_Soft(Pb_Data* myData) : Split(myData) {}

	int solve();
};

#endif
