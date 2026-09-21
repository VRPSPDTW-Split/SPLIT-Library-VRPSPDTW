#ifndef SPLIT_BELLMAN_VRPSPDTW_H
#define SPLIT_BELLMAN_VRPSPDTW_H

#include "Split.h"

// Simple Bellman algorithm
// For the VRPSPDTW with hard capacity constraints
class Split_Bellman_VRPSPDTW: public Split 
{

private:

	// Potential vector
	vector < double > potential ; 

	// Indice of the predecessor in an optimal path
	vector < int > pred ; 

public:

	Split_Bellman_VRPSPDTW(Pb_Data * myData) : Split(myData) {}

	int solve();
};

#endif
