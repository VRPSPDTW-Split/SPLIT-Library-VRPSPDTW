//--------------------------------------------------------
// Correctness driver for the Split algorithm implementations.
//
// Native benchmark instances are loaded, deterministic customer permutations
// are generated, and the linear implementation is compared with the existing
// quadratic Bellman implementation on every permutation.
//--------------------------------------------------------

#include "../Benchmark-Instances/BenchmarkInstanceReader.h"
#include "Split.h"
#include "Split_Bellman_Soft_CVRP.h"
#include "Split_Bellman_Soft_VRPSPD.h"
#include "Split_Bellman_Soft_VRPSPDTW.h"
#include "Split_Bellman_Soft_VRPTW.h"
#include "Split_Bellman_VRPSPDTW.h"
#include "Split_Linear_CVRP.h"
#include "Split_Linear_Soft_CVRP.h"
#include "Split_Linear_Soft_VRPSPD.h"
#include "Split_Linear_Soft_VRPSPDTW.h"
#include "Split_Linear_Soft_VRPTW.h"
#include "Split_Linear_VRPSPD.h"
#include "Split_Linear_VRPSPDTW.h"
#include "Split_Linear_VRPTW.h"

#include <algorithm>
#include <cstdlib>
#include <cmath>
#include <iomanip>
#include <iostream>
#include <memory>
#include <numeric>
#include <random>
#include <stdexcept>
#include <string>
#include <vector>

namespace
{
	const unsigned int PERMUTATION_SEED = 42;

	struct CorrectnessOptions
	{
		std::string instancePath;
		ProblemVariant variant;
		bool variantSpecified;
		int numberOfPermutations;
		bool permutationsSpecified;
		double penaltyLoad;
		double penaltyWarp;
		bool loadPenaltySpecified;
		bool warpPenaltySpecified;

		CorrectnessOptions()
			: variant(ProblemVariant::CVRP), variantSpecified(false),
			  numberOfPermutations(0), permutationsSpecified(false),
			  penaltyLoad(0.0), penaltyWarp(0.0),
			  loadPenaltySpecified(false), warpPenaltySpecified(false)
		{}
	};

	void printUsage(std::ostream& output)
	{
		output << "Usage:\n"
		       << "  ./split_correctness INSTANCE --variant VARIANT --num_permutations N"
		          " [--alpha LOAD_PENALTY] [--beta WARP_PENALTY]\n\n"
		       << "Variants:\n"
		       << "  CVRP, VRPSPD, VRPTW, VRPSPDTW\n"
		       << "  Soft_CVRP, Soft_VRPSPD, Soft_VRPTW, Soft_VRPSPDTW\n\n"
		       << "Soft_CVRP and Soft_VRPSPD require --alpha.\n"
		       << "Soft_VRPTW and Soft_VRPSPDTW require --alpha and --beta.\n";
	}

	int parsePositiveInteger(const std::string& value, const std::string& option)
	{
		std::size_t consumed = 0;
		const int parsed = std::stoi(value, &consumed);
		if (consumed != value.size() || parsed <= 0)
			throw std::runtime_error(option + " must be a positive integer");
		return parsed;
	}

	double parseNonnegativeDouble(const std::string& value, const std::string& option)
	{
		std::size_t consumed = 0;
		const double parsed = std::stod(value, &consumed);
		if (consumed != value.size() || !std::isfinite(parsed) || parsed < 0.0)
			throw std::runtime_error(option + " must be a finite, nonnegative number");
		return parsed;
	}

	CorrectnessOptions parseOptions(int argc, char* argv[])
	{
		CorrectnessOptions options;
		for (int i = 1; i < argc; ++i)
		{
			const std::string argument(argv[i]);
			if (argument == "--help" || argument == "-h")
			{
				printUsage(std::cout);
				std::exit(0);
			}

			if (argument == "--variant")
			{
				if (++i >= argc) throw std::runtime_error("Missing value after --variant");
				options.variant = parseProblemVariant(argv[i]);
				options.variantSpecified = true;
			}
			else if (argument == "--num_permutations")
			{
				if (++i >= argc) throw std::runtime_error("Missing value after --num_permutations");
				options.numberOfPermutations = parsePositiveInteger(argv[i], "--num_permutations");
				options.permutationsSpecified = true;
			}
			else if (argument == "--instance")
			{
				if (++i >= argc) throw std::runtime_error("Missing value after --instance");
				if (!options.instancePath.empty()) throw std::runtime_error("Instance path was specified more than once");
				options.instancePath = argv[i];
			}
			else if (argument == "--alpha" || argument == "-alpha")
			{
				if (++i >= argc) throw std::runtime_error("Missing value after " + argument);
				options.penaltyLoad = parseNonnegativeDouble(argv[i], argument);
				options.loadPenaltySpecified = true;
			}
			else if (argument == "--beta" || argument == "-beta")
			{
				if (++i >= argc) throw std::runtime_error("Missing value after " + argument);
				options.penaltyWarp = parseNonnegativeDouble(argv[i], argument);
				options.warpPenaltySpecified = true;
			}
			else if (!argument.empty() && argument[0] == '-')
				throw std::runtime_error("Unrecognized option: " + argument);
			else
			{
				if (!options.instancePath.empty()) throw std::runtime_error("Instance path was specified more than once");
				options.instancePath = argument;
			}
		}

		if (options.instancePath.empty()) throw std::runtime_error("An instance file is required");
		if (!options.variantSpecified) throw std::runtime_error("--variant is required");
		if (!options.permutationsSpecified) throw std::runtime_error("--num_permutations is required");

		if (isSoftVariant(options.variant) && !options.loadPenaltySpecified)
			throw std::runtime_error("Soft variants require the load penalty --alpha");
		if ((options.variant == ProblemVariant::SOFT_VRPTW ||
			 options.variant == ProblemVariant::SOFT_VRPSPDTW) && !options.warpPenaltySpecified)
			throw std::runtime_error("Soft variants with time windows require the warp penalty --beta");
		if (options.variant == ProblemVariant::SOFT_VRPSPDTW && options.penaltyLoad <= 0.0)
			throw std::runtime_error("Soft_VRPSPDTW requires a positive load penalty --alpha");
		if (options.variant == ProblemVariant::SOFT_VRPSPDTW && options.penaltyWarp <= 0.0)
			throw std::runtime_error("Soft_VRPSPDTW requires a positive warp penalty --beta");
		if (!isSoftVariant(options.variant) && (options.loadPenaltySpecified || options.warpPenaltySpecified))
			throw std::runtime_error("--alpha and --beta are only valid for Soft_* variants");
		return options;
	}

	SolverType referenceSolverType(ProblemVariant variant)
	{
		if (variant == ProblemVariant::SOFT_CVRP)
			return BELLMAN_SOFT_CVRP;
		if (variant == ProblemVariant::SOFT_VRPSPD)
			return BELLMAN_SOFT_VRPSPD;
		if (variant == ProblemVariant::SOFT_VRPTW)
			return BELLMAN_SOFT_VRPTW;
		if (variant == ProblemVariant::SOFT_VRPSPDTW)
			return BELLMAN_SOFT_VRPSPDTW;
		return BELLMAN_VRPSPDTW;
	}

	SolverType linearSolverType(ProblemVariant variant)
	{
		switch (variant)
		{
			case ProblemVariant::CVRP: return LINEAR_CVRP;
			case ProblemVariant::VRPSPD: return LINEAR_VRPSPD;
			case ProblemVariant::VRPTW: return LINEAR_VRPTW;
			case ProblemVariant::VRPSPDTW: return LINEAR_VRPSPDTW;
			case ProblemVariant::SOFT_CVRP: return LINEAR_SOFT_CVRP;
			case ProblemVariant::SOFT_VRPSPD: return LINEAR_SOFT_VRPSPD;
			case ProblemVariant::SOFT_VRPTW: return LINEAR_SOFT_VRPTW;
			case ProblemVariant::SOFT_VRPSPDTW: return LINEAR_SOFT_VRPSPDTW;
		}
		throw std::runtime_error("No linear solver exists for " + problemVariantName(variant));
	}

	std::unique_ptr<Split> makeReferenceSolver(ProblemVariant variant, Pb_Data* data)
	{
		if (variant == ProblemVariant::SOFT_CVRP)
			return std::unique_ptr<Split>(new Split_Bellman_Soft(data));
		if (variant == ProblemVariant::SOFT_VRPSPD)
			return std::unique_ptr<Split>(new Split_Bellman_Soft_VRPSPD(data));
		if (variant == ProblemVariant::SOFT_VRPTW)
			return std::unique_ptr<Split>(new Split_Bellman_Soft_VRPTW(data));
		if (variant == ProblemVariant::SOFT_VRPSPDTW)
			return std::unique_ptr<Split>(new Split_Bellman_Soft_VRPSPDTW(data));
		return std::unique_ptr<Split>(new Split_Bellman_VRPSPDTW(data));
	}

	std::unique_ptr<Split> makeLinearSolver(ProblemVariant variant, Pb_Data* data)
	{
		switch (variant)
		{
			case ProblemVariant::CVRP:
				return std::unique_ptr<Split>(new Split_Linear_CVRP(data));
			case ProblemVariant::VRPSPD:
				return std::unique_ptr<Split>(new Split_Linear_VRPSPD(data));
			case ProblemVariant::VRPTW:
				return std::unique_ptr<Split>(new Split_Linear_VRPTW(data));
			case ProblemVariant::VRPSPDTW:
				return std::unique_ptr<Split>(new Split_Linear_VRPSPDTW(data));
			case ProblemVariant::SOFT_CVRP:
				return std::unique_ptr<Split>(new Split_Linear_Soft(data));
			case ProblemVariant::SOFT_VRPSPD:
				return std::unique_ptr<Split>(new Split_Linear_Soft_VRPSPD(data));
			case ProblemVariant::SOFT_VRPTW:
				return std::unique_ptr<Split>(new Split_Linear_Soft_VRPTW(data));
			case ProblemVariant::SOFT_VRPSPDTW:
				return std::unique_ptr<Split>(new Split_Linear_Soft_VRPSPDTW(data));
		}
		throw std::runtime_error("No linear solver exists for " + problemVariantName(variant));
	}

	bool costsMatch(double referenceCost, double linearCost)
	{
		const double scale = std::max(1.0, std::max(std::fabs(referenceCost), std::fabs(linearCost)));
		// Vidal's original linear soft-CVRP dominance tests use a 1e-4 epsilon.
		return std::fabs(referenceCost - linearCost) <= 1.e-4 + 1.e-10 * scale;
	}

	void printPermutation(const BenchmarkInstance& instance, const std::vector<int>& permutation)
	{
		std::cerr << "PERMUTATION  :";
		for (std::vector<int>::const_iterator it = permutation.begin(); it != permutation.end(); ++it)
			std::cerr << ' ' << instance.customers[*it].id;
		std::cerr << '\n';
	}
}

int main(int argc, char* argv[])
{
	try
	{
		const CorrectnessOptions options = parseOptions(argc, argv);
		const BenchmarkInstance instance = loadBenchmarkInstance(options.instancePath);

		std::cout << "INSTANCE     : " << instance.name << '\n'
		          << "CUSTOMERS    : " << instance.customers.size() << '\n'
		          << "VARIANT      : " << problemVariantName(options.variant) << '\n'
		          << "PERMUTATIONS : " << options.numberOfPermutations << '\n'
		          << "RANDOM SEED  : " << PERMUTATION_SEED << '\n';
		if (isSoftVariant(options.variant))
			std::cout << "PENALTY LOAD : " << options.penaltyLoad << '\n';
		if (options.variant == ProblemVariant::SOFT_VRPTW ||
			options.variant == ProblemVariant::SOFT_VRPSPDTW)
			std::cout << "PENALTY WARP : " << options.penaltyWarp << '\n';

		std::vector<int> permutation(instance.customers.size());
		std::iota(permutation.begin(), permutation.end(), 0);
		std::mt19937 generator(PERMUTATION_SEED);

		for (int run = 1; run <= options.numberOfPermutations; ++run)
		{
			std::shuffle(permutation.begin(), permutation.end(), generator);

			Pb_Data referenceData = makePermutedData(instance, permutation, options.variant,
				referenceSolverType(options.variant), options.penaltyLoad, options.penaltyWarp);
			Pb_Data linearData = makePermutedData(instance, permutation, options.variant,
				linearSolverType(options.variant), options.penaltyLoad, options.penaltyWarp);

			std::unique_ptr<Split> reference = makeReferenceSolver(options.variant, &referenceData);
			std::unique_ptr<Split> linear = makeLinearSolver(options.variant, &linearData);
			reference->solve();
			linear->solve();

			if (!std::isfinite(referenceData.solutionCost) ||
				!std::isfinite(linearData.solutionCost) ||
				!costsMatch(referenceData.solutionCost, linearData.solutionCost))
			{
				std::cerr << std::setprecision(17)
				          << "FAIL         : costs differ on permutation " << run << '\n'
				          << "REFERENCE    : " << referenceData.solutionCost
				          << " (" << referenceData.solutionNbRoutes << " routes)\n"
				          << "LINEAR       : " << linearData.solutionCost
				          << " (" << linearData.solutionNbRoutes << " routes)\n";
				printPermutation(instance, permutation);
				return 1;
			}
		}

		std::cout << "RESULT       : PASS (all linear costs match Bellman)\n";
		return 0;
	}
	catch (const std::exception& error)
	{
		std::cerr << "ERROR        : " << error.what() << '\n';
	}
	catch (const std::string& error)
	{
		std::cerr << "ERROR        : " << error << '\n';
	}
	catch (...)
	{
		std::cerr << "ERROR        : unknown failure\n";
	}

	printUsage(std::cerr);
	return 2;
}
