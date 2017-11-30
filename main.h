
#include "Exceptions.h"

#include <boost/algorithm/string/predicate.hpp>
#include <cstdlib>
#include <gsl/gsl>
#include <iostream>
#include <stdexcept>

static inline ::gsl::span<const char*const> makeArgsSpan(
	size_t argCount, const char*const*const argList)
{
	// Skip over the 0th element (program name):
	return ::gsl::make_span(argList + 1, argCount - 1);
}

template <typename TC>
static inline ::gsl::span<const char*const> makeArgsSpan(const TC& tc)
{
	return makeArgsSpan(tc.m_testArgsCount, tc.m_testArgs);
}

template <typename T>
int commonMain(size_t argCount, const char*const*const argList)
{
	using ::std::cout;
	using ::std::endl;

	int exitCode = EXIT_FAILURE;
	try
	{
		T program(makeArgsSpan(argCount, argList));
		exitCode = program.run();
	}
	catch (const CmdLineError& ex)
	{
		exitCode = T::usage(cout, ex.what());
	}
	catch (const ::std::exception& ex)
	{
		cout << endl
			<< "Exception of type " << typeid(ex).name() << endl
			<< "   " << ex.what() << endl
			<< endl;
	}
	return exitCode;
}

static inline bool isIEqual(const char* pStr1, const char* pStr2)
{
	return ::boost::algorithm::iequals(
		::gsl::ensure_z(pStr1), ::gsl::ensure_z(pStr2));
}
