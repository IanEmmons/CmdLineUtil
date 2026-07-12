
#include "RegExMove.h"
#include "main.h"

#include <boost/algorithm/string/predicate.hpp>
#include <format>
#include <vector>

namespace balg = ::boost::algorithm;
namespace fs = ::std::filesystem;

using ::std::cout;
using ::std::endl;
using ::std::format;
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

static constexpr string_view k_usageMsg = R"(
Usage:  {0} [-i] [-d] [-dd] [-r] [-v] [-y] <rootdir> <regex> <replacement>

Renames a collection of files using a regular expression search-and-replace.
Uses ECMAScript regular expression syntax. Options:
   -i  Perform a case-insensitive search.
   -d  Rename directories as well as files.
   -dd Rename only directories.
   -r  Search recursively in subdirectories of <rootdir> for matching files.
   -v  Verbose output.
   -y  (Potentially Dangerous) Causes renamed files to overwrite existing
       files of the same name without prompting.
   <rootdir> The directory to search for files to rename.
   <regex> The regular expression that selects the files whose name is
       to be changed.
   <replacement> The substitution expression that gives the files their
       new names.
)";

int RegExMove::usage(ostream& out, string_view progName, string_view msg)
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
		throw CmdLineError(format("The directory '{0}' does not exist", posArgs[0]));
	}
	else if (!is_directory(m_rootDir))
	{
		throw CmdLineError(format("'{0}' is not a directory", posArgs[0]));
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
		throw CmdLineError(format("'{0}' is not a valid regular expression ({1})",
			posArgs[1], ex.what()));
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
		processDirectoryEntries<fs::recursive_directory_iterator>();
	}
	else
	{
		processDirectoryEntries<fs::directory_iterator>();
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
				cout << format("'{0}' --> '{1}'", p.generic_string(), result) << endl;
			}
		}
		else if (m_allowOverwriteOnNameCollision)
		{
			rename(p, newPath);
			cout << format("'{0}' overwrote '{1}'", p.generic_string(), result) << endl;
		}
		else
		{
			cout << format("'{0}' skipped -- '{1}' exists", p.generic_string(), result) << endl;
		}
	}
}
