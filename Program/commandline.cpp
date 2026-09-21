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

#include <cmath>

void commandline::set_instance_name(string to_parse)
{ instance_name = to_parse; }

int commandline::set_solver_type(string to_parse)
{ 

	if (to_parse == "BELLMAN_VRPSPDTW")
		solverType = BELLMAN_VRPSPDTW ;
	else if (to_parse == "BELLMAN_CVRP")
		solverType = BELLMAN_CVRP ;
	else if (to_parse == "BELLMAN_VRPSPD")
		solverType = BELLMAN_VRPSPD ;
	else if (to_parse == "BELLMAN_VRPTW")
		solverType = BELLMAN_VRPTW ;
	else if (to_parse == "BELLMAN_SOFT_CVRP")
		solverType = BELLMAN_SOFT_CVRP ;
	else if (to_parse == "BELLMAN_SOFT_VRPSPD")
		solverType = BELLMAN_SOFT_VRPSPD ;
	else if (to_parse == "BELLMAN_SOFT_VRPTW")
		solverType = BELLMAN_SOFT_VRPTW ;
	else if (to_parse == "BELLMAN_SOFT_VRPSPDTW")
		solverType = BELLMAN_SOFT_VRPSPDTW ;
	else if (to_parse == "LINEAR_VRPSPDTW")
		solverType = LINEAR_VRPSPDTW ;
	else if (to_parse == "LINEAR_VRPSPD")
		solverType = LINEAR_VRPSPD ;
	else if (to_parse == "LINEAR_VRPTW")
		solverType = LINEAR_VRPTW ;
	else if (to_parse == "LINEAR_SOFT_CVRP")
		solverType = LINEAR_SOFT_CVRP ;
	else if (to_parse == "LINEAR_SOFT_VRPSPD")
		solverType = LINEAR_SOFT_VRPSPD ;
	else if (to_parse == "LINEAR_SOFT_VRPTW")
		solverType = LINEAR_SOFT_VRPTW ;
	else if (to_parse == "LINEAR_SOFT_VRPSPDTW")
		solverType = LINEAR_SOFT_VRPSPDTW ;
	else if (to_parse == "LINEAR_CVRP")
		solverType = LINEAR_CVRP ;
	else if (to_parse == "LINEAR_VIDAL")
		solverType = LINEAR_VIDAL ;

	
	else
		return -1; // problem

	return 0 ; // OK
}

void commandline::display_problem_name(string to_parse)
{ 
    char caractere1 = '/' ;
    char caractere2 = '\\' ;

    int position = (int)to_parse.find_last_of(caractere1) ;
	int position2 = (int)to_parse.find_last_of(caractere2) ;
	if (position2 > position) position = position2 ;

	if (position != -1)
		cout << "INSTANCE     : " << to_parse.substr(position+1,to_parse.length() - 1)  << endl ;
	else
		cout << "INSTANCE     : " << to_parse << endl ;
}

// constructeur
commandline::commandline(int argc, char* argv[])
{
	bool isNameSpecified = false ;
	
	if (argc%2 != 0 || argc > 14 || argc < 2)
	{
		cout << "ERROR : invalid command line" << endl ;
		cout << "USAGE : ./executable path_to_instance [-solver solver_type] [-alpha capacity_penalty_factor] [-beta warp_penalty_factor] [-capacity vehicle_capacity] [-window_factor multiplier_to_closing_windows] [-seconds number_of_seconds]" << endl ;
		throw string ("ERROR : invalid command line") ;
        command_ok = false;
	}
	else
	{
		// Default values
		set_instance_name(string(argv[1]));
		display_problem_name(string(argv[1]));
		penaltyLoad = 1.e30 ;
		penaltyWarp = 1.e30 ;
		numSeconds = 1 ;
		capacity = -1 ;
		window_factor = 1;

		// parameters
		for ( int i = 2 ; i < argc ; i += 2 )
		{
			if ( string(argv[i]) == "-solver" )
			{
				if (set_solver_type(string(argv[i+1])) != 0)
					throw string ("ERROR : Unrecognized solver type : " + string(argv[i+1])) ;
				cout << "ALGORITHM    : " << argv[i+1] << endl ;
				isNameSpecified = true ;
			}
			else if ( string(argv[i]) == "-alpha" )
			{
				penaltyLoad = atof(argv[i+1]);
				cout << "PENALTY LOAD : " << penaltyLoad << endl ;
			}
			else if ( string(argv[i]) == "-beta" )
			{
				penaltyWarp = atof(argv[i+1]);
				cout << "PENALTY WARP : " << penaltyWarp << endl ;
			}
			else if ( string(argv[i]) == "-capacity" )
			{
				capacity = atof(argv[i+1]);
				cout << "SET CAPACITY : " << capacity << endl ;
			}
			else if ( string(argv[i]) == "-window_factor" )
			{
				window_factor = atof(argv[i+1]);
				cout << "window multiplier : " << window_factor << endl ;
			}
			else if ( string(argv[i]) == "-seconds" )
			{
				numSeconds = atoi(argv[i+1]);
				cout << "Guaranteed number of Seconds : " << numSeconds << endl ;
			}
			else
			{
				throw string ("ERROR : Unrecognized command : " + string(argv[i])) ;
				command_ok = false ;
			}
		}

		if (!isNameSpecified)
		{
			solverType = LINEAR_VRPSPDTW ;
			cout << "ALGORITHM    : LINEAR_VRPSPDTW " << endl ;
		}

		if (penaltyLoad == 1.e30 && (solverType == BELLMAN_SOFT_CVRP || solverType == LINEAR_SOFT_CVRP || solverType == BELLMAN_SOFT_VRPSPD || solverType == LINEAR_SOFT_VRPSPD || solverType == BELLMAN_SOFT_VRPTW || solverType == LINEAR_SOFT_VRPTW || solverType == BELLMAN_SOFT_VRPSPDTW || solverType == LINEAR_SOFT_VRPSPDTW)) 
		{
			cout << "ERROR : For solvers with soft capacity constraints, need to specify the penalty factor using '-alpha XXX' in the commandline" << endl ;
			throw string ("ERROR : For solvers with soft capacity constraints, need to specify the penalty factor using '-alpha XXX' in the commandline") ;
		}
		if (penaltyWarp == 1.e30 && (solverType == BELLMAN_SOFT_VRPTW || solverType == LINEAR_SOFT_VRPTW || solverType == BELLMAN_SOFT_VRPSPDTW || solverType == LINEAR_SOFT_VRPSPDTW)) 
		{
			cout << "ERROR : For solvers with soft time warp constraints, need to specify the penalty factor using '-beta XXX' in the commandline" << endl ;
			throw string ("ERROR : For solvers with soft time warp constraints, need to specify the penalty factor using '-beta XXX' in the commandline") ;
		}
		if ((solverType == BELLMAN_SOFT_VRPSPDTW || solverType == LINEAR_SOFT_VRPSPDTW) &&
			(!std::isfinite(penaltyLoad) || penaltyLoad <= 0.0))
		{
			cout << "ERROR : The soft VRPSPDTW load penalty must be positive" << endl ;
			throw string ("ERROR : The soft VRPSPDTW load penalty must be positive") ;
		}
		if ((solverType == BELLMAN_SOFT_VRPSPDTW || solverType == LINEAR_SOFT_VRPSPDTW) &&
			(!std::isfinite(penaltyWarp) || penaltyWarp <= 0.0))
		{
			cout << "ERROR : The soft VRPSPDTW time-warp penalty must be positive" << endl ;
			throw string ("ERROR : The soft VRPSPDTW time-warp penalty must be positive") ;
		}
		command_ok = true;
	}
}

commandline::~commandline(){}

string commandline::get_path_to_instance()
{
    return instance_name;
}

SolverType commandline::get_solver_type()
{
	return solverType ;
}

double commandline::get_penaltyLoad()
{
	return penaltyLoad ;
}

double commandline::get_penaltyWarp()
{
	return penaltyWarp ;
}

double commandline::get_capacity()
{
	return capacity ;
}

double commandline::get_window_factor()
{
	return window_factor ;
}

int commandline::get_numSeconds()
{
	return numSeconds ;
}

bool commandline::is_valid()
{
    return command_ok;
}
