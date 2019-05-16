
#include "IndentClassifier.h"
#include "main.h"

#include <boost/filesystem/fstream.hpp>
#include <boost/format.hpp>
#include <iostream>
#include <regex>
#include <stdexcept>

namespace b = ::boost;
namespace bfs = ::boost::filesystem;

using ::std::cout;
using ::std::endl;
using ::std::istream;
using ::std::invalid_argument;
using ::std::ostream;
using ::std::regex;
using ::std::string;

static const regex k_spacePattern{"^ +([^ \\t].*)?$"};
static const regex k_tabPattern{"^\\t+([^ \\t].*)?$"};
static const regex k_javadocTabPattern{"^\\t+ \\*.*$"};
static const regex k_javadocLeftPattern{"^ \\*.*$"};
static const regex k_indeterminatePattern{"^([^ \\t].*)?$"};

static string getNextLine(istream& in)
{
	string line;
	getline(in, line);
	return line;
}

size_t get(const LineTypeCounts& lineTypeCounts, IndentType indentType)
{
	auto it{lineTypeCounts.find(indentType)};
	return (it == end(lineTypeCounts))
		? 0
		: it->second;
}

#if !defined(CMDLINEUTIL_TEST_MODE)
int main(int argCount, const char*const*const argList)
{
	return commonMain<IndentClassifier>(argCount, argList);
}
#endif

int IndentClassifier::usage(ostream& out, const string& progName, const char* pMsg)
{
	int exitCode = EXIT_SUCCESS;
	if (pMsg != nullptr && *pMsg != '\0')
	{
		exitCode = EXIT_FAILURE;
		out << endl << pMsg << endl;
	}
	out << "\n"
		"Usage:  " << progName << " [-r] <file1> <file2> ...\n"
		"\n"
		"Lists the indent type of each file, in the following format:\n"
		"\n"
		"   X <file-path>\n"
		"\n"
		"where X is:\n"
		"   * '" << indicatorLetter(IndentType::space) << "' for spaces,\n"
		"   * '" << indicatorLetter(IndentType::tab) << "' for tabs,\n"
		"   * '" << indicatorLetter(IndentType::javadocTab) << "' for tabs and tab-indented JavaDoc comment lines,\n"
		"   * '" << indicatorLetter(IndentType::mixed) << "' for mixed, or\n"
		"   * '" << indicatorLetter(IndentType::indeterminate) << "' for indeterminate (meaning no lines are indented).\n"
		"\n"
		"Options:\n"
		"\n"
		"   -r Search for files in sub-directories recursively\n"
		<< endl;

	return exitCode;
}

IndentClassifier::IndentClassifier(const ::gsl::span<const char*const> args) :
	m_fileEnumerator()
{
	for (auto pArg : args)
	{
		if (isIEqual(pArg, "-?") || isIEqual(pArg, "-h") || isIEqual(pArg, "-help"))
		{
			throw CmdLineError();
		}
		else if (isIEqual(pArg, "-r"))
		{
			m_fileEnumerator.setRecursive();
		}
		else
		{
			m_fileEnumerator.insert(pArg);
		}
	}

	if (m_fileEnumerator.numFileSpecs() <= 0)
	{
		throw CmdLineError("No files specified");
	}
}

int IndentClassifier::run() const
{
	m_fileEnumerator.enumerateFiles([this] (const Path& p) { processFile(p); });
	return EXIT_SUCCESS;
}

void IndentClassifier::processFile(const Path& p) const
{
	const auto lineTypeCounts{scanFile(p)};
	auto fileType{classifyFile(lineTypeCounts)};

	char iLetter = indicatorLetter(fileType);
	string dispPath = displayPath(p);
	cout << b::format("%1% %2%")
			% iLetter
			% dispPath
		<< endl;
	if (fileType == IndentType::mixed)
	{
		cout << b::format("     (Mixed:  %1% space, %2% tab, %3% JavaDoc tab, %4% mixed, %5% indeterminate)")
				% get(lineTypeCounts, IndentType::space)
				% get(lineTypeCounts, IndentType::tab)
				% (get(lineTypeCounts, IndentType::javadocTab) + get(lineTypeCounts, IndentType::javadocLeft))
				% get(lineTypeCounts, IndentType::mixed)
				% get(lineTypeCounts, IndentType::indeterminate)
			<< endl;
	}
}

LineTypeCounts IndentClassifier::scanFile(const Path& p)
{
	bool isJavaFile = isIEqual(p.extension().generic_string().c_str(), ".java");
	bfs::ifstream in(p, ::std::ios_base::in);
	return scanFile(in, isJavaFile);
}

LineTypeCounts IndentClassifier::scanFile(istream& in, bool isJavaFile)
{
	LineTypeCounts lineTypeCounts;

	for (;;)
	{
		if (in.eof())
		{
			break;
		}
		else if (!in.good())
		{
			throw IOError("Error while reading input file");
		}

		auto line{getNextLine(in)};
		auto lineType{classifyLine(line, isJavaFile)};
		++lineTypeCounts[lineType];
	}

	return lineTypeCounts;
}

IndentType IndentClassifier::classifyLine(const string& line, bool isJavaFile)
{
	if (regex_match(line, k_indeterminatePattern))
	{
		return IndentType::indeterminate;
	}
	else if (isJavaFile && regex_match(line, k_javadocLeftPattern))
	{
		return IndentType::javadocLeft;
	}
	else if (regex_match(line, k_spacePattern))
	{
		return IndentType::space;
	}
	else if (regex_match(line, k_tabPattern))
	{
		return IndentType::tab;
	}
	else if (isJavaFile && regex_match(line, k_javadocTabPattern))
	{
		return IndentType::javadocTab;
	}
	else
	{
		return IndentType::mixed;
	}
}

IndentType IndentClassifier::classifyFile(const LineTypeCounts& lineTypeCounts)
{
	if (get(lineTypeCounts, IndentType::space) > 0
		&& get(lineTypeCounts, IndentType::tab)
			+ get(lineTypeCounts, IndentType::javadocTab)
			+ get(lineTypeCounts, IndentType::mixed) == 0)
	{
		return IndentType::space;
	}
	else if (get(lineTypeCounts, IndentType::tab) > 0
		&& get(lineTypeCounts, IndentType::space)
			+ get(lineTypeCounts, IndentType::javadocTab)
			+ get(lineTypeCounts, IndentType::javadocLeft)
			+ get(lineTypeCounts, IndentType::mixed) == 0)
	{
		return IndentType::tab;
	}
	else if (get(lineTypeCounts, IndentType::tab)
			+ get(lineTypeCounts, IndentType::javadocTab) > 0
		&& get(lineTypeCounts, IndentType::space)
			+ get(lineTypeCounts, IndentType::mixed) == 0)
	{
		return IndentType::javadocTab;
	}
	else if (get(lineTypeCounts, IndentType::javadocLeft) > 0
		&& get(lineTypeCounts, IndentType::space)
			+ get(lineTypeCounts, IndentType::javadocTab)
			+ get(lineTypeCounts, IndentType::mixed) == 0)
	{
		return IndentType::javadocTab;
	}
	else if (get(lineTypeCounts, IndentType::space)
		+ get(lineTypeCounts, IndentType::tab)
		+ get(lineTypeCounts, IndentType::javadocTab)
		+ get(lineTypeCounts, IndentType::javadocLeft)
		+ get(lineTypeCounts, IndentType::mixed) == 0)
	{
		return IndentType::indeterminate;
	}
	else
	{
		return IndentType::mixed;
	}
}

char IndentClassifier::indicatorLetter(IndentType fileType)
{
	switch (fileType)
	{
	case IndentType::space:
		return 'S';
	case IndentType::tab:
		return 'T';
	case IndentType::javadocTab:
		return 'J';
	case IndentType::mixed:
		return 'M';
	case IndentType::indeterminate:
		return 'I';
	default:
		throw invalid_argument("Unrecognized IndentType enumeration value in "
			"IndentClassifier::indicatorLetter");
	}
}

// This method breaks the portability of Boost.FileSystem, but it's portable enough.
string IndentClassifier::displayPath(const Path& p)
{
	string result = p.generic_string();
	return (result.length() >= 2 && result[0] == '.' && (result[1] == '/' || result[1] == '\\'))
		? result.substr(2)
		: result;
}
