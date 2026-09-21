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

#include "Split_Linear_Soft_VRPSPD.h"

int Split_Linear_Soft_VRPSPD::solve()
{ 
	// Initialization of the data structures
	potential = vector <double> (myData->nbNodes+1) ; 
	pred =  vector < int > (myData->nbNodes+1) ; 
	sumDistance = vector <double> (myData->nbNodes+1) ;
	sumLoad = vector <double> (myData->nbNodes+1) ;
	sumPickup = vector <double> (myData->nbNodes+1) ;
	h = vector<int> (myData->nbNodes + 1);
	nodes = vector<Node> (myData->nbNodes + 1);
	potential[0] = 0 ;
	pred[0] = -1 ; 
	sumDistance[0] = 0 ; 
	sumLoad[0] = 0 ;  
	sumPickup[0] = 0 ; 
	// Algorithm 3 initializes every highest-load candidate before its main loop.
	for (int i = 0; i <= myData->nbNodes; i++) {
		h[i] = i;
		nodes[i] = Node(i);
	}

	Convoluted_Deque_VRPSPD Lambda = Convoluted_Deque_VRPSPD();
	Trivial_Deque Lambda_h = Trivial_Deque(myData->nbNodes+1,0);
	// Main loop of the program
	for (int x = 1; x <= myData->nbNodes; x++) {
		sumLoad[x] = sumLoad[x-1] + myData->cli[x].demand ;
		sumDistance[x] = sumDistance[x-1] + myData->cli[x-1].dnext ;
		sumPickup[x] = sumPickup[x-1] + myData->cli[x].pickup ;
		while (!Lambda.is_empty() && dominated(Lambda.back(), x - 1)) {
            Lambda.pop_back();
        }
        Lambda.push_back(&nodes[x - 1]);
		if (Lambda.best() == x - 1) {
			while (Lambda.has_prev() && dominated_alpha_p(Lambda.prev(), Lambda.best())) {
                Lambda.remove_prev();
            }
		}
		while (Lambda_h.size() > 0 && dominated_h(Lambda_h.get_back(), x)) {
            Lambda_h.pop_back();
        }
        Lambda_h.push_back(x);

		if (dominated_h(h[Lambda.best()], x)) {
            h[Lambda.best()] = x;
        
			while (Lambda.has_prev() && dominated_h(h[Lambda.prev()], x)) {
				h[Lambda.prev()] = x;
				Lambda.move_prev();
				Lambda.remove_next();
			}

			if (Lambda.has_prev() && !dominated_alpha(Lambda.prev(), h[Lambda.prev()], Lambda.best(), h[Lambda.best()], x)) {
				Lambda.move_prev();
				Lambda.remove_next();
			}
		}
		if (Lambda.has_next()) {
			while (Lambda_h.get_front() < Lambda.next()) {
				Lambda_h.pop_front();
			}
        }
		while (Lambda.has_next() && pen_alpha(Lambda.next(), Lambda_h.get_front(), x) > 0) {
			if (!dominated_alpha(Lambda.best(), h[Lambda.best()], Lambda.next(), Lambda_h.get_front(), x)) {
                Lambda.remove_next();
            }
            else {
                h[Lambda.next()] = Lambda_h.get_front();
                Lambda.move_next();
				while (Lambda.has_prev() && dominated_alpha_p(Lambda.prev(), Lambda.best())) {
                    Lambda.remove_prev();
                }
            }
			if (Lambda.has_next()) {
				while (Lambda_h.get_front() < Lambda.next()) {
					Lambda_h.pop_front();
				}
			}
        }

		if (Lambda.has_next() && dominated_alpha(Lambda.best(), h[Lambda.best()], Lambda.next(), Lambda_h.get_front(), x)) {
			h[Lambda.next()] = Lambda_h.get_front();
            Lambda.move_next();
			while (Lambda.has_prev() && dominated_alpha_p(Lambda.prev(), Lambda.best())) {
                Lambda.remove_prev();
            }
        }

        potential[x] = potential[Lambda.best()] + c_alpha(Lambda.best(), h[Lambda.best()], x);
        pred[x] = Lambda.best();
    }
	// THE CORE OF THE SPLIT ALGORITHM IS FINISHED HERE, 
	// NOW JUST SWEEPING THE ROUTE in O(n) TO REPORT THE SOLUTION (IN THE GOOD DIRECTION)

	if (potential[myData->nbNodes] > 1.e29)
	{
		cout << "ERROR : no Split solution has been propagated until the last node" << endl ;
		throw string ("ERROR : no Split solution has been propagated until the last node");
	}

	// Finally counting the number of routes using the pred structure (linear complexity) 
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
