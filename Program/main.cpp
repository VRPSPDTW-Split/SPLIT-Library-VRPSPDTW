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

#include "commandline.h"
#include "Split_Bellman_Soft_CVRP.h"
#include "Split_Bellman_Soft_VRPSPD.h"
#include "Split_Bellman_Soft_VRPTW.h"
#include "Split_Bellman_Soft_VRPSPDTW.h"
#include "Split_Bellman_VRPSPDTW.h"
#include "Split_Linear.h"
#include "Split_Linear_Soft_CVRP.h"
#include "Split_Linear_Soft_VRPSPD.h"
#include "Split_Linear_Soft_VRPSPDTW.h"
#include "Split_Linear_Soft_VRPTW.h"
#include "Split_Linear_VRPSPD.h"
#include "Split_Linear_VRPSPDTW.h"
#include "Split_Linear_VRPTW.h"
#include "Split_Linear_CVRP.h"


#include <iomanip>
#include <iostream>

int main (int argc, char *argv[])
{
	Pb_Data * myData ;
	Split * mySolver ;

	commandline c(argc, argv);
	if (c.is_valid())
	{
		// Parsing the problem instance
		myData = new Pb_Data(c.get_path_to_instance(),c.get_solver_type(),c.get_penaltyLoad(),c.get_penaltyWarp(),c.get_capacity(),c.get_window_factor());

		// Create the solver data structures
		if (myData->solverType == BELLMAN_VRPSPDTW ||
			myData->solverType == BELLMAN_CVRP ||
			myData->solverType == BELLMAN_VRPSPD ||
			myData->solverType == BELLMAN_VRPTW)
			mySolver = new Split_Bellman_VRPSPDTW(myData);
		else if (myData->solverType == BELLMAN_SOFT_CVRP)
			mySolver = new Split_Bellman_Soft(myData);
		else if (myData->solverType == BELLMAN_SOFT_VRPSPD)
			mySolver = new Split_Bellman_Soft_VRPSPD(myData);
		else if (myData->solverType == BELLMAN_SOFT_VRPTW)
			mySolver = new Split_Bellman_Soft_VRPTW(myData);
		else if (myData->solverType == BELLMAN_SOFT_VRPSPDTW)
			mySolver = new Split_Bellman_Soft_VRPSPDTW(myData);
		else if (myData->solverType == LINEAR_VRPSPDTW)
			mySolver = new Split_Linear_VRPSPDTW(myData);
		else if (myData->solverType == LINEAR_VRPSPD)
			mySolver = new Split_Linear_VRPSPD(myData);
		else if (myData->solverType == LINEAR_VRPTW)
			mySolver = new Split_Linear_VRPTW(myData);
		else if (myData->solverType == LINEAR_SOFT_CVRP)
			mySolver = new Split_Linear_Soft(myData);
		else if (myData->solverType == LINEAR_SOFT_VRPSPD)
			mySolver = new Split_Linear_Soft_VRPSPD(myData);
		else if (myData->solverType == LINEAR_SOFT_VRPTW)
			mySolver = new Split_Linear_Soft_VRPTW(myData);
		else if (myData->solverType == LINEAR_SOFT_VRPSPDTW)
			mySolver = new Split_Linear_Soft_VRPSPDTW(myData);
		else if (myData->solverType == LINEAR_CVRP)
			mySolver = new Split_Linear_CVRP(myData);
		else if (myData->solverType == LINEAR_VIDAL)
			mySolver = new Split_Linear(myData);
		else
		{
			cout << "ERROR : no solver with this name" << endl ;
			throw string ("ERROR : no solver with this name") ;
		}

		// Begin timing after construction so only calls to solve are measured.
		myData->time_StartComput = clock();
		int numRuns = 0;
		// Compute here the split solution
		while(numRuns == 0 || (double)(clock() - myData->time_StartComput)/CLOCKS_PER_SEC < c.get_numSeconds()){
			mySolver->solve() ;
			numRuns ++;
		}

		// End of clock
		myData->time_EndComput = clock() ;

		// Check and Print the solution
		myData->printSolution() ;
		// myData->checkSolution() ;

		// the time measure is not precise on a single instance
		// to make more precise time measures, the code inside this function needs to be looped several times.
		cout << "Number of Runs : " << numRuns << endl ;
		cout << std::setprecision(12)
			 << "Average Millisecond Time (Only Split) : "
			 << ((double)(myData->time_EndComput - myData->time_StartComput) * 1000.0/numRuns)/CLOCKS_PER_SEC << endl ;

		delete mySolver ;
		delete myData ;
	}
	else
		throw string("ERROR : Non-valid commandline, Usage : myProgram instance [-sol solution] [-solver solverType]");
}
