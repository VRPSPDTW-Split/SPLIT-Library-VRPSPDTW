#ifndef SPLIT_LINEAR_CVRP_H
#define SPLIT_LINEAR_CVRP_H

#include "Split.h"
#include "Trivial_Deque.h"

class Split_Linear_CVRP: public Split 
{

private:

// Potential vector
vector < double > potential ;

// Indice of the predecessor in an optimal path
vector < int > pred ;

// sumDistance[i] for i > 1 contains the sum of distances : sum_{k=1}^{i-1} d_{k,k+1}
vector <double> sumDistance ;

// sumLoad[i] for i >= 1 contains the sum of loads : sum_{k=1}^{i} q_k
vector <double> sumLoad ;

// To be called with i < j only
// Computes the cost of propagating the label i until j
inline double c(int i, int j) 
{
	return sumDistance[j] - sumDistance[i+1] + myData->cli[i+1].dreturn + myData->cli[j].dreturn ;
}

// Tests if i is dominated by j as a predecessor for all nodes x >= j+1
// We assume that i < j
inline bool dominated(int i, int j) { 
	return potential[i] + myData->cli[i + 1].dreturn + (sumDistance[j + 1] - sumDistance[i + 1]) >= potential[j] + myData->cli[j + 1].dreturn;
}

inline bool infeasible_d(int i, int j) {
    return sumLoad[j] - sumLoad[i] > myData->vehCapacity; 
}

public:

	Split_Linear_CVRP(Pb_Data * myData) : Split(myData) {}

	int solve();
};

#endif
