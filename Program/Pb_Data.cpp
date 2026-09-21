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

#include "Pb_Data.h"

Pb_Data::Pb_Data(string pathToInstance, SolverType solverType, double penaltyLoad, double penaltyWarp, double capacity, double window_factor) : pathToInstance(pathToInstance), solverType(solverType), penaltyLoad(penaltyLoad), penaltyWarp(penaltyWarp)
{
	// For now it's hard coded, but should be a variable of the problem or a commandline input
	ifstream fichier ;
	ofstream outFile ;
	string contenu ;
	string uselessStr ;

	// to generate different demands for each instance
	srand((unsigned int)time(NULL)); 

	/* parsing the TSPlib instance and deriving node and depot informations */
	fichier.open(pathToInstance.c_str());
	if (fichier.is_open())
	{
		// just skipping useless entries in the header and filling the general problem data
		getline(fichier, contenu);
		getline(fichier, contenu);
		fichier >>  uselessStr ;
		fichier >>  uselessStr ;
		fichier >>  nbNodes ;
		fichier >>  uselessStr ;
		fichier >>  uselessStr ;
		fichier >>  vehCapacity ;
		fichier >>  uselessStr ;
		fichier >>  uselessStr ;
		fichier >> time_limit ;
		getline(fichier, contenu);
		getline(fichier, contenu);

		// creating the data structure for clients
		// to match the index of the auxiliary graph, the node 0 in the vector of Clients will be a sentinel (which can somehow stand for the depot)
		cli = vector <Client> (nbNodes+1) ;
		cli[0].demand = 0 ;
		cli[0].dreturn = 0 ;
		cli[0].dnext = 0 ;
		cli[0].index = 0 ;
		for (int i = 1 ; i <= nbNodes ; i++)
		{
			fichier >> cli[i].index ;
			fichier >> cli[i].demand ;
			fichier >> cli[i].pickup ;
			fichier >> cli[i].tw_open ;
			fichier >> cli[i].tw_close ;
			fichier >> cli[i].service_time ;	
			fichier >> cli[i].dreturn ;
			if (i < nbNodes)
				fichier >> cli[i].dnext ;
			else
				cli[i].dnext = -1 ;
		}
		cli[0].dnext = cli[1].dreturn ; // the distance from the depot to the first client

		// little debugging test
		fichier >> uselessStr ;
		if (!(uselessStr == "EOF"))
		{
			cout << "ERROR when reading instance, not finding EOF when it should be" << endl ;
			throw string ("ERROR when reading instance, not finding EOF when it should be");
		}

		// if there are too many vehicles, simply reducing it to a more reasonable value
		if (nbVehicles > nbNodes) 
			nbVehicles = nbNodes ;

		// creating the solution structure
		solution = vector <int> (nbNodes) ;
		for (int i=0 ; i < nbNodes ; i++)
			solution[i] = -1 ;

		fichier.close();
	}
	else 
	{
		cout << "ERROR : Impossible to find instance file : " << pathToInstance << endl ;
		throw string("ERROR : Impossible to find instance file : " + pathToInstance);
	}
	if (capacity > 0)
	{
		vehCapacity = capacity;
	}
	if (window_factor != 1.0)
	{
		for (int i = 0; i <= nbNodes; i++){
			cli[i].tw_close *= window_factor;
		}
		time_limit *= window_factor;
	}
	if (solverType == BELLMAN_CVRP || solverType == BELLMAN_VRPTW)
	{
		for (int i = 0; i <= nbNodes; i++)
			cli[i].pickup = 0.0;
	}
	if (solverType == BELLMAN_CVRP || solverType == BELLMAN_VRPSPD)
	{
		time_limit = 1.e30;
		for (int i = 0; i <= nbNodes; i++)
		{
			cli[i].tw_open = 0.0;
			cli[i].tw_close = 1.e30;
			cli[i].service_time = 0.0;
		}
	}
}

Pb_Data::Pb_Data(string sourceName, SolverType solverType, double penaltyLoad, double penaltyWarp,
	double capacity, int numberOfVehicles, double depotClosingTime,
	const vector<Client>& orderedClients)
	: pathToInstance(sourceName), solverType(solverType), nbNodes((int)orderedClients.size() - 1),
	  vehCapacity(capacity), nbVehicles(numberOfVehicles), penaltyLoad(penaltyLoad),
	  time_limit(depotClosingTime), penaltyWarp(penaltyWarp), cli(orderedClients),
	  time_StartComput(0), time_EndComput(0), solutionCost(0.0), solutionNbRoutes(0)
{
	if (orderedClients.empty())
		throw string("ERROR : Parsed benchmark data must contain a sentinel client");

	solution = vector<int>(nbNodes, -1);
}

Pb_Data::~Pb_Data(void)
{}
