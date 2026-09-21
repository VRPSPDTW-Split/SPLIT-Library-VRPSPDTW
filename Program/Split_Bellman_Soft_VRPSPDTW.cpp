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
//License: This code is distributed for research purposes.
//         Original rights reserved by the original author.
//--------------------------------------------------------

#include "Split_Bellman_Soft_VRPSPDTW.h"

// The Split algorithm can be viewed as a shortest path in a graph with n+1 nodes.
// Any edge between nodes i and j corresponds to the trip 0, i+1, ..., j, 0.
int Split_Bellman_Soft_VRPSPDTW::solve()
{
	double highest, pickup, distance, cost, duration, warp, time_limit ;

	// Initialization of the structures
	potential = vector < double > (myData->nbNodes+1) ;
	pred = vector < int > (myData->nbNodes+1) ;

	for (int i = 0 ; i < myData->nbNodes+1 ; i++)
	{
		potential[i] = 1.e30 ;
		pred[i] = -1 ;
	}
	potential[0] = 0 ;

	// Split algorithm here
	for (int i = 0 ; i < myData->nbNodes ; i++)
	{
		highest = 0 ;
		pickup = 0 ;
		distance = 0 ;
		duration = 0 ;
		warp = 0 ;
		for (int j = i+1 ; j <= myData->nbNodes ; j++)
		{
			if (j == i+1)
			{
				distance = myData->cli[j].dreturn ;
				duration = max<double>(myData->cli[j].tw_open, myData->cli[j].dreturn) ;
			}
			else
			{
				distance += myData->cli[j-1].dnext ;
				duration = max<double>(duration + myData->cli[j-1].service_time
					+ myData->cli[j-1].dnext, myData->cli[j].tw_open) ;
			}

			pickup += myData->cli[j].pickup ;
			highest += myData->cli[j].demand ;
			if (pickup > highest)
				highest = pickup ;

			time_limit = min<double>(myData->cli[j].tw_close,
				myData->time_limit - myData->cli[j].service_time - myData->cli[j].dreturn) ;
			if (duration > time_limit)
			{
				if (duration > time_limit + tolerance)
					warp += duration - time_limit ;
				duration = time_limit ;
			}

			cost = distance + myData->cli[j].dreturn
				+ (highest > myData->vehCapacity + tolerance
					? myData->penaltyLoad * (highest - myData->vehCapacity) : 0.0)
				+ myData->penaltyWarp * warp ;
			if (potential[i] + cost < potential[j])
			{
				potential[j] = potential[i] + cost ;
				pred[j] = i ;
			}
		}
	}

	// The core Split algorithm is finished. Sweep pred to report the solution.
	if (potential[myData->nbNodes] > 1.e29)
	{
		cout << "ERROR : no Split solution has been propagated until the last node" << endl ;
		throw string ("ERROR : no Split solution has been propagated until the last node") ;
	}

	myData->solutionNbRoutes = 0 ;
	int cour = myData->nbNodes ;
	while (cour != 0)
	{
		cour = pred[cour] ;
		myData->solutionNbRoutes ++ ;
	}

	cour = myData->nbNodes ;
	for (int i = myData->solutionNbRoutes-1 ; i >= 0 ; i--)
	{
		cour = pred[cour] ;
		myData->solution[i] = cour+1 ;
	}

	myData->solutionCost = potential[myData->nbNodes] ;

	return 0 ;
}
