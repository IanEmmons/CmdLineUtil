
#if !defined(MAIN_H_INCLUDED)
#define MAIN_H_INCLUDED

#include "Exceptions.h"

#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <span>
#include <stdexcept>
#include <string_view>

template <typename T>
int commonMain(size_t argCount, const char*const*const argList)
{
	using ::std::cout;
	using ::std::endl;
	using ::std::filesystem::path;
	using ::std::span;
	using ::std::string_view;

	auto exitCode{EXIT_FAILURE};
	try
	{
		// Pass the argument list, not including the program name (0th element):
		T program{span{argList + 1, argCount - 1}};
		exitCode = program.run();
	}
	catch (const CmdLineError& ex)
	{
		auto progName = path{argList[0]}.stem().generic_string();
		auto msg = (ex.what() == nullptr)
			? string_view{}
			: string_view{ex.what()};
		exitCode = T::usage(cout, progName, msg);
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
