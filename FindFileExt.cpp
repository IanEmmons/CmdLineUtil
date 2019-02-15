
#include "FindFileExt.h"
#include "main.h"
#include "Utils.h"

#include <boost/format.hpp>
#include <iostream>

namespace b = ::boost;

using ::std::cout;
using ::std::endl;
using ::std::ostream;
using ::std::string;

#if !defined(CMDLINEUTIL_TEST_MODE)
int main(int argCount, const char*const*const argList)
{
	return commonMain<FindFileExt>(argCount, argList);
}
#endif

int FindFileExt::usage(ostream& out, const string& progName, const char* pMsg)
{
	int exitCode = EXIT_SUCCESS;
	if (pMsg != nullptr && *pMsg != '\0')
	{
		exitCode = EXIT_FAILURE;
		out << endl << pMsg << endl;
	}
	out << "\n"
		"Usage:  " << progName << " [-c] [-r] [-w] <dir1> <dir2> ...\n"
		"\n"
		"Compiles a list of all file extensions found within the indicated\n"
		"directories.\n"
		"\n"
		"Options:\n"
		"\n"
		"   -c Includes counts in the output\n"
		"\n"
		"   -r Search in sub-directories recursively\n"
		"\n"
		"   -w Report extensions as wildcards\n"
		<< endl;

	return exitCode;
}

FindFileExt::FindFileExt(const ::gsl::span<const char*const> args) :
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
				throw CmdLineError(b::format("%1% does not exist") % pArg);
			}
			else if (!is_directory(p))
			{
				throw CmdLineError(b::format("%1% is not a directory") % pArg);
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
	b::for_each(m_extToCountMap,
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
		string ext = extension(file);
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
			cout << b::format("*%1% -- %2%\n") % ext % extToCountMapping.second;
		}
	}
	else if (m_includeCounts)
	{
		cout << b::format("%1% -- %2%\n") % ext % extToCountMapping.second;
		if (isNoExtensionEntry)
		{
			for (const auto& noExtPath : m_noExtList)
			{
				cout << b::format("   %1%\n") % noExtPath;
			}
		}
	}
	else if (m_outputAsWildcards)
	{
		if (!isNoExtensionEntry)
		{
			cout << b::format("*%1%\n") % ext;
		}
	}
	else
	{
		cout << b::format("%1%\n") % ext;
	}
}
