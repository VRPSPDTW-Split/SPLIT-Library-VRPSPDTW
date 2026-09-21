#include "Split_Linear_Soft_VRPSPDTW.h"

#include <algorithm>

int Split_Linear_Soft_VRPSPDTW::solve()
{
    // Initialization of the data structures
    potential = vector <double> (myData->nbNodes + 1);
    pred = vector <int> (myData->nbNodes + 1);
    sumDistance = vector <double> (myData->nbNodes + 1);
    sumLoad = vector <double> (myData->nbNodes + 1);
    sumPickup = vector <double> (myData->nbNodes + 1);
    sumDuration = vector <double> (myData->nbNodes + 1);
    r = vector <double> (myData->nbNodes + 1);
    q = vector <int> (myData->nbNodes + 1);
    W = vector <double> (myData->nbNodes + 1);
    h = vector <int> (myData->nbNodes + 1);

    potential[0] = 0.0;
    pred[0] = -1;
    sumDistance[0] = 0.0;
    sumLoad[0] = 0.0;
    sumPickup[0] = 0.0;
    sumDuration[0] = 0.0;
    r[0] = 0.0;
    q[0] = 0;
    W[0] = 0.0;
    h[0] = 0;


    Trivial_Deque Lambda_1(myData->nbNodes + 1);
    Trivial_Deque Lambda_2(myData->nbNodes + 1);
    Trivial_Deque Lambda_3(myData->nbNodes + 1);
    Outer_Deque_VRPSPDTW Gamma_2;
    Gamma_2.reset(myData->nbNodes, myData->penaltyLoad,
                  sumPickup, sumLoad);

    Trivial_Deque Lambda_a(myData->nbNodes + 1);
    Trivial_Deque Lambda_h(myData->nbNodes + 1, 0);

    int firstUnpenalizedLoad = 0; // oldest surviving predecessor not penalized by capacity
    int firstUnpenalizedTime = 0; // oldest surviving predecessor not penalized by time warp
    double dur = 0.0;

    for (int x = 1; x <= myData->nbNodes; ++x)
    {
        // preprocessing step
        sumDistance[x] = sumDistance[x - 1] + myData->cli[x - 1].dnext;
        sumLoad[x] = sumLoad[x - 1] + myData->cli[x].demand;
        sumPickup[x] = sumPickup[x - 1] + myData->cli[x].pickup;
        sumDuration[x] = sumDuration[x - 1] + myData->cli[x - 1].service_time + myData->cli[x - 1].dnext;
        dur = std::max(
            dur + myData->cli[x - 1].service_time
                + myData->cli[x - 1].dnext,
            myData->cli[x].tw_open);
        if (dur > std::min(
                myData->cli[x].tw_close,
                myData->time_limit - myData->cli[x].service_time
                    - myData->cli[x].dreturn))
        {
            W[x] = W[x - 1];
            if (dur > std::min(
                    myData->cli[x].tw_close,
                    myData->time_limit - myData->cli[x].service_time
                        - myData->cli[x].dreturn) + tolerance)
                W[x] += dur - std::min(
                    myData->cli[x].tw_close,
                    myData->time_limit - myData->cli[x].service_time
                        - myData->cli[x].dreturn);
            dur = std::min(
                myData->cli[x].tw_close,
                myData->time_limit - myData->cli[x].service_time
                    - myData->cli[x].dreturn);
        }
        else
            W[x] = W[x - 1];
        h[x] = x;
        // done preprocessing

        // Maintain Y3 
        while (Lambda_3.size() > 0)
        {
            if (!dominated(Lambda_3.get_back(), x - 1))
                break;
            Lambda_3.pop_back();
        }

        if (Lambda_3.size() == 0)
        {
            if (!Gamma_2.empty())
            {
                while (!Gamma_2.empty())
                {
                    if (!dominated(Gamma_2.back(), x - 1))
                        break;
                    Gamma_2.popBack();
                }
            }
            else
            {
                while (Lambda_2.size() > 0)
                {
                    if (!dominated(Lambda_2.get_back(), x - 1))
                        break;
                    Lambda_2.pop_back();
                }
            }

            if (Gamma_2.empty() && Lambda_2.size() == 0)
            {
                while (Lambda_1.size() > 0)
                {
                    if (!dominated(Lambda_1.get_back(), x - 1))
                        break;
                    Lambda_1.pop_back();
                }
            }
        }
        Lambda_3.push_back(x - 1);
        // Lambda_3 is now maintained.

        while (Lambda_h.size() > 0 &&
               dominated_h(Lambda_h.get_back(), x))
            Lambda_h.pop_back();
        Lambda_h.push_back(x);

        if (!Gamma_2.empty())
            Gamma_2.updateLoads(x);

        while (Lambda_a.size() > 0 &&
               dominated_a(Lambda_a.get_back(), x))
            Lambda_a.pop_back();
        Lambda_a.push_back(x);

        // Maintain predecessors already in Lambda_1.
        if (Lambda_1.size() > 0 &&
            dominated_h(h[Lambda_1.get_back()], x))
        {
            h[Lambda_1.get_back()] = x;
            while (Lambda_1.size() > 1 &&
                   dominated_h(h[Lambda_1.get_next_back()], x))
            {
                Lambda_1.pop_back();
                h[Lambda_1.get_back()] = x;
            }
            if (Lambda_1.size() > 1 &&
                cheaper_alpha_h_beta_a(
                    Lambda_1.get_next_back(), Lambda_1.get_back(), x))
                Lambda_1.pop_back();
        }
        // done maintaining predecessors already in Lambda_1

        // Move predecessors that just became penalized by time warp.
        if (!Gamma_2.empty())
            firstUnpenalizedTime = Gamma_2.front();
        else
            firstUnpenalizedTime = Lambda_3.get_front();

        while (Lambda_a.get_front() <= firstUnpenalizedTime)
            Lambda_a.pop_front();

        r[firstUnpenalizedTime] = initial_time_warp(
            firstUnpenalizedTime, Lambda_a.get_front(), x);

        while (r[firstUnpenalizedTime] > tolerance)
        {
            q[firstUnpenalizedTime] = x;

            if (!Gamma_2.empty())
            {
                Gamma_2.popFront(h[firstUnpenalizedTime]);
                //maintain Lambda_1 for predecessors entering it
                if (Lambda_1.size() == 0 ||
                    !cheaper_alpha_h_beta_a(
                        Lambda_1.get_back(), firstUnpenalizedTime, x))
                {
                    while (Lambda_1.size() > 0 &&
                           dominated_alpha_p_beta_a(
                               Lambda_1.get_back(), firstUnpenalizedTime))
                        Lambda_1.pop_back();
                    Lambda_1.push_back(firstUnpenalizedTime);
                }
            }
            else
            {
                Lambda_3.pop_front();
                //maintain Lambda_2 for predecessors entering it
                while (Lambda_2.size() > 0 &&
                       dominated_beta_a(
                           Lambda_2.get_back(), firstUnpenalizedTime))
                    Lambda_2.pop_back();
                Lambda_2.push_back(firstUnpenalizedTime);
            }
            // increment firstUnpenalizedTime to the next surviving predecessor
            if (!Gamma_2.empty())
                firstUnpenalizedTime = Gamma_2.front();
            else
                firstUnpenalizedTime = Lambda_3.get_front();

            while (Lambda_a.get_front() <= firstUnpenalizedTime)
                Lambda_a.pop_front();

            r[firstUnpenalizedTime] = initial_time_warp(
                firstUnpenalizedTime, Lambda_a.get_front(), x);
        }

        // Move predecessors that just became penalized by capacity.
        if (Lambda_2.size() > 0)
            firstUnpenalizedLoad = Lambda_2.get_front();
        else
            firstUnpenalizedLoad = Lambda_3.get_front();

        while (Lambda_h.get_front() < firstUnpenalizedLoad)
            Lambda_h.pop_front();

        while (pen_alpha_h(firstUnpenalizedLoad,
                           Lambda_h.get_front(), x) > 0.0)
        {
            const int z = Lambda_h.get_front();
            if (Lambda_2.size() > 0)
            {
                h[firstUnpenalizedLoad] = z;
                Lambda_2.pop_front();

                if (Lambda_1.size() == 0 ||
                    !cheaper_alpha_h_beta_a(
                        Lambda_1.get_back(), firstUnpenalizedLoad, x))
                {
                    while (Lambda_1.size() > 0 &&
                           dominated_alpha_p_beta_a(
                               Lambda_1.get_back(), firstUnpenalizedLoad))
                        Lambda_1.pop_back();
                    Lambda_1.push_back(firstUnpenalizedLoad);
                }
            }
            else
            {
                Lambda_3.pop_front();
                Gamma_2.pushBack(
                    firstUnpenalizedLoad, z,
                    potential[firstUnpenalizedLoad]
                        + myData->cli[firstUnpenalizedLoad + 1].dreturn
                        - sumDistance[firstUnpenalizedLoad + 1]
                        - myData->penaltyLoad
                            * sumPickup[firstUnpenalizedLoad]);
            }

            if (Lambda_2.size() > 0)
                firstUnpenalizedLoad = Lambda_2.get_front();
            else
                firstUnpenalizedLoad = Lambda_3.get_front();

            while (Lambda_h.get_front() < firstUnpenalizedLoad)
                Lambda_h.pop_front();
        }

        // Compare Lambda_1.back, Gamma_2.minimum, Lambda_2.front, and
        // Lambda_3.front.
        const double common_cost = sumDistance[x] + myData->cli[x].dreturn;
        double bestValue = 1.e30;
        int bestPred = -1;

        if (Lambda_1.size() > 0)
        {
            const int i = Lambda_1.get_back();
            const double value = potential[i]
                + c_alpha_h_beta_a(i, h[i], x);
            if (better(value, i, bestValue, bestPred))
            {
                bestValue = value;
                bestPred = i;
            }
        }
        if (!Gamma_2.empty())
        {
            const Outer_Deque_VRPSPDTW::Minimum minimum = Gamma_2.minimum();
            const int i = minimum.predecessor;
            const double value = common_cost
                + myData->penaltyLoad
                    * (sumLoad[x] - myData->vehCapacity)
                + minimum.score;
            if (better(value, i, bestValue, bestPred))
            {
                bestValue = value;
                bestPred = i;
            }
        }
        if (Lambda_2.size() > 0)
        {
            const int i = Lambda_2.get_front();
            const double value = potential[i] + c(i, x)
                + pen_beta_a(i, x);
            if (better(value, i, bestValue, bestPred))
            {
                bestValue = value;
                bestPred = i;
            }
        }
        if (Lambda_3.size() > 0)
        {
            const int i = Lambda_3.get_front();
            const double value = potential[i] + c(i, x);
            if (better(value, i, bestValue, bestPred))
            {
                bestValue = value;
                bestPred = i;
            }
        }

        potential[x] = bestValue;
        pred[x] = bestPred;
    }

    myData->solutionNbRoutes = 0;
    int cour = myData->nbNodes;
    while (cour != 0)
    {
        cour = pred[cour];
        ++myData->solutionNbRoutes;
    }

    cour = myData->nbNodes;
    for (int i = myData->solutionNbRoutes - 1; i >= 0; --i)
    {
        cour = pred[cour];
        myData->solution[i] = cour + 1;
    }
    myData->solutionCost = potential[myData->nbNodes];
    return 0;
}
