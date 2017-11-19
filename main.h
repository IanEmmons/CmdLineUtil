
#include "Exceptions.h"

#include <boost/algorithm/string/predicate.hpp>
#include <cstdlib>
#include <gsl/gsl>
#include <iostream>
#include <stdexcept>

template <typename T>
int commonMain(size_t argCount, const char*const*const argList)
{
	using ::std::cout;
	using ::std::endl;

	int exitCode = EXIT_FAILURE;
	try
	{
		//T program(argCount, argList);
		T program(::gsl::make_span(argList, argCount));
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
