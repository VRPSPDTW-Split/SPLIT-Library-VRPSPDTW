#ifndef SPLIT_H
#define SPLIT_H

#include "Pb_Data.h"
#include "Trivial_Deque.h"
#include <iostream>
using namespace std;

class Split {

protected:

	// Parameters of the problem
	Pb_Data * myData ;

	// Tolerance used when comparing accumulated floating-point values.
	static constexpr double tolerance = 1.e-7;

public:

	// Constructor common to all Split solvers
	Split (Pb_Data * myData) : myData(myData) {}

	// Solution methods, implemented in each solver independently
	virtual int solve (void) = 0 ;

	// Virtual destructor (needs to stay virtual otherwise inherited destructors will not be called)
	virtual ~Split() {} ;

};

#endif
