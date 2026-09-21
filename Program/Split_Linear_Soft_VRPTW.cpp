//--------------------------------------------------------
//SPLIT ALGORITHM EXTENSION FOR VEHICLE ROUTING PROBLEMS
//Original Author : Thibaut VIDAL (vidalt@inf.puc-rio.br)
//Original Date   : August 15th, 2015
//Original Project: https://w.cba.ulaval.ca/vidalt/
//
//Additional Split Algorithms by: *anonymized for review*
//Correspondence: *anonymized for review*
//Date        : September 2026
//
//Summary:
//This project contains a number of additional Split algorithms to the original 
//Split algorithm library by Thibaut Vidal. The code includes O(nB) and O(n)
//Split implementations for the following variants:
//     • VRPSPDTW
//     • VRPSPD with Linear Capacity Penalty
//     • VRPTW with Linear Capacity Penalty and Time Warp Penalty
//
//License: This code is distributed for research purposes.
//         Original rights reserved by the original author.
//--------------------------------------------------------

#include "Split_Linear_Soft_VRPTW.h"

int Split_Linear_Soft_VRPTW::solve()
{ 
	// Initialization of the data structures
	potential = vector <double> (myData->nbNodes+1) ; 
	pred =  vector < int > (myData->nbNodes+1) ; 
	sumDistance = vector <double> (myData->nbNodes+1) ;
	sumLoad = vector <double> (myData->nbNodes+1) ;
	sumDuration = vector <double> (myData->nbNodes+1) ;
	nodes = vector <Node> (myData->nbNodes+1) ;
	W = vector <double> (myData->nbNodes+1) ;
	q = vector <int> (myData->nbNodes+1) ;
	r = vector <double> (myData->nbNodes+1) ;

	sumLoad[0] = 0.0; 
    sumDistance[0] = 0.0; 
    sumDuration[0] = 0.0;
    W[0] = 0.0;
    q[0] = 0;
    r[0] = 0.0;
	nodes[0] = Node(0);
    double dur = myData->time_limit;
    potential[0] = 0.0;
    Convoluted_Deque_VRPTW Lambda;
    Trivial_Deque Lambda_a(myData->nbNodes + 1);

    for (int x = 1; x <=myData->nbNodes; x++) {

        nodes[x] = Node(x);
        sumLoad[x] = sumLoad[x - 1] + myData->cli[x].demand;
        sumDistance[x] = sumDistance[x - 1] + myData->cli[x - 1].dnext;
        sumDuration[x] = sumDuration[x - 1] + myData->cli[x - 1].service_time + myData->cli[x - 1].dnext;
        q[x] = 0;
        r[x] = 0.0;
        dur = std::max(dur + myData->cli[x - 1].service_time + myData->cli[x - 1].dnext, myData->cli[x].tw_open);
        if (dur > std::min(myData->cli[x].tw_close, myData->time_limit - myData->cli[x].service_time - myData->cli[x].dreturn)) {
            W[x] = W[x - 1];
            if (dur > std::min(myData->cli[x].tw_close, myData->time_limit - myData->cli[x].service_time - myData->cli[x].dreturn) + tolerance)
                W[x] += dur - std::min(myData->cli[x].tw_close, myData->time_limit - myData->cli[x].service_time - myData->cli[x].dreturn);
            dur = std::min(myData->cli[x].tw_close, myData->time_limit - myData->cli[x].service_time - myData->cli[x].dreturn);
        }
        else{
            W[x] = W[x - 1];
        }

        while (!Lambda.is_empty() && dominated(Lambda.back(), x - 1)) {
            Lambda.pop_back();
        }
        Lambda.push_back(&nodes[x - 1]);

        while (Lambda_a.size() > 0 && dominated_a(Lambda_a.get_back(), x)) {
            Lambda_a.pop_back();
        }
        Lambda_a.push_back(x);
        while (Lambda.has_no_time_warp()) {
            int candidate = Lambda.no_time_warp();
            while (Lambda_a.get_front() <= candidate) {
                Lambda_a.pop_front();
            }
            r[candidate] = std::max(0.0, initial_time_warp(candidate, Lambda_a.get_front(), x));
            if (r[candidate] <= tolerance) {
                break;
            }
            q[candidate] = x;
            Lambda.move_no_warp_next();
        }

        while (Lambda.has_feasible() && (pen_alpha(Lambda.feas(), x) > 0 || pen_beta(Lambda.feas(), x) > 0)) {

            while (Lambda.feasible_has_prev() && dominated_alpha_beta(Lambda.feasible_prev(), Lambda.feas(), x)) {

                Lambda.remove_feasible_prev();
            }
            Lambda.move_feas_next();
        }

        while (Lambda.has_feasible() && Lambda.feasible_has_prev() && dominated_alpha_beta(Lambda.feasible_prev(), Lambda.feas(), x)) {
            Lambda.remove_feasible_prev();
        }

        while (Lambda.size_greater_than_1() && pen_alpha(Lambda.front_2(), x) > 0 && pen_beta(Lambda.front_2(), x) > 0) {
            
            if (!dominated_alpha_beta(Lambda.front(), Lambda.front_2(), x)) {
                Lambda.pop_front_2();
            }
            else{
                Lambda.pop_front();
            }
        }
        if (Lambda.size_greater_than_1() && dominated_alpha_beta(Lambda.front(), Lambda.front_2(), x)) {
            Lambda.pop_front();
        }
        potential[x] = potential[Lambda.front()] + c_alpha_beta(Lambda.front(), x);
        pred[x] = Lambda.front();
    }
	// THE CORE OF THE SPLIT ALGORITHM IS FINISHED HERE, 
	//myData->nbNodesOW JUST SWEEPING THE ROUTE in O(n) TO REPORT THE SOLUTION (IN THE GOOD DIRECTION)

	if (potential[myData->nbNodes] > 1.e29)
	{
		cout << "ERROR :myData->nbNodeso Split solution has been propagated until the lastmyData->nbNodesode" << endl ;
		throw string ("ERROR :myData->nbNodeso Split solution has been propagated until the lastmyData->nbNodesode");
	}

	// Finally counting themyData->nbNodesumber of routes using the pred structure (linear complexity) 
	myData->solutionNbRoutes = 0 ;
	int cour = myData->nbNodes ;
	while (cour != 0)
	{
		cour = pred[cour] ;
		myData->solutionNbRoutes ++ ;
	}

	// And filling myData->solution in the good order (linear complexity) 
	cour = myData->nbNodes ;
	for (int i = myData->solutionNbRoutes-1 ; i >= 0 ; i --)
	{
		cour = pred[cour] ;
		myData->solution[i] = cour+1 ;
	}

	myData->solutionCost = potential[myData->nbNodes] ;
	
	return 0 ;
}
