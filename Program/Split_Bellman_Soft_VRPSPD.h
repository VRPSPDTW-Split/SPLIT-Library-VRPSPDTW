#ifndef SPLIT_BELLMAN_SOFT_VRPSPD_H
#define SPLIT_BELLMAN_SOFT_VRPSPD_H

#include "Split.h"

// Simple Bellman algorithm
// For the CVRP with soft capacity constraints
class Split_Bellman_Soft_VRPSPD: public Split 
{

private:

	// Potential vector
	vector < double > potential ; 

	// Index of the predecessor in an optimal path
	vector < int > pred ; 

public:

	Split_Bellman_Soft_VRPSPD(Pb_Data * myData) : Split(myData) {}

	int solve();
};

#endif

