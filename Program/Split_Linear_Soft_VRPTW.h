#ifndef SPLIT_LINEAR_SOFT_VRPTW_H
#define SPLIT_LINEAR_SOFT_VRPTW_H

#include "Split.h"
#include "Trivial_Deque.h"
#include "Convoluted_Deque_VRPTW.h"

class Split_Linear_Soft_VRPTW: public Split 
{

private:

// Potential vector
vector < double > potential ;

// Indice of the predecessor in an optimal path
vector < int > pred ;

vector <Node> nodes;

// sumDistance[i] for i > 1 contains the sum of distances : sum_{k=1}^{i-1} d_{k,k+1}
vector <double> sumDistance ;

// sumLoad[i] for i >= 1 contains the sum of loads : sum_{k=1}^{i} q_k
vector <double> sumLoad ;

// sumDuration[i] for i >= 1 contains the sum of durations : sum_{k=1}^{i} d_k
vector <double> sumDuration ;

//r[i] contains the amount of warp i experienced at its first infeasible closing window
vector <double> r;

//q[i] corresponds with r[i] and gives the index of the first instance of time warp for pred i
vector <int> q;

//W[i] contains the sum of warps experienced from 0 to i by predecessor 0
vector <double> W;

// To be called with i < j only
// Computes the cost of propagating the label i until j
inline double c(int i, int j) 
{
	return sumDistance[j] - sumDistance[i+1] + myData->cli[i+1].dreturn + myData->cli[j].dreturn ;
}

// when only delivery demand is considered
inline double pen_alpha(int i, int j) { 
	double excess = sumLoad[j] - sumLoad[i] - myData->vehCapacity;
	return excess <= tolerance ? 0.0 : myData->penaltyLoad * excess;
}

inline double pen_beta(int i, int j) {
	return r[i] <= tolerance ? 0 : myData->penaltyWarp * (r[i] + W[j] - W[q[i]]);
}

inline double dominated_alpha_beta(int i, int j, int k) {
	return potential[i] + myData->cli[i + 1].dreturn + sumDistance[j + 1] - sumDistance[i + 1] + pen_alpha(i, k) + pen_beta(i, k) >= potential[j] + myData->cli[j + 1].dreturn + pen_alpha(j, k) + pen_beta(j, k) - tolerance;
}

inline double c_alpha_beta(int i, int j) {
	return c(i, j) + pen_alpha(i, j) + pen_beta(i, j);
}

inline double initial_time_warp(int i, int j, int k) {
	return std::max(myData->cli[i + 1].dreturn + (sumDuration[k] - sumDuration[i + 1]), myData->cli[j].tw_open + (sumDuration[k] - sumDuration[j])) - 
		std::min(myData->cli[k].tw_close, myData->time_limit - myData->cli[k].dreturn - myData->cli[k].service_time);
}

// Tests if i is dominated by j as a predecessor for all nodes x >= j+1
// We assume that i < j
inline bool dominated(int i, int j) { 
	return potential[i] + myData->cli[i + 1].dreturn + (sumDistance[j + 1] - sumDistance[i + 1]) >= potential[j] + myData->cli[j + 1].dreturn - tolerance;
}

inline bool dominated_a(int i, int j) { // switched from original
	return myData->cli[i].tw_open + (sumDuration[j] - sumDuration[i]) <= myData->cli[j].tw_open + tolerance;
}

public:

	Split_Linear_Soft_VRPTW(Pb_Data * myData) : Split(myData) {}

	int solve();
};

#endif
