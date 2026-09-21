//--------------------------------------------------------
// LIBRARY OF SPLIT ALGORITHM FOR VEHICLE ROUTING PROBLEMS
// Author : Thibaut VIDAL
// Date   : August 15th, 2015.
// E-mail : vidalt@inf.puc-rio.br
//
// Imported from vidalt/Split-Library commit
// 0005ac156a0d632cb214c5504ddd1e409bd340c9.
// Upstream license: GNU GPL v3.
// Integrated and reformatted on September 20th, 2026; solver logic unchanged.
//--------------------------------------------------------

#include "Split_Bellman_Soft_CVRP.h"

// The Split algorithm can be viewed as a shortest path in a graph with n+1 nodes.
// Any edge between nodes i and j corresponds to the trip 0, i+1, ..., j, 0.
int Split_Bellman_Soft::solve()
{
	double load, distance, cost;

	potential = vector<double>(myData->nbNodes + 1);
	pred = vector<int>(myData->nbNodes + 1);

	for (int i = 0; i < myData->nbNodes + 1; i++)
	{
		potential[i] = 1.e30;
		pred[i] = -1;
	}
	potential[0] = 0;

	for (int i = 0; i < myData->nbNodes; i++)
	{
		load = 0;
		distance = 0;
		// Optionally, one can impose a hard limit for the total load in the route,
		// e.g., 4Q, to avoid propagating to every successor.
		for (int j = i + 1; j <= myData->nbNodes; j++)
		{
			load += myData->cli[j].demand;
			if (j == i + 1)
				distance += myData->cli[j].dreturn;
			else
				distance += myData->cli[j - 1].dnext;
			cost = distance + myData->cli[j].dreturn
				+ myData->penaltyLoad * max<double>(load - myData->vehCapacity, 0);
			if (potential[i] + cost < potential[j])
			{
				potential[j] = potential[i] + cost;
				pred[j] = i;
			}
		}
	}

	if (potential[myData->nbNodes] > 1.e29)
	{
		cout << "ERROR : no Split solution has been propagated until the last node" << endl;
		throw string("ERROR : no Split solution has been propagated until the last node");
	}

	myData->solutionNbRoutes = 0;
	int cour = myData->nbNodes;
	while (cour != 0)
	{
		cour = pred[cour];
		myData->solutionNbRoutes++;
	}

	cour = myData->nbNodes;
	for (int i = myData->solutionNbRoutes - 1; i >= 0; i--)
	{
		cour = pred[cour];
		myData->solution[i] = cour + 1;
	}

	myData->solutionCost = potential[myData->nbNodes];
	return 0;
}
