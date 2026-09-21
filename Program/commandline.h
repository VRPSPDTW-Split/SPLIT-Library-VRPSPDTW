#ifndef COMMANDLINE_H
#define COMMANDLINE_H

#include <iostream>
#include <cstdlib>
#include <string>
#include "Pb_Data.h"
using namespace std;

class commandline
{
    private:

        // is the commandline valid ? 
        bool command_ok;

        // path of the TSP instance (simple TSPlib instance format)
        string instance_name;

		// type de solver
		SolverType solverType ;

		// unit penalty per amount off capacity excess, only necessary for problems with soft capacity constraints
		double penaltyLoad ;

		// unit penalty per amount off capacity excess, only necessary for problems with soft capacity constraints
		double penaltyWarp ;

		// number of seconds the program runs to ensure accurate timing
		int numSeconds ;

		// allows for higher capacity to artificially allow for longer routes per vehicle
		double capacity ;

		// allows for broader windows to artificially allow for longer routes per vehicle
		double window_factor ;		

		// set the name of the instance
        void set_instance_name(string to_parse);

		// set the solver type
		int set_solver_type(string to_parse);

		// display the name of the problem
        void display_problem_name(string to_parse);

    public:

        // constructor
        commandline(int argc, char* argv[]);

        // destructor
        ~commandline();

		// gets the path to the instance
        string get_path_to_instance();

		// gets the solver type
		SolverType get_solver_type();

		// get the load penalty
		double get_penaltyLoad();

		double get_penaltyWarp();

		int get_numSeconds();

		double get_capacity();

		double get_window_factor();

		// is the commandline valid ?
        bool is_valid();
};

#endif

