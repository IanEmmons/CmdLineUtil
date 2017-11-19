
#include "Random.h"
#include "main.h"

#include <boost/lexical_cast.hpp>
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <iostream>

namespace b = ::boost;

using ::std::cout;
using ::std::endl;
using ::std::ostream;

#if !defined(CMDLINEUTIL_TEST_MODE)
int main(int argCount, const char*const*const argList)
{
	return commonMain<Random>(argCount, argList);
}
#endif

int Random::usage(ostream& out, const char* pMsg)
{
	int exitCode = EXIT_SUCCESS;
	if (pMsg != nullptr && *pMsg != '\0')
	{
		exitCode = EXIT_FAILURE;
		out << endl << pMsg << endl;
	}
	out << "\n"
		"Usage:  random <lower-bound> <upper-bound> [ <count> ]\n"
		"\n"
		"Generates <count> uniformly distributed random integers\n"
		"between <lower-bound> and <upper-bound>.  The two bounds\n"
		"are required, and <count> defaults to one.\n"
		<< endl;

	return exitCode;
}

Random::Random(size_t argCount, const char*const*const ppArgList) :
	m_lowerBound(0),
	m_upperBound(0),
	m_count(0)
{
	if (argCount < 3)
	{
		throw CmdLineError("Too few arguments");
	}
	else if (argCount > 4)
	{
		throw CmdLineError("Too many arguments");
	}
	else
	{
		try
		{
			m_lowerBound = b::lexical_cast<int>(ppArgList[1]);
			m_upperBound = b::lexical_cast<int>(ppArgList[2]);
			if (argCount == 4)
			{
				m_count = b::lexical_cast<unsigned>(ppArgList[3]);
			}
		}
		catch(const b::bad_lexical_cast&)
		{
			throw CmdLineError("Invalid numeric format");
		}
	}

	if (m_count <= 0)
	{
		m_count = 1;
	}
}

int Random::run() const
{
	srand(static_cast<unsigned int>(time(nullptr)));
	for (unsigned i = 0; i < m_count; ++i)
	{
		cout << getRandomInteger(m_lowerBound, m_upperBound) << endl;
	}
	return EXIT_SUCCESS;
}

int Random::getRandomInteger(int low, int high)
{
	double r = static_cast<double>(rand()) / static_cast<double>(RAND_MAX);
	r *= static_cast<double>(high - low + 1);
	return low + static_cast<int>(floor(r));
}
