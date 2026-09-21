#include "BenchmarkInstanceReader.h"

#include <algorithm>
#include <cctype>
#include <cmath>
#include <fstream>
#include <limits>
#include <sstream>
#include <stdexcept>

namespace
{
	std::string trim(const std::string& value)
	{
		std::string::size_type first = 0;
		while (first < value.size() && std::isspace(static_cast<unsigned char>(value[first])))
			++first;

		std::string::size_type last = value.size();
		while (last > first && std::isspace(static_cast<unsigned char>(value[last - 1])))
			--last;

		return value.substr(first, last - first);
	}

	std::string upperAndNormalize(std::string value)
	{
		for (std::string::size_type i = 0; i < value.size(); ++i)
		{
			if (value[i] == '-')
				value[i] = '_';
			else
				value[i] = static_cast<char>(std::toupper(static_cast<unsigned char>(value[i])));
		}
		return value;
	}

	std::vector<std::string> readLines(const std::string& path)
	{
		std::ifstream input(path.c_str());
		if (!input)
			throw std::runtime_error("Cannot open benchmark instance: " + path);

		std::vector<std::string> lines;
		std::string line;
		while (std::getline(input, line))
			lines.push_back(trim(line));

		if (lines.empty())
			throw std::runtime_error("Benchmark instance is empty: " + path);
		return lines;
	}

	bool parseNumericLine(const std::string& line, std::vector<double>& values)
	{
		values.clear();
		std::istringstream stream(line);
		double value;
		while (stream >> value)
			values.push_back(value);

		stream.clear();
		stream >> std::ws;
		return !values.empty() && stream.eof();
	}

	std::string metadataValue(const std::vector<std::string>& lines, const std::string& key)
	{
		for (std::vector<std::string>::const_iterator it = lines.begin(); it != lines.end(); ++it)
		{
			const std::string::size_type colon = it->find(':');
			if (colon != std::string::npos && trim(it->substr(0, colon)) == key)
				return trim(it->substr(colon + 1));
		}
		throw std::runtime_error("Missing " + key + " field");
	}

	int sectionIndex(const std::vector<std::string>& lines, const std::string& section)
	{
		for (std::vector<std::string>::size_type i = 0; i < lines.size(); ++i)
			if (lines[i] == section)
				return static_cast<int>(i);
		return -1;
	}

	std::vector<std::vector<double> > euclideanDistances(const std::vector<double>& x,
		const std::vector<double>& y)
	{
		const std::size_t count = x.size();
		std::vector<std::vector<double> > distances(count, std::vector<double>(count, 0.0));
		for (std::size_t i = 0; i < count; ++i)
			for (std::size_t j = 0; j < count; ++j)
				distances[i][j] = std::hypot(x[i] - x[j], y[i] - y[j]);
		return distances;
	}

	BenchmarkInstance parseTabularInstance(const std::string& path,
		const std::vector<std::string>& lines)
	{
		int customerHeader = -1;
		for (std::vector<std::string>::size_type i = 0; i < lines.size(); ++i)
		{
			if (lines[i].find("CUST NO.") != std::string::npos)
			{
				customerHeader = static_cast<int>(i);
				break;
			}
		}
		if (customerHeader < 0)
			throw std::runtime_error("Cannot find the customer table in " + path);

		std::vector<double> vehicleLine;
		for (int i = 0; i < customerHeader; ++i)
		{
			std::vector<double> candidate;
			if (parseNumericLine(lines[i], candidate) && (candidate.size() == 2 || candidate.size() == 3))
				vehicleLine = candidate;
		}
		if (vehicleLine.empty())
			throw std::runtime_error("Cannot find vehicle/capacity data in " + path);

		std::vector<std::vector<double> > rows;
		for (std::vector<std::string>::size_type i = customerHeader + 1; i < lines.size(); ++i)
		{
			if (lines[i].empty())
				continue;
			std::vector<double> row;
			if (!parseNumericLine(lines[i], row))
				throw std::runtime_error("Invalid customer row in " + path + ": " + lines[i]);
			if (row.size() != 7 && row.size() != 8)
				throw std::runtime_error("Unexpected customer column count in " + path);
			if (!rows.empty() && row.size() != rows.front().size())
				throw std::runtime_error("Inconsistent customer rows in " + path);
			rows.push_back(row);
		}
		if (rows.size() < 2)
			throw std::runtime_error("No customers found in " + path);

		const bool hasPickups = rows.front().size() == 8;
		const int declaredCustomers = vehicleLine.size() == 3 ? static_cast<int>(vehicleLine[0]) : -1;
		if (declaredCustomers >= 0 && declaredCustomers != static_cast<int>(rows.size()) - 1)
			throw std::runtime_error("Declared customer count does not match the customer table in " + path);

		BenchmarkInstance result;
		result.name = lines.front();
		result.sourcePath = path;
		result.numberOfVehicles = static_cast<int>(vehicleLine[vehicleLine.size() - 2]);
		result.capacity = vehicleLine.back();
		result.hasPickupData = hasPickups;
		result.hasTimeWindowData = true;

		std::vector<double> x(rows.size());
		std::vector<double> y(rows.size());
		for (std::size_t i = 0; i < rows.size(); ++i)
		{
			x[i] = rows[i][1];
			y[i] = rows[i][2];
		}
		result.distances = euclideanDistances(x, y);

		const std::vector<double>& depot = rows.front();
		result.depotClosingTime = depot[hasPickups ? 6 : 5];
		for (std::size_t i = 1; i < rows.size(); ++i)
		{
			const std::vector<double>& row = rows[i];
			BenchmarkCustomer customer;
			customer.id = static_cast<int>(row[0]);
			customer.sourceIndex = static_cast<int>(i);
			customer.delivery = row[3];
			customer.pickup = hasPickups ? row[4] : 0.0;
			customer.twOpen = row[hasPickups ? 5 : 4];
			customer.twClose = row[hasPickups ? 6 : 5];
			customer.serviceTime = row[hasPickups ? 7 : 6];
			result.customers.push_back(customer);
		}
		return result;
	}

	BenchmarkInstance parseTsplibSpdInstance(const std::string& path,
		const std::vector<std::string>& lines)
	{
		const int dimension = std::stoi(metadataValue(lines, "DIMENSION"));
		if (dimension < 2)
			throw std::runtime_error("Invalid DIMENSION in " + path);

		BenchmarkInstance result;
		result.name = metadataValue(lines, "NAME");
		result.sourcePath = path;
		result.capacity = std::stod(metadataValue(lines, "CAPACITY"));
		result.numberOfVehicles = dimension - 1;
		result.hasPickupData = true;
		result.hasTimeWindowData = false;

		const int pickupSection = sectionIndex(lines, "PICKUP_AND_DELIVERY_SECTION");
		if (pickupSection < 0)
			throw std::runtime_error("Missing PICKUP_AND_DELIVERY_SECTION in " + path);

		std::vector<BenchmarkCustomer> nodes(dimension);
		std::vector<bool> seen(dimension, false);
		for (std::vector<std::string>::size_type i = pickupSection + 1; i < lines.size(); ++i)
		{
			if (lines[i].empty() || lines[i] == "EOF")
				continue;
			std::vector<double> row;
			if (!parseNumericLine(lines[i], row) || row.size() != 7)
				throw std::runtime_error("Invalid pickup/delivery row in " + path + ": " + lines[i]);
			const int id = static_cast<int>(row[0]);
			if (id < 1 || id > dimension || seen[id - 1])
				throw std::runtime_error("Invalid or duplicate node id in " + path);

			BenchmarkCustomer customer;
			customer.id = id;
			customer.sourceIndex = id - 1;
			customer.delivery = row[6];
			customer.pickup = row[5];
			customer.twOpen = row[2];
			customer.twClose = row[3];
			customer.serviceTime = row[4];
			nodes[id - 1] = customer;
			seen[id - 1] = true;
		}
		if (std::find(seen.begin(), seen.end(), false) != seen.end())
			throw std::runtime_error("PICKUP_AND_DELIVERY_SECTION has the wrong size in " + path);

		result.depotClosingTime = nodes[0].twClose;
		result.customers.assign(nodes.begin() + 1, nodes.end());

		const std::string edgeType = metadataValue(lines, "EDGE_WEIGHT_TYPE");
		if (edgeType == "EUC_2D")
		{
			const int coordinateSection = sectionIndex(lines, "NODE_COORD_SECTION");
			if (coordinateSection < 0)
				throw std::runtime_error("Missing NODE_COORD_SECTION in " + path);
			std::vector<double> x(dimension, 0.0);
			std::vector<double> y(dimension, 0.0);
			std::vector<bool> coordinateSeen(dimension, false);
			for (int i = coordinateSection + 1; i < pickupSection; ++i)
			{
				if (lines[i].empty())
					continue;
				std::vector<double> row;
				if (!parseNumericLine(lines[i], row) || row.size() != 3)
					throw std::runtime_error("Invalid coordinate row in " + path + ": " + lines[i]);
				const int id = static_cast<int>(row[0]);
				if (id < 1 || id > dimension || coordinateSeen[id - 1])
					throw std::runtime_error("Invalid or duplicate coordinate id in " + path);
				x[id - 1] = row[1];
				y[id - 1] = row[2];
				coordinateSeen[id - 1] = true;
			}
			if (std::find(coordinateSeen.begin(), coordinateSeen.end(), false) != coordinateSeen.end())
				throw std::runtime_error("NODE_COORD_SECTION has the wrong size in " + path);
			result.distances = euclideanDistances(x, y);
		}
		else if (edgeType == "EXPLICIT")
		{
			const int edgeSection = sectionIndex(lines, "EDGE_WEIGHT_SECTION");
			if (edgeSection < 0)
				throw std::runtime_error("Missing EDGE_WEIGHT_SECTION in " + path);
			std::vector<double> weights;
			for (int i = edgeSection + 1; i < pickupSection; ++i)
			{
				if (lines[i].empty())
					continue;
				std::vector<double> row;
				if (!parseNumericLine(lines[i], row))
					throw std::runtime_error("Invalid edge-weight row in " + path + ": " + lines[i]);
				weights.insert(weights.end(), row.begin(), row.end());
			}
			if (weights.size() != static_cast<std::size_t>(dimension) * static_cast<std::size_t>(dimension))
				throw std::runtime_error("EDGE_WEIGHT_SECTION has the wrong size in " + path);
			result.distances.assign(dimension, std::vector<double>(dimension, 0.0));
			for (int i = 0; i < dimension; ++i)
				for (int j = 0; j < dimension; ++j)
					result.distances[i][j] = weights[static_cast<std::size_t>(i) * dimension + j];
		}
		else
			throw std::runtime_error("Unsupported EDGE_WEIGHT_TYPE in " + path + ": " + edgeType);

		return result;
	}
}

ProblemVariant parseProblemVariant(const std::string& value)
{
	const std::string normalized = upperAndNormalize(value);
	if (normalized == "CVRP") return ProblemVariant::CVRP;
	if (normalized == "VRPSPD") return ProblemVariant::VRPSPD;
	if (normalized == "VRPTW") return ProblemVariant::VRPTW;
	if (normalized == "VRPSPDTW") return ProblemVariant::VRPSPDTW;
	if (normalized == "SOFT_CVRP") return ProblemVariant::SOFT_CVRP;
	if (normalized == "SOFT_VRPSPD") return ProblemVariant::SOFT_VRPSPD;
	if (normalized == "SOFT_VRPTW") return ProblemVariant::SOFT_VRPTW;
	if (normalized == "SOFT_VRPSPDTW") return ProblemVariant::SOFT_VRPSPDTW;
	throw std::runtime_error("Unrecognized variant: " + value);
}

std::string problemVariantName(ProblemVariant variant)
{
	switch (variant)
	{
		case ProblemVariant::CVRP: return "CVRP";
		case ProblemVariant::VRPSPD: return "VRPSPD";
		case ProblemVariant::VRPTW: return "VRPTW";
		case ProblemVariant::VRPSPDTW: return "VRPSPDTW";
		case ProblemVariant::SOFT_CVRP: return "Soft_CVRP";
		case ProblemVariant::SOFT_VRPSPD: return "Soft_VRPSPD";
		case ProblemVariant::SOFT_VRPTW: return "Soft_VRPTW";
		case ProblemVariant::SOFT_VRPSPDTW: return "Soft_VRPSPDTW";
	}
	throw std::runtime_error("Unknown problem variant");
}

bool isSoftVariant(ProblemVariant variant)
{
	return variant == ProblemVariant::SOFT_CVRP ||
		variant == ProblemVariant::SOFT_VRPSPD ||
		variant == ProblemVariant::SOFT_VRPTW ||
		variant == ProblemVariant::SOFT_VRPSPDTW;
}

bool usesPickupData(ProblemVariant variant)
{
	return variant == ProblemVariant::VRPSPD ||
		variant == ProblemVariant::VRPSPDTW ||
		variant == ProblemVariant::SOFT_VRPSPD ||
		variant == ProblemVariant::SOFT_VRPSPDTW;
}

bool usesTimeWindowData(ProblemVariant variant)
{
	return variant == ProblemVariant::VRPTW ||
		variant == ProblemVariant::VRPSPDTW ||
		variant == ProblemVariant::SOFT_VRPTW ||
		variant == ProblemVariant::SOFT_VRPSPDTW;
}

BenchmarkInstance loadBenchmarkInstance(const std::string& path)
{
	const std::vector<std::string> lines = readLines(path);
	if (sectionIndex(lines, "PICKUP_AND_DELIVERY_SECTION") >= 0)
		return parseTsplibSpdInstance(path, lines);
	return parseTabularInstance(path, lines);
}

Pb_Data makePermutedData(const BenchmarkInstance& instance,
	const std::vector<int>& permutation, ProblemVariant variant,
	SolverType solverType, double penaltyLoad, double penaltyWarp)
{
	if (permutation.size() != instance.customers.size())
		throw std::runtime_error("Permutation size does not match the instance");
	if (usesPickupData(variant) && !instance.hasPickupData)
		throw std::runtime_error(problemVariantName(variant) + " requires pickup data, but the instance has none");
	if (usesTimeWindowData(variant) && !instance.hasTimeWindowData)
		throw std::runtime_error(problemVariantName(variant) + " requires time-window data, but the instance has none");

	std::vector<bool> seen(instance.customers.size(), false);
	for (std::vector<int>::const_iterator it = permutation.begin(); it != permutation.end(); ++it)
	{
		if (*it < 0 || *it >= static_cast<int>(instance.customers.size()) || seen[*it])
			throw std::runtime_error("The generated customer order is not a permutation");
		seen[*it] = true;
	}

	const bool withPickups = usesPickupData(variant);
	const bool withWindows = usesTimeWindowData(variant);
	const double noTimeLimit = 1.e30;
	std::vector<Client> ordered(instance.customers.size() + 1);
	ordered[0].index = 0;
	ordered[0].demand = 0.0;
	ordered[0].pickup = 0.0;
	ordered[0].dreturn = 0.0;
	ordered[0].tw_open = 0.0;
	ordered[0].tw_close = withWindows ? instance.depotClosingTime : noTimeLimit;
	ordered[0].service_time = 0.0;

	for (std::size_t position = 0; position < permutation.size(); ++position)
	{
		const BenchmarkCustomer& source = instance.customers[permutation[position]];
		Client client;
		client.index = source.id;
		client.demand = source.delivery;
		client.pickup = withPickups ? source.pickup : 0.0;
		client.dreturn = instance.distances[0][source.sourceIndex];
		client.tw_open = withWindows ? source.twOpen : 0.0;
		client.tw_close = withWindows ? source.twClose : noTimeLimit;
		client.service_time = withWindows ? source.serviceTime : 0.0;
		if (position + 1 < permutation.size())
		{
			const BenchmarkCustomer& next = instance.customers[permutation[position + 1]];
			client.dnext = instance.distances[source.sourceIndex][next.sourceIndex];
		}
		else
			client.dnext = -1.0;
		ordered[position + 1] = client;
	}
	ordered[0].dnext = ordered[1].dreturn;

	return Pb_Data(instance.sourcePath, solverType, penaltyLoad, penaltyWarp,
		instance.capacity, instance.numberOfVehicles,
		withWindows ? instance.depotClosingTime : noTimeLimit, ordered);
}
