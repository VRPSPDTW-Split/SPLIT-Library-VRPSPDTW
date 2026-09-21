#ifndef BENCHMARK_INSTANCE_READER_H
#define BENCHMARK_INSTANCE_READER_H

#include "../Program/Pb_Data.h"

#include <string>
#include <vector>

enum class ProblemVariant
{
	CVRP,
	VRPSPD,
	VRPTW,
	VRPSPDTW,
	SOFT_CVRP,
	SOFT_VRPSPD,
	SOFT_VRPTW,
	SOFT_VRPSPDTW
};

struct BenchmarkCustomer
{
	int id;
	int sourceIndex;
	double delivery;
	double pickup;
	double twOpen;
	double twClose;
	double serviceTime;
};

struct BenchmarkInstance
{
	std::string name;
	std::string sourcePath;
	double capacity;
	int numberOfVehicles;
	double depotClosingTime;
	bool hasPickupData;
	bool hasTimeWindowData;
	std::vector<BenchmarkCustomer> customers;
	std::vector<std::vector<double> > distances;
};

ProblemVariant parseProblemVariant(const std::string& value);
std::string problemVariantName(ProblemVariant variant);
bool isSoftVariant(ProblemVariant variant);
bool usesPickupData(ProblemVariant variant);
bool usesTimeWindowData(ProblemVariant variant);

BenchmarkInstance loadBenchmarkInstance(const std::string& path);

// permutation contains zero-based indices into BenchmarkInstance::customers.
Pb_Data makePermutedData(const BenchmarkInstance& instance,
	const std::vector<int>& permutation, ProblemVariant variant,
	SolverType solverType, double penaltyLoad, double penaltyWarp);

#endif
