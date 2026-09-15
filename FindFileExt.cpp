
#include "FindFileExt.h"
#include "main.h"
#include "Utils.h"

#include <format>
#include <iostream>

using ::std::cout;
using ::std::endl;
using ::std::format;
using ::std::ostream;
using ::std::ranges::for_each;
using ::std::string;
using ::std::string_view;

#if !defined(CMDLINEUTIL_TEST_MODE)
int main(int argCount, const char*const*const argList)
{
	return commonMain<FindFileExt>(argCount, argList);
}
#endif

static constexpr string_view k_usageMsg = R"(
Usage:  {0} [-c] [-r] [-w] <dir1> <dir2> ...

Lists all file extensions found within the indicated directories.

Options:
   -c Includes counts in the output
   -r Search in sub-directories recursively
   -w Report extensions as wildcards
)";

int FindFileExt::usage(ostream& out, string_view progName, string_view msg)
{
	int exitCode = EXIT_SUCCESS;
	if (msg.size() > 0)
	{
		exitCode = EXIT_FAILURE;
		out << endl << msg << endl;
	}
	out << format(k_usageMsg, progName) << endl;

	return exitCode;
}

FindFileExt::FindFileExt(const ::std::span<const char*const> args) :
	m_includeCounts(false),
	m_outputAsWildcards(false),
	m_fileEnumerator(),
	m_extToCountMap(),
	m_noExtList()
{
	for (auto pArg : args)
	{
		if (isIEqual(pArg, "-?") || isIEqual(pArg, "-h") || isIEqual(pArg, "-help"))
		{
			throw CmdLineError();
		}
		else if (isIEqual(pArg, "-c"))
		{
			m_includeCounts = true;
		}
		else if (isIEqual(pArg, "-r"))
		{
			m_fileEnumerator.setRecursive();
		}
		else if (isIEqual(pArg, "-w"))
		{
			m_outputAsWildcards = true;
		}
		else
		{
			auto p = Path{pArg};
			if (!exists(p))
			{
				throw CmdLineError(format("'{0}' does not exist", pArg));
			}
			else if (!is_directory(p))
			{
				throw CmdLineError(format("'{0}' is not a directory", pArg));
			}
			p /= "*";
			m_fileEnumerator.insert(p);
		}
	}

	if (m_fileEnumerator.numFileSpecs() <= 0)
	{
		throw CmdLineError("No directories specified");
	}
}

int FindFileExt::run()
{
	countFiles();

	cout << endl;
	for_each(m_extToCountMap,
		[this](const StrToCountMap::value_type& extToCountMapping)
		{
			reportExtension(extToCountMapping);
		});
	cout << endl;

	return EXIT_SUCCESS;
}

void FindFileExt::countFiles()
{
	m_fileEnumerator.enumerateFiles([this](const Path& file)
	{
		string ext = file.extension().generic_string();
		auto result = m_extToCountMap.insert(make_pair(ext, 0));
		result.first->second += 1;
		if (ext.empty())
		{
			m_noExtList.push_back(file);
		}
	});
}

void FindFileExt::reportExtension(const StrToCountMap::value_type& extToCountMapping)
{
	const bool isNoExtensionEntry = extToCountMapping.first.empty();
	string ext(isNoExtensionEntry
		? "<no extension>"
		: extToCountMapping.first);
	if (m_includeCounts && m_outputAsWildcards)
	{
		if (!isNoExtensionEntry)
		{
			cout << format("*{0} -- {1}", ext, extToCountMapping.second) << endl;
		}
	}
	else if (m_includeCounts)
	{
		cout << format("{0} -- {1}", ext, extToCountMapping.second) << endl;
		if (isNoExtensionEntry)
		{
			for (const auto& noExtPath : m_noExtList)
			{
				cout << format("   {0}", noExtPath.generic_string()) << endl;
			}
		}
	}
	else if (m_outputAsWildcards)
	{
		if (!isNoExtensionEntry)
		{
			cout << format("*{0}", ext) << endl;
		}
	}
	else
	{
		cout << ext << endl;
	}
}
