#ifndef DATA_H
#define DATA_H

#include <stdlib.h>
#include <stdio.h> 
#include <string>
#include <vector>
#include <fstream>
#include <iostream>
#include <iomanip>
#include <sstream> 
#include <time.h>
#include <math.h>

using namespace std ;

enum SolverType {
	BELLMAN_VRPSPDTW,
	BELLMAN_SOFT_CVRP,
	BELLMAN_SOFT_VRPSPD,
	BELLMAN_SOFT_VRPTW,
	BELLMAN_SOFT_VRPSPDTW,
	LINEAR_VRPSPDTW,
	LINEAR_VRPSPD,
	LINEAR_VRPTW,
	LINEAR_SOFT_CVRP,
	LINEAR_SOFT_VRPSPD,
	LINEAR_SOFT_VRPTW,
	LINEAR_SOFT_VRPSPDTW,
	LINEAR_CVRP,
	LINEAR_VIDAL,
	BELLMAN_CVRP,
	BELLMAN_VRPSPD,
	BELLMAN_VRPTW
} ;

struct Client
{
	int index ;
	double demand ;
	double pickup ; // pickup time at the client
	double dreturn ;
	double dnext ;
	double tw_open ;  // opening time window
	double tw_close ; // closing time window
	double service_time ; // service time at the client
};

class Pb_Data
{
public:

/* PROBLEM DATA */

// address of the problem instance
string pathToInstance ;

/* METHOD PARAMETERS */
// type of solver used
SolverType solverType ;

// number of nodes
int nbNodes ;

// vehicle capacity
double vehCapacity ;

// max number of vehicle (for problems with limited fleet)
int nbVehicles ;

// penalty coefficient for one unit of load excess (for problems with soft capacity constraint)
double penaltyLoad ;

// time limit for the optimization
double time_limit ;

// penalty coefficient for one unit of time warp (for problems with soft time windows)
double penaltyWarp ;

// vector of clients, i.e., locations to be visited in the TSP
vector < Client > cli ;

// Starting time of the optimization
clock_t time_StartComput ;

// End time of the optimization
clock_t time_EndComput ;

/* SOLUTION STRUCTURE */

// list of indices of clients where a route starts, -1 if not filled
vector < int > solution ;

// cost of the solution
double solutionCost ;

// number of routes in the solution
int solutionNbRoutes ;

/* METHODS TO TEST AND EXPORT THE FINAL SOLUTION */

// Method to display a solution
void printSolution()
{ 
	cout << endl ;
	cout << "------------------------------------" << endl ;
	cout << "SOLUTION COST : " << std::setprecision(12) << solutionCost << endl ;
	cout << "NB ROUTES : " << solutionNbRoutes << endl ;
	// cout << "SOLUTION : [ " ;
	// for (int i=0 ; i < solutionNbRoutes ; i++)
	// 	cout << solution[i] << " " ; 
	// cout << "]" << endl ;
	cout << "------------------------------------" << endl ;
	cout << endl ;
}

// Method to test a solution, verify the solution cost 
void checkSolution()
{ 
	double costTotal = 0 ;
	int begin, end ;
	double load, distance ;

	if (solution[0] != 1)
		cout << " ERROR : First route should start with customer 0" << endl ;

	for (int i=0 ; i < solutionNbRoutes ; i++)
	{
		begin = solution[i] ;
		if (i < solutionNbRoutes-1)
			end = solution[i+1]-1 ;
		else
			end = nbNodes ;

		load = 0 ;
		for (int j = begin ; j <= end ; j++)
			load += cli[j].demand ;

		distance = cli[begin].dreturn + cli[end].dreturn ;
		for (int j = begin ; j < end ; j++)
			distance += cli[j].dnext ;

		costTotal += distance + penaltyLoad * max<double>(load - vehCapacity, 0);
	}

	//cout << "Cost reported by the Split algorithm : " << solutionCost << endl ;
	//cout << "Cost evaluated by the solution checker : " << costTotal << endl ;

	if (costTotal > solutionCost + 0.0001 || costTotal < solutionCost - 0.0001)
		cout << "ERROR : Solution checker does not find the same solution cost" << endl ;
}

// Constructor
Pb_Data(string pathToInstance, SolverType solverType, double penaltyLoad, double penaltyWarp, double capacity, double window_factor);

// Constructor for already-parsed benchmark data in giant-tour order.
Pb_Data(string sourceName, SolverType solverType, double penaltyLoad, double penaltyWarp,
	double capacity, int numberOfVehicles, double depotClosingTime,
	const vector<Client>& orderedClients);

~Pb_Data(void);
};

#endif
