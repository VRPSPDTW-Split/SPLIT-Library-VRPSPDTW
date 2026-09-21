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
#include "Split_Linear_VRPSPD.h"

int Split_Linear_VRPSPD::solve()
{
	potential = vector<double>(myData->nbNodes + 1);
	pred = vector<int>(myData->nbNodes + 1);
	sumDistance = vector<double>(myData->nbNodes + 1);
	sumLoad = vector<double>(myData->nbNodes + 1);
	sumPickup = vector<double>(myData->nbNodes + 1);
	potential[0] = 0;
	pred[0] = -1;
	sumDistance[0] = 0;
	sumLoad[0] = 0;
	sumPickup[0] = 0;

	Trivial_Deque Lambda(myData->nbNodes + 1);
	Trivial_Deque Lambda_h(myData->nbNodes + 1);
	Lambda_h.push_back(0);

	for (int x = 1; x <= myData->nbNodes; x++)
	{
		sumLoad[x] = sumLoad[x - 1] + myData->cli[x].demand;
		sumDistance[x] = sumDistance[x - 1] + myData->cli[x - 1].dnext;
		sumPickup[x] = sumPickup[x - 1] + myData->cli[x].pickup;

		while (Lambda.size() > 0 && dominated(Lambda.get_back(), x - 1))
		{
			Lambda.pop_back();
		}
		Lambda.push_back(x - 1);

		while (Lambda_h.size() > 0 && dominated_h(Lambda_h.get_back(), x))
		{
			Lambda_h.pop_back();
		}
		Lambda_h.push_back(x);

		while (Lambda_h.get_front() < Lambda.get_front())
		{
			Lambda_h.pop_front();
		}

		while (infeasible_h(Lambda.get_front(), Lambda_h.get_front(), x))
		{
			Lambda.pop_front();
			while (Lambda_h.get_front() < Lambda.get_front())
			{
				Lambda_h.pop_front();
			}
		}

		potential[x] = potential[Lambda.get_front()] + c(Lambda.get_front(), x);
		pred[x] = Lambda.get_front();
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
