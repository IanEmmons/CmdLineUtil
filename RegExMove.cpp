
#include "RegExMove.h"
#include "main.h"

#include <boost/algorithm/string/predicate.hpp>
#include <boost/format.hpp>
#include <vector>

namespace b = ::boost;
namespace balg = ::boost::algorithm;
namespace fs = ::std::filesystem;

using ::std::cout;
using ::std::endl;
using ::std::ostream;
using ::std::regex;
using ::std::regex_error;
using ::std::string;
using ::std::string_view;
using ::std::vector;

#if !defined(CMDLINEUTIL_TEST_MODE)
int main(int argCount, const char*const*const argList)
{
	return commonMain<RegExMove>(argCount, argList);
}
#endif

int RegExMove::usage(ostream& out, string_view progName, const char* pMsg)
{
	int exitCode = EXIT_SUCCESS;
	if (pMsg != nullptr && *pMsg != '\0')
	{
		exitCode = EXIT_FAILURE;
		out << endl << pMsg << endl;
	}
	out << "\n"
	"Usage:  " << progName << " [-i] [-d] [-dd] [-r] [-v] [-y] <rootdir> <regex> <replacement>\n"
	"\n"
	"Renames a collection of files using a regular expression search-and-replace.\n"
	"Uses ECMAScript regular expression syntax.\n"
	"\n"
	"   -i Perform a case-insensitive search.\n"
	"\n"
	"   -d Rename directories as well as files.\n"
	"\n"
	"   -dd Rename only directories.\n"
	"\n"
	"   -r Search recursively within subdirectories of <rootdir> for\n"
	"      matching files.\n"
	"\n"
	"   -v Verbose output.\n"
	"\n"
	"   -y (Potentially Dangerous) Causes renamed files to overwrite existing\n"
	"      files of the same name without prompting.\n"
	"\n"
	"   <rootdir> The directory to search for files to rename.\n"
	"\n"
	"   <regex> The regular expression that selects the files whose name is\n"
	"      to be changed.\n"
	"\n"
	"   <replacement> The substitution expression that gives the files their\n"
	"      new names.\n"
	<< endl;

	return exitCode;
}

RegExMove::RegExMove(::std::span<const char*const> args) :
	m_caseSensitive(true),
	m_renameDirectories(false),
	m_renameFiles(true),
	m_recursiveSearch(false),
	m_verboseOutput(false),
	m_allowOverwriteOnNameCollision(false),
	m_rootDir(),
	m_patternStr(),
	m_pattern(),
	m_replacement()
{
	vector<string> posArgs;	// positional arguments
	for (auto pArg : args)
	{
		if (isIEqual(pArg, "-?") || isIEqual(pArg, "-h") || isIEqual(pArg, "-help"))
		{
			throw CmdLineError();
		}
		else if (isIEqual(pArg, "-i"))
		{
			m_caseSensitive = false;
		}
		else if (isIEqual(pArg, "-d"))
		{
			m_renameDirectories = true;
		}
		else if (isIEqual(pArg, "-dd"))
		{
			m_renameDirectories = true;
			m_renameFiles = false;
		}
		else if (isIEqual(pArg, "-r"))
		{
			m_recursiveSearch = true;
		}
		else if (isIEqual(pArg, "-v"))
		{
			m_verboseOutput = true;
		}
		else if (isIEqual(pArg, "-y"))
		{
			m_allowOverwriteOnNameCollision = true;
		}
		else
		{
			posArgs.push_back(pArg);
		}
	}

	if (posArgs.size() < 3)
	{
		throw CmdLineError("All of the arguments <rootdir>, <regex>, and "
			"<replacement> are required");
	}
	else if (posArgs.size() > 3)
	{
		throw CmdLineError("Only three positional arguments are accepted:  "
			"<rootdir>, <regex>, and <replacement>");
	}

	m_rootDir = posArgs[0];
	if (!exists(m_rootDir))
	{
		throw CmdLineError(b::format("The directory \"%1%\" does not exist")
			% posArgs[0]);
	}
	else if (!is_directory(m_rootDir))
	{
		throw CmdLineError(b::format("\"%1%\" is not a directory") % posArgs[0]);
	}

	try
	{
		m_patternStr = posArgs[1];
		if (!balg::starts_with(m_patternStr, "^"))
		{
			m_patternStr = '^' + m_patternStr;
		}
		if (!balg::ends_with(m_patternStr, "$"))
		{
			m_patternStr += '$';
		}
		regex::flag_type flags = m_caseSensitive
			? regex::ECMAScript
			: regex::ECMAScript | regex::icase;
		m_pattern.assign(m_patternStr, flags);
	}
	catch (regex_error const& ex)
	{
		throw CmdLineError(b::format("\"%1%\" is not a valid regular expression (%2%)")
			% posArgs[1] % ex.what());
	}

	m_replacement = posArgs[2];
}

template <typename DirIter>
void RegExMove::processDirectoryEntries() const
{
	DirIter end;
	for (DirIter it(m_rootDir); it != end; ++it)
	{
		processDirectoryEntry(*it);
	}
}

int RegExMove::run() const
{
	if (m_recursiveSearch)
	{
		processDirectoryEntries<fs::directory_iterator>();
	}
	else
	{
		processDirectoryEntries<fs::recursive_directory_iterator>();
	}
	return EXIT_SUCCESS;
}

void RegExMove::processDirectoryEntry(DirEntry const& dirEntry) const
{
	switch (dirEntry.status().type())
	{
	case fs::file_type::directory:
		if (m_renameDirectories)
		{
			renamePath(dirEntry.path());
		}
		break;

	case fs::file_type::regular:
		if (m_renameFiles)
		{
			renamePath(dirEntry.path());
		}
		break;

	default:
		// Do nothing
		break;
	}
}

void RegExMove::renamePath(Path const& p) const
{
	string lastSegment = p.filename().string();
	if (regex_match(lastSegment, m_pattern))
	{
		string result = regex_replace(lastSegment, m_pattern, m_replacement);
		Path newPath(p.parent_path());
		newPath /= result;
		if (!exists(newPath))
		{
			rename(p, newPath);
			if (m_verboseOutput)
			{
				cout << b::format("\"%1%\" --> \"%2%\"") % p % result << endl;
			}
		}
		else if (m_allowOverwriteOnNameCollision)
		{
			rename(p, newPath);
			cout << b::format("\"%1%\" overwrote \"%2%\"") % p % result << endl;
		}
		else
		{
			cout << b::format("\"%1%\" skipped -- \"%2%\" exists") % p % result << endl;
		}
	}
}
