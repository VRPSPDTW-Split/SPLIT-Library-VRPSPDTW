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
#include "Split_Linear_CVRP.h"

int Split_Linear_CVRP::solve()
{ 
	// Initialization of the data structures
	potential = vector <double> (myData->nbNodes+1) ; 
	pred =  vector < int > (myData->nbNodes+1) ; 
	sumDistance = vector <double> (myData->nbNodes+1) ;
	sumLoad = vector <double> (myData->nbNodes+1) ;
	potential[0] = 0 ;
	pred[0] = -1 ; 
	sumDistance[0] = 0 ; 
	sumLoad[0] = 0 ;  

	// Creating the queue
	Trivial_Deque Lambda = Trivial_Deque(myData->nbNodes+1);
	// Main loop of the program
	for (int x = 1 ; x <= myData->nbNodes ; x++)
	{
		sumLoad[x] = sumLoad[x-1] + myData->cli[x].demand ;
		sumDistance[x] = sumDistance[x-1] + myData->cli[x-1].dnext ;
		while (Lambda.size() > 0 && dominated(Lambda.get_back(), x - 1)) {
            Lambda.pop_back();
        }
        Lambda.push_back(x - 1);

        while (infeasible_d(Lambda.get_front(), x)) {
            Lambda.pop_front();
        }
        potential[x] = potential[Lambda.get_front()] + c(Lambda.get_front(), x);
        pred[x] = Lambda.get_front();
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
