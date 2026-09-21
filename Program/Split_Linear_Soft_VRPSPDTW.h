#ifndef SPLIT_LINEAR_SOFT_VRPSPDTW_H
#define SPLIT_LINEAR_SOFT_VRPSPDTW_H

#include "Outer_Deque_VRPSPDTW.h"
#include "Split.h"
#include "Trivial_Deque.h"

#include <cmath>

class Split_Linear_Soft_VRPSPDTW: public Split
{
private:
    vector<double> potential;
    vector<int> pred;
    vector<double> sumDistance;
    vector<double> sumLoad;
    vector<double> sumPickup;
    vector<double> sumDuration;
    vector<double> r;
    vector<int> q;
    vector<double> W;
    vector<int> h;

    inline double c(int i, int j)
    {
        return sumDistance[j] - sumDistance[i + 1]
            + myData->cli[i + 1].dreturn + myData->cli[j].dreturn;
    }

    inline double pen_alpha_h(int i, int j, int k)
    {
        double excess = sumLoad[k] - sumLoad[j] + sumPickup[j]
            - sumPickup[i] - myData->vehCapacity;
        return excess <= tolerance ? 0.0
            : myData->penaltyLoad * excess;
    }

    inline double pen_beta_a(int i, int j)
    {
        return r[i] <= tolerance ? 0.0
            : myData->penaltyWarp * (r[i] + W[j] - W[q[i]]);
    }

    inline double c_alpha_h_beta_a(int i, int j, int k)
    {
        return c(i, k) + pen_alpha_h(i, j, k) + pen_beta_a(i, k);
    }

    inline bool dominated(int i, int j)
    {
        return potential[i] + myData->cli[i + 1].dreturn
            + sumDistance[j + 1] - sumDistance[i + 1]
            >= potential[j] + myData->cli[j + 1].dreturn - tolerance;
    }

    inline bool dominated_h(int i, int j)
    {
        return sumLoad[j] - sumLoad[i] + sumPickup[i]
            <= sumPickup[j] + tolerance;
    }

    inline bool dominated_a(int i, int j)
    {
        return myData->cli[i].tw_open
            + sumDuration[j] - sumDuration[i]
            <= myData->cli[j].tw_open + tolerance;
    }

    inline double initial_time_warp(int i, int j, int k)
    {
        return std::max(
            myData->cli[i + 1].dreturn
                + sumDuration[k] - sumDuration[i + 1],
            myData->cli[j].tw_open
                + sumDuration[k] - sumDuration[j])
            - std::min(
                myData->cli[k].tw_close,
                myData->time_limit - myData->cli[k].dreturn
                    - myData->cli[k].service_time);
    }

    inline bool dominated_alpha_p_beta_a(int i, int j)
    {
        return potential[i] + myData->cli[i + 1].dreturn
            - sumDistance[i + 1]
            + myData->penaltyLoad * (sumPickup[j] - sumPickup[i])
            + myData->penaltyWarp
                * (r[i] - W[q[i]] - r[j] + W[q[j]])
            >= potential[j] + myData->cli[j + 1].dreturn
                - sumDistance[j + 1] - tolerance;
    }

    inline bool cheaper_alpha_h_beta_a(int i, int j, int k)
    {
        return potential[i] + c_alpha_h_beta_a(i, h[i], k)
            < potential[j] + c_alpha_h_beta_a(j, h[j], k) - tolerance;
    }

    inline bool dominated_beta_a(int i, int j)
    {
        return potential[i] + myData->cli[i + 1].dreturn
            - sumDistance[i + 1]
            + myData->penaltyWarp * (r[i] - W[q[i]])
            >= potential[j] + myData->cli[j + 1].dreturn
                - sumDistance[j + 1]
                + myData->penaltyWarp * (r[j] - W[q[j]]) - tolerance;
    }

    inline bool better(double firstValue, int firstPred,
                       double secondValue, int secondPred)
    {
        if (firstValue < secondValue - tolerance) return true;
        if (secondValue < firstValue - tolerance) return false;
        return firstPred > secondPred;
    }

public:
    Split_Linear_Soft_VRPSPDTW(Pb_Data * myData) : Split(myData) {}
    int solve();
    const vector<double>& getPotential() const { return potential; }
    const vector<int>& getPred() const { return pred; }
};

#endif
