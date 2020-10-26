
#include "Random.h"
#include "main.h"

#include <cmath>
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <random>
#include <stdexcept>
#include <string>

using ::std::cout;
using ::std::default_random_engine;
using ::std::endl;
using ::std::invalid_argument;
using ::std::ostream;
using ::std::out_of_range;
using ::std::string;
using ::std::uniform_int_distribution;

long fromCString(const char* pStr)
{
	string str{pStr};
	try
	{
		size_t endIndex;
		auto result = stol(str, &endIndex);
		if (endIndex != str.size())
		{
			throw invalid_argument(
				"Numeric argument with trailing non-numeric characters");
		}
		return result;
	}
	catch(const invalid_argument& ex)
	{
		string msg{"Invalid numeric format ('"};
		msg += str;
		msg += "'): ";
		msg += ex.what();
		throw CmdLineError(msg);
	}
	catch(const out_of_range& ex)
	{
		string msg{"Out-of-range number ('"};
		msg += str;
		msg += "')";
		throw CmdLineError(msg);
	}
}

#if !defined(CMDLINEUTIL_TEST_MODE)
int main(int argCount, const char*const*const argList)
{
	return commonMain<Random>(argCount, argList);
}
#endif

int Random::usage(ostream& out, const string& progName, const char* pMsg)
{
	int exitCode = EXIT_SUCCESS;
	if (pMsg != nullptr && *pMsg != '\0')
	{
		exitCode = EXIT_FAILURE;
		out << endl << pMsg << endl;
	}
	out << "\n"
		"Usage:  " << progName << " <lower-bound> <upper-bound> [ <count> ]\n"
		"\n"
		"Generates <count> uniformly distributed random integers\n"
		"between <lower-bound> and <upper-bound>.  The two bounds\n"
		"are required, and <count> defaults to one.\n"
		<< endl;

	return exitCode;
}

Random::Random(::gsl::span<const char*const> args) :
	m_lowerBound(0),
	m_upperBound(0),
	m_count(1)
{
	if (args.size() < 3)
	{
		throw CmdLineError("Too few arguments");
	}
	else if (args.size() > 4)
	{
		throw CmdLineError("Too many arguments");
	}
	else
	{
		m_lowerBound = fromCString(args[1]);
		m_upperBound = fromCString(args[2]);
		if (args.size() == 4)
		{
			m_count = fromCString(args[3]);
		}
	}

	if (m_count <= 0)
	{
		m_count = 1;
	}
}

int Random::run() const
{
	for (unsigned i = 0; i < m_count; ++i)
	{
		cout << getRandomInteger(m_lowerBound, m_upperBound) << endl;
	}
	return EXIT_SUCCESS;
}

long Random::getRandomInteger(long low, long high)
{
	default_random_engine generator;
	uniform_int_distribution<long> distribution(low, high);
	return distribution(generator);
}
