
#if !defined(MAIN_H_INCLUDED)
#define MAIN_H_INCLUDED

#include "Exceptions.h"

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
		// Skip over the 0th element (program name):
		T program(::gsl::make_span(argList + 1, argCount - 1));
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

#endif // MAIN_H_INCLUDED
