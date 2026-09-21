#ifndef SPLIT_BELLMAN_SOFT_VRPSPDTW_H
#define SPLIT_BELLMAN_SOFT_VRPSPDTW_H

#include "Split.h"

// Simple Bellman algorithm
// For the VRPSPDTW with soft capacity and time-window constraints
class Split_Bellman_Soft_VRPSPDTW: public Split
{

private:

	// Potential vector
	vector < double > potential ;

	// Index of the predecessor in an optimal path
	vector < int > pred ;

public:

	Split_Bellman_Soft_VRPSPDTW(Pb_Data * myData) : Split(myData) {}

	int solve();
};

#endif
