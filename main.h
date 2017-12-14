
#if !defined(MAIN_H_INCLUDED)
#define MAIN_H_INCLUDED

#include "Exceptions.h"

#include <boost/filesystem.hpp>
#include <cstdlib>
#include <gsl/gsl>
#include <iostream>
#include <stdexcept>

template <typename T>
int commonMain(size_t argCount, const char*const*const argList)
{
	using ::boost::filesystem::path;
	using ::gsl::make_span;
	using ::std::cout;
	using ::std::endl;

	auto exitCode{EXIT_FAILURE};
	try
	{
		// Pass the argument list, not including the program name (0th element):
		T program{make_span(argList + 1, argCount - 1)};
		exitCode = program.run();
	}
	catch (const CmdLineError& ex)
	{
		exitCode = T::usage(cout, path{argList[0]}.stem().generic_string(), ex.what());
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

#endif // MAIN_H_INCLUDED
