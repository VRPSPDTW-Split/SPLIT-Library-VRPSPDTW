
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

#include "Split_Bellman_Soft_VRPTW.h"

// The Split algorithm can be viewed as a shortest path in a graph with n+1 nodes
// any edge between nodes i and j corresponds to the trip that goes from the depot 0, i+1, ..., j, 0
int Split_Bellman_Soft_VRPTW::solve()
{ 
	double load, distance, cost, duration, warp, time_limit ;

	// Initialization of the structures
	potential = vector <double> (myData->nbNodes+1) ; 
	pred =  vector < int > (myData->nbNodes+1) ; 

	for (int i=0 ; i < myData->nbNodes+1 ; i++)
	{
		potential[i] = 1.e30 ; 
		pred[i] = -1 ;
	}
	potential[0] = 0 ;

	// Split algorithm here
	for (int i = 0 ; i < myData->nbNodes ; i++)
	{
        load = 0 ;
        duration = 0 ;
		distance = 0 ;
		warp = 0 ;
		for (int j = i+1 ; j <= myData->nbNodes; j++)
		{
            if (j == i + 1) {
                distance     = myData->cli[j].dreturn;
                duration = std::max(myData->cli[j].tw_open, myData->cli[j].dreturn);
            } else {
                distance     += myData->cli[j-1].dnext;
                duration  = std::max(duration + myData->cli[j-1].service_time + myData->cli[j-1].dnext, myData->cli[j].tw_open);
            }
            load += myData->cli[j].demand ;
            time_limit = std::min(myData->cli[j].tw_close, myData->time_limit - myData->cli[j].service_time - myData->cli[j].dreturn);
            if (duration > time_limit) {
                    if (duration > time_limit + tolerance)
						warp += duration - time_limit;
                    duration = time_limit;
            }
			cost = distance + myData->cli[j].dreturn
				+ (load > myData->vehCapacity + tolerance
					? myData->penaltyLoad * (load - myData->vehCapacity) : 0.0)
				+ myData->penaltyWarp * warp;
			if (potential[i] + cost < potential[j])
			{
				potential[j] = potential[i] + cost ;
				pred[j] = i ;
			}
		}
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
