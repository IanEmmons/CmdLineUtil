
#include "IndentClassifier.h"
#include "main.h"

#include <boost/filesystem/fstream.hpp>
#include <boost/format.hpp>
#include <iostream>
#include <stdexcept>

namespace b = ::boost;
namespace bfs = ::boost::filesystem;

using ::std::cout;
using ::std::endl;
using ::std::istream;
using ::std::invalid_argument;
using ::std::ios_base;
using ::std::logic_error;
using ::std::ostream;
using ::std::string;

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
		"   <I> <file-path>\n"
		"\n"
		"where <I> is 'S' for spaces, 'T' for tabs, 'M' for mixed,\n"
		"or 'I' for indeterminate (meaning lines are not indented).\n"
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
	m_fileEnumerator.enumerateFiles([this] (const Path& p) { queryFile(p); });
	return EXIT_SUCCESS;
}

void IndentClassifier::queryFile(const Path& p) const
{
	size_t numSpaceLines = 0;
	size_t numTabLines = 0;
	size_t numMixedLines = 0;
	size_t numIndLines = 0;
	size_t totalLines = 0;
	bfs::ifstream in(p, ios_base::in);
	IndentType iType = scanFile(in, numSpaceLines, numTabLines, numMixedLines,
		numIndLines, totalLines);

	char iLetter = getIndicatorLetter(iType);
	string dispPath = displayPath(p);
	if (iType == IndentType::MIXED)
	{
		cout << b::format("%1% %2%\n"
			"     (Mixed:  %3% space, %4% tab, %5% mixed, %6% indeterminate)")
				% iLetter
				% dispPath
				% numSpaceLines
				% numTabLines
				% numMixedLines
				% numIndLines
			<< endl;
	}
	else
	{
		cout << b::format("%1% %2%")
				% iLetter
				% dispPath
			<< endl;
	}
}

IndentClassifier::IndentType IndentClassifier::scanFile(istream& in,
	/* out */ size_t& numSpaceLines, /* out */ size_t& numTabLines,
	/* out */ size_t& numMixedLines, /* out */ size_t& numIndLines,
	/* out */ size_t& totalLines)
{
	numSpaceLines = 0;
	numTabLines = 0;
	numMixedLines = 0;
	numIndLines = 0;
	totalLines = 0;

	while (true)
	{
		if (in.eof())
		{
			break;
		}
		else if (!in.good())
		{
			throw IOError("Error while reading input file");
		}

		string line;
		getline(in, line);

		size_t numSpaces = 0;
		size_t numTabs = 0;
		string::size_type firstNonWhiteSpace = line.find_first_not_of(" \t");
		if (firstNonWhiteSpace == string::npos)
		{
			firstNonWhiteSpace = line.length();
		}
		for (string::size_type i = 0; i < firstNonWhiteSpace; ++i)
		{
			if (line[i] == ' ')
			{
				++numSpaces;
			}
			else if (line[i] == '\t')
			{
				++numTabs;
			}
		}

		if (numSpaces == 0 && numTabs == 0)
		{
			++numIndLines;
		}
		else if (numSpaces > 0 && numTabs == 0)
		{
			++numSpaceLines;
		}
		else if (numSpaces == 0 && numTabs > 0)
		{
			++numTabLines;
		}
		else
		{
			++numMixedLines;
		}
	}

	totalLines = numSpaceLines + numTabLines + numMixedLines + numIndLines;

	IndentType iType = IndentType::MIXED;
	if (numSpaceLines > 0 && numTabLines == 0 && numMixedLines == 0)
	{
		iType = IndentType::SPACE;
	}
	else if (numSpaceLines == 0 && numTabLines > 0 && numMixedLines == 0)
	{
		iType = IndentType::TAB;
	}
	else if (numSpaceLines == 0 && numTabLines == 0 && numMixedLines == 0)
	{
		iType = IndentType::INDETERMINATE;
	}
	return iType;
}

char IndentClassifier::getIndicatorLetter(IndentType iType)
{
	switch (iType)
	{
	case IndentType::SPACE:
		return 'S';
	case IndentType::TAB:
		return 'T';
	case IndentType::MIXED:
		return 'M';
	case IndentType::INDETERMINATE:
		return 'I';
	default:
		throw invalid_argument("Unrecognized IndentType enumeration value in "
			"IndentClassifier::getIndicatorLetter");
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
