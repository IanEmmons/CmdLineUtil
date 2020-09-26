
#include "Xeol.h"
#include "PathDeleter.h"
#include "main.h"
#include "Utils.h"

#include <boost/format.hpp>
#include <fstream>
#include <iostream>
#include <stdexcept>

namespace b = ::boost;

using ::std::cout;
using ::std::endl;
using ::std::ifstream;
using ::std::istream;
using ::std::invalid_argument;
using ::std::ios_base;
using ::std::logic_error;
using ::std::ofstream;
using ::std::ostream;
using ::std::string;

#if !defined(CMDLINEUTIL_TEST_MODE)
int main(int argCount, const char*const*const argList)
{
	return commonMain<Xeol>(argCount, argList);
}
#endif

int Xeol::usage(ostream& out, const string& progName, const char* pMsg)
{
	int exitCode = EXIT_SUCCESS;
	if (pMsg != nullptr && *pMsg != '\0')
	{
		exitCode = EXIT_FAILURE;
		out << endl << pMsg << endl;
	}
	out << "\n"
		"Usage 1:  " << progName << " [-r] <file1> <file2> ...\n"
		"Usage 2:  " << progName << " {-d|-m|-u} [-f] [-r] <file1> <file2> ...\n"
		"\n"
		"The first usage lists the line ending convention of each file,\n"
		"in the following format:\n"
		"\n"
		"   <L> <file-path>\n"
		"\n"
		"where <L> is 'D' for DOS, 'M' for Macintosh, 'U' for Unix,\n"
		"'I' for indeterminate (meaning the file has no line endings),\n"
		"or 'X' for mixed.\n"
		"\n"
		"The second usage changes the line endings to the indicated type.\n"
		"\n"
		"Options:\n"
		"\n"
		"   -d Change to DOS line endings\n"
		"\n"
		"   -m Change to pre-System X Macintosh line endings\n"
		"\n"
		"   -u Change to Unix line endings (also Linux and MacOS X)\n"
		"\n"
		"   -f Force translation of files with mixed line endings\n"
		"\n"
		"   -r Search for files in sub-directories recursively\n"
		<< endl;

	return exitCode;
}

Xeol::Xeol(::gsl::span<const char*const> args) :
	m_isInQueryMode(true),
	m_targetEolType(EolType::INDETERMINATE),
	m_forceTranslation(false),
	m_fileEnumerator()
{
	unsigned numTargetTypeArgs = 0;
	for (auto pArg : args)
	{
		if (isIEqual(pArg, "-?") || isIEqual(pArg, "-h") || isIEqual(pArg, "-help"))
		{
			throw CmdLineError();
		}
		else if (isIEqual(pArg, "-d"))
		{
			if (m_targetEolType != EolType::DOS)
			{
				m_isInQueryMode = false;
				m_targetEolType = EolType::DOS;
				++numTargetTypeArgs;
			}
		}
		else if (isIEqual(pArg, "-m"))
		{
			if (m_targetEolType != EolType::MACINTOSH)
			{
				m_isInQueryMode = false;
				m_targetEolType = EolType::MACINTOSH;
				++numTargetTypeArgs;
			}
		}
		else if (isIEqual(pArg, "-u"))
		{
			if (m_targetEolType != EolType::UNIX)
			{
				m_isInQueryMode = false;
				m_targetEolType = EolType::UNIX;
				++numTargetTypeArgs;
			}
		}
		else if (isIEqual(pArg, "-f"))
		{
			m_forceTranslation = true;
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

	if (m_isInQueryMode && m_forceTranslation)
	{
		throw CmdLineError("The option '-f' is allowed only if one of '-d', '-m', and '-u' is present");
	}
	else if (numTargetTypeArgs > 1)
	{
		throw CmdLineError("The options '-d', '-m', and '-u' are mutually exclusive");
	}
	else if (m_fileEnumerator.numFileSpecs() <= 0)
	{
		throw CmdLineError("No files specified");
	}
}

int Xeol::run() const
{
	m_fileEnumerator.enumerateFiles([this] (const Path& p)
		{ m_isInQueryMode ? queryFile(p) : translateFile(p); });
	return EXIT_SUCCESS;
}

void Xeol::queryFile(const Path& p) const
{
	size_t numDosEols = 0;
	size_t numMacEols = 0;
	size_t numUnixEols = 0;
	size_t totalEols = 0;
	ifstream in(p, ios_base::in | ios_base::binary);
	EolType eolType = scanFile(in, numDosEols, numMacEols, numUnixEols, totalEols);

	char iLetter = getIndicatorLetter(eolType);
	string dispPath = displayPath(p);
	if (eolType == EolType::MIXED)
	{
		cout << b::format("%1% %2%\n"
			"     (Mixed:  %3% DOS, %4% Mac, %5% Unix)")
				% iLetter
				% dispPath
				% numDosEols
				% numMacEols
				% numUnixEols
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

void Xeol::translateFile(const Path& p) const
{
	size_t numDosEols = 0;
	size_t numMacEols = 0;
	size_t numUnixEols = 0;
	size_t totalEols = 0;

	ifstream in(p, ios_base::in | ios_base::binary);

	Path tempPath(getTempPath(p));
	PathDeleter tempPathDeleter(tempPath);
	ofstream out(tempPath, ios_base::out | ios_base::trunc | ios_base::binary);

	EolType eolType = scanFile(in, numDosEols, numMacEols, numUnixEols,
		totalEols, &out, m_targetEolType);

	in.close();
	out.close();

	if (eolType == EolType::INDETERMINATE || eolType == m_targetEolType)
	{
		// Do nothing
	}
	else if (eolType == EolType::MIXED && !m_forceTranslation)
	{
		cout << b::format("---- %1%\n"
			"        (Skipped, possibly binary:  %2% DOS, %3% Mac, %4% Unix)")
				% p
				% numDosEols
				% numMacEols
				% numUnixEols
			<< endl;
	}
	else
	{
		if (eolType != EolType::DOS
			&& eolType != EolType::MACINTOSH
			&& eolType != EolType::UNIX
			&& !(eolType == EolType::MIXED && m_forceTranslation))
		{
			throw logic_error("Unrecognized EolType enumeration value in Xeol::translateFile");
		}

		replaceOriginalFileWithTemp(p, tempPath);
		cout << b::format("%1%->%2% %3%")
				% getIndicatorLetter(eolType)
				% getIndicatorLetter(m_targetEolType)
				% p
			<< endl;
	}
}

Xeol::EolType Xeol::scanFile(istream& in, /* out */ size_t& numDosEols, /* out */ size_t& numMacEols,
	/* out */ size_t& numUnixEols, /* out */ size_t& totalEols, ostream* pOut,
	EolType targetEolType)
{
	numDosEols = 0;
	numMacEols = 0;
	numUnixEols = 0;
	totalEols = 0;

	string eol(toEolString(targetEolType));
	if (pOut != nullptr && eol.length() <= 0)
	{
		throw invalid_argument("Invalid value for argument targetEolType in method Xeol::scanFile");
	}

	for (bool lastCharWasReturn = false; true;)
	{
		int ch = in.get();
		if (in.eof())
		{
			if (lastCharWasReturn)
			{
				++numMacEols;
				if (pOut != nullptr) { (*pOut) << eol; }
			}
			break;
		}
		else if (!in.good())
		{
			throw IOError("Error while reading input file");
		}
		else if (pOut != nullptr && !pOut->good())
		{
			throw IOError("Error while writing output to temporary file");
		}
		else if (lastCharWasReturn)
		{
			if (ch == '\r')
			{
				++numMacEols;
				if (pOut != nullptr) { (*pOut) << eol; }
				lastCharWasReturn = true;
			}
			else if (ch == '\n')
			{
				++numDosEols;
				if (pOut != nullptr) { (*pOut) << eol; }
				lastCharWasReturn = false;
			}
			else
			{
				++numMacEols;
				if (pOut != nullptr) { (*pOut) << eol << istream::traits_type::to_char_type(ch); }
				lastCharWasReturn = false;
			}
		}
		else
		{
			if (ch == '\r')
			{
				lastCharWasReturn = true;
			}
			else if (ch == '\n')
			{
				++numUnixEols;
				if (pOut != nullptr) { (*pOut) << eol; }
				lastCharWasReturn = false;
			}
			else
			{
				if (pOut != nullptr) { (*pOut) << istream::traits_type::to_char_type(ch); }
				lastCharWasReturn = false;
			}
		}
	}

	totalEols = numDosEols + numMacEols + numUnixEols;

	EolType eolType = EolType::MIXED;
	if (numDosEols == 0 && numMacEols == 0 && numUnixEols == 0)
	{
		eolType = EolType::INDETERMINATE;
	}
	else if (numDosEols > 0 && numMacEols == 0 && numUnixEols == 0)
	{
		eolType = EolType::DOS;
	}
	else if (numDosEols == 0 && numMacEols > 0 && numUnixEols == 0)
	{
		eolType = EolType::MACINTOSH;
	}
	else if (numDosEols == 0 && numMacEols == 0 && numUnixEols > 0)
	{
		eolType = EolType::UNIX;
	}
	return eolType;
}

char Xeol::getIndicatorLetter(EolType eolType)
{
	switch (eolType)
	{
	case EolType::INDETERMINATE:
		return 'I';
	case EolType::MIXED:
		return 'X';
	case EolType::DOS:
		return 'D';
	case EolType::MACINTOSH:
		return 'M';
	case EolType::UNIX:
		return 'U';
	default:
		throw invalid_argument("Unrecognized EolType enumeration value in "
			"Xeol::getIndicatorLetter");
	}
}

string Xeol::toEolString(EolType eolType)
{
	switch (eolType)
	{
	case EolType::DOS:
		return "\r\n";
	case EolType::MACINTOSH:
		return "\r";
	case EolType::UNIX:
		return "\n";
	default:
		return "";
	}
}

// This method breaks the portability of Boost.FileSystem, but it's portable enough.
string Xeol::displayPath(const Path& p)
{
	string result = p.generic_string();
	return (result.length() >= 2 && result[0] == '.' && (result[1] == '/' || result[1] == '\\'))
		? result.substr(2)
		: result;
}

void Xeol::replaceOriginalFileWithTemp(const Path& originalPath, const Path& tempPath)
{
	Path savedOriginalPath(getTempPath(originalPath));
	rename(originalPath, savedOriginalPath);
	rename(tempPath, originalPath);
	PathDeleter savedOriginalPathDeleter(savedOriginalPath);
}
