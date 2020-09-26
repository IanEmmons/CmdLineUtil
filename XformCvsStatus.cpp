
//TODO:  Add unit tests

#include "XformCvsStatus.h"
#include "Exceptions.h"
#include "main.h"
#include "Utils.h"

#include <algorithm>
#include <boost/algorithm/string/trim.hpp>
#include <boost/format.hpp>
#include <cstdlib>
#include <iostream>
#include <regex>
#include <stdexcept>
#include <utility>

#if defined(_WIN32)
#	define WIN32_LEAN_AND_MEAN
#	include <Windows.h>
#endif

namespace b = ::boost;

using ::std::cin;
using ::std::cout;
using ::std::endl;
using ::std::getline;
using ::std::ostream;
using ::std::pair;
using ::std::regex;
using ::std::smatch;
using ::std::string;

static const char k_lastWorkingDirPattern[] = "^cvs (?:server|status): Examining (.*)$";
static const char k_newFilePattern[] = "^(\\?) (.*)$";
static const char k_fileStatusPattern[] = "^File: (?:no file )?(.*)[ \t]+Status: (.*)$";

#if !defined(CMDLINEUTIL_TEST_MODE)
int main(int argCount, const char*const*const argList)
{
	return commonMain<XformCvsStatus>(argCount, argList);
}
#endif

int XformCvsStatus::usage(ostream& strm, const string& progName, const char* pMsg)
{
	int exitCode = EXIT_SUCCESS;
	if (pMsg != nullptr && *pMsg != '\0')
	{
		exitCode = EXIT_FAILURE;
		strm << endl << pMsg << endl;
	}
	strm << "\n"
	"Usage:  " << progName << " [-n] [-u] [-l]\n"
	"\n"
	"Takes the output of the \"cvs status\" command and transforms it\n"
	"to a more succinct summary, with one file per line.  Options:\n"
	"\n"
	"   -n Suppresses new (unknown) files from the listing\n"
	"\n"
	"   -u Suppresses up-to-date files from the listing\n"
	"\n"
	"   -l Suppresses locally modified, locally added, and locally\n"
	"      removed files from the listing\n"
	<< endl;

	return exitCode;
}

XformCvsStatus::XformCvsStatus(::gsl::span<const char*const> args) :
	m_suppressNew(false),
	m_suppressUpToDate(false),
	m_suppressLocal(false),
	m_lastWorkingDir("."),
	m_map()
{
	for (auto pArg : args)
	{
		if (isIEqual(pArg, "-?") || isIEqual(pArg, "-h") || isIEqual(pArg, "-help"))
		{
			throw CmdLineError();
		}
		else if (isIEqual(pArg, "-n"))
		{
			m_suppressNew = true;
		}
		else if (isIEqual(pArg, "-u"))
		{
			m_suppressUpToDate = true;
		}
		else if (isIEqual(pArg, "-l"))
		{
			m_suppressLocal = true;
		}
		else
		{
			throw CmdLineError(b::format("Unrecognized option \"%1%\"") % pArg);
		}
	}
}

string XformCvsStatus::pathStringToConsoleString(const Path::string_type& str)
{
#if defined(_WIN32)
	int bufSize = ::WideCharToMultiByte(CP_OEMCP, 0, str.c_str(), -1, 0, 0, 0, 0);
	if (bufSize == 0 && ::GetLastError() != ERROR_SUCCESS)
	{
		throw WindowsError(b::format("WideCharToMultiByte failure, 1st call, error code %1%")
			% ::GetLastError());
	}
	bufSize += 2;
	::std::vector<char> buffer(bufSize);
	if (::WideCharToMultiByte(CP_OEMCP, 0, str.c_str(), -1, &(buffer[0]), bufSize, 0, 0) == 0
		&& ::GetLastError() != ERROR_SUCCESS)
	{
		throw WindowsError(b::format("WideCharToMultiByte failure, 2nd call, error code %1%")
			% ::GetLastError());
	}
	return &(buffer[0]);
#else
	return str;
#endif
}

int XformCvsStatus::run()
{
	regex lwdRegex(k_lastWorkingDirPattern);
	regex newRegex(k_newFilePattern);
	regex statusRegex(k_fileStatusPattern);

	while (cin)
	{
		string line;
		getline(cin, line);

		smatch match;
		if (regex_match(line, match, lwdRegex))
		{
			m_lastWorkingDir = string(match[1]);
		}
		else if (regex_match(line, match, newRegex))
		{
			insertInMap(match[2], match[1]);
		}
		else if (regex_match(line, match, statusRegex))
		{
			insertInMap(match[1], match[2]);
		}
	}

	for (auto const& mapEntry : m_map)
	{
		string st = mapEntry.second;
		if ((!m_suppressNew || st != "? ")
			&& (!m_suppressUpToDate || st != "  ")
			&& (!m_suppressLocal || (st != "M " && st != "A " && st != "D ")))
		{
			cout << st << " " << pathStringToConsoleString(mapEntry.first.native()) << endl;
		}
	}

	return EXIT_SUCCESS;
}

void XformCvsStatus::insertInMap(const string& rawFile, const string& rawStatus)
{
	Path file	 = buildPath(rawFile);
	string status = xlateStatus(rawStatus);
	pair<FileToStatusMap::iterator, bool> result = m_map.insert(
		make_pair(file, status));
	if (!result.second && result.first->second == "? " && status == "A ")
	{
		result.first->second = status;
	}
}

XformCvsStatus::Path XformCvsStatus::buildPath(const string& file) const
{
	string trFile = b::trim_copy(file);
	return (m_lastWorkingDir.empty() || m_lastWorkingDir == ".")
		? Path{trFile}
		: m_lastWorkingDir / trFile;
}

string XformCvsStatus::xlateStatus(const string& status)
{
	string trStatus = b::trim_copy(status);
	if (trStatus == "Locally Added")
	{
		return "A ";
	}
	else if (trStatus == "Locally Modified")
	{
		return "M ";
	}
	else if (trStatus == "Locally Removed")
	{
		return "D ";
	}
	else if (trStatus == "Up-to-date")
	{
		return "  ";
	}
	else if (trStatus == "?" || trStatus == "Unknown")
	{
		return "? ";
	}
	else if (trStatus == "Needs Checkout" || trStatus == "Needs Patch")
	{
		return " *";
	}
	else if (trStatus == "Needs Merge")
	{
		return "M*";
	}
	else if (trStatus == "File had conflicts on merge" || trStatus == "Unresolved Conflict")
	{
		return "C ";
	}
	else
	{
		return trStatus;
	}
}
