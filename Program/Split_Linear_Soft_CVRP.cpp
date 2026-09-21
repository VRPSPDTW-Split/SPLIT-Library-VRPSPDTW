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

#include "Split_Linear_Soft_CVRP.h"

int Split_Linear_Soft::solve()
{
	potential = vector<double>(myData->nbNodes + 1);
	pred = vector<int>(myData->nbNodes + 1);
	sumDistance = vector<double>(myData->nbNodes + 1);
	sumLoad = vector<double>(myData->nbNodes + 1);
	potential[0] = 0;
	pred[0] = -1;
	sumDistance[0] = 0;
	sumLoad[0] = 0;

	for (int i = 1; i <= myData->nbNodes; i++)
	{
		potential[i] = 1.e30;
		pred[i] = -1;
		sumLoad[i] = sumLoad[i - 1] + myData->cli[i].demand;
		sumDistance[i] = sumDistance[i - 1] + myData->cli[i - 1].dnext;
	}

	Trivial_Deque queue = Trivial_Deque(myData->nbNodes + 1, 0);

	for (int i = 1; i <= myData->nbNodes; i++)
	{
		potential[i] = propagate(queue.get_front(), i);
		pred[i] = queue.get_front();

		if (i < myData->nbNodes)
		{
			if (!dominates(queue.get_back(), i))
			{
				while (queue.size() > 0 && dominatesRight(queue.get_back(), i))
					queue.pop_back();
				queue.push_back(i);
			}
			while (queue.size() > 1
				&& propagate(queue.get_front(), i + 1)
					> propagate(queue.get_next_front(), i + 1) - 0.0001)
				queue.pop_front();
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
