#ifndef SPLIT_LINEAR_SOFT_VRPSPD_H
#define SPLIT_LINEAR_SOFT_VRPSPD_H

#include "Split.h"
#include "Trivial_Deque.h"
#include "Convoluted_Deque_VRPSPD.h"
#include "Node.h"

class Split_Linear_Soft_VRPSPD: public Split 
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

// sumPickup[i] for i >= 1 contains the sum of pickups : sum_{k=1}^{i} p_k
vector <double> sumPickup ;

// Highest-load point for Lambda.best and all predecessors to its left.
vector <int> h;

vector <Node> nodes;

// To be called with i < j only
// Computes the cost of propagating the label i until j
inline double c(int i, int j) 
{
	return sumDistance[j] - sumDistance[i+1] + myData->cli[i+1].dreturn + myData->cli[j].dreturn ;
}

inline double pen_alpha(int i, int j, int k) {
	double excess = sumLoad[k] - sumLoad[j] + sumPickup[j] - sumPickup[i] - myData->vehCapacity;
	return excess <= tolerance ? 0.0 : myData->penaltyLoad * excess;
}

inline double c_alpha(int i, int j, int k) {
	return c(i, k) + pen_alpha(i, j, k);
}
// Tests if i is dominated by j as a predecessor for all nodes x >= j+1.
// We assume that i < j.
inline bool dominated(int i, int j) {
	return potential[i] + myData->cli[i + 1].dreturn + (sumDistance[j + 1] - sumDistance[i + 1]) >= potential[j] + myData->cli[j + 1].dreturn - tolerance;
}

inline bool dominated_h(int i, int j) {
	return sumLoad[j] - sumLoad[i] + sumPickup[i] <= sumPickup[j] + tolerance;
}

inline bool dominated_alpha(int i, int j, int k, int l, int x) {
	return potential[i] + myData->cli[i + 1].dreturn + sumDistance[k + 1] - sumDistance[i + 1] + pen_alpha(i, j, x) >= potential[k] + myData->cli[k + 1].dreturn + pen_alpha(k, l, x) - tolerance;
}

inline bool dominated_alpha_p(int i, int j) {
	return potential[i] + myData->cli[i + 1].dreturn + (sumDistance[j + 1] - sumDistance[i + 1]) + myData->penaltyLoad * (sumPickup[j] - sumPickup[i]) >= potential[j] + myData->cli[j + 1].dreturn - tolerance;
}

public:

	Split_Linear_Soft_VRPSPD(Pb_Data * myData) : Split(myData) {}

	int solve();
};

#endif
