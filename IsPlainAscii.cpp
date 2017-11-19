
#include "IsPlainAscii.h"
#include "main.h"

#include <boost/algorithm/string/case_conv.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/format.hpp>
#include <iostream>

namespace b = ::boost;
namespace bfs = ::boost::filesystem;

using ::std::cout;
using ::std::endl;
using ::std::istream;
using ::std::ios_base;
using ::std::ostream;
using ::std::string;

#if !defined(CMDLINEUTIL_TEST_MODE)
int main(int argCount, const char*const*const argList)
{
	return commonMain<IsPlainAscii>(argCount, argList);
}
#endif

int IsPlainAscii::usage(ostream& out, const char* pMsg)
{
	int exitCode = EXIT_SUCCESS;
	if (pMsg != nullptr && *pMsg != '\0')
	{
		exitCode = EXIT_FAILURE;
		out << endl << pMsg << endl;
	}
	out << "\n"
		"Usage:  isplainascii [-r] <file1> <file2> ...\n"
		"\n"
		"Finds characters in the given files that are not strict 7-bit ASCII.\n"
		"\n"
		"Options:\n"
		"\n"
		"   -r Search for files in sub-directories recursively\n"
	<< endl;

	return exitCode;
}

IsPlainAscii::IsPlainAscii(size_t argCount, const char*const*const ppArgList) :
	m_fileEnumerator()
{
	for (int i = 1; i < argCount; ++i)
	{
		string arg = ppArgList[i];
		string larg = b::to_lower_copy(arg);
		if (larg == "-?" || larg == "-h" || larg == "-help")
		{
			throw CmdLineError();
		}
		else if (larg == "-r")
		{
			m_fileEnumerator.setRecursive();
		}
		else
		{
			m_fileEnumerator.insert(arg);
		}
	}

	if (m_fileEnumerator.numFileSpecs() <= 0)
	{
		throw CmdLineError("No files specified");
	}
}

int IsPlainAscii::run() const
{
	m_fileEnumerator.enumerateFiles(&IsPlainAscii::scanFile);
	return EXIT_SUCCESS;
}

void IsPlainAscii::scanFile(const Path& filePath)
{
	bfs::ifstream in(filePath, ios_base::in | ios_base::binary);
	scanFile2(filePath, in, cout);
	in.close();
}

void IsPlainAscii::scanFile2(const Path& filePath, istream& in, ostream& out)
{
	static const int k_mask = static_cast<int>((~0u) << 7);
	string nonAsciiRun;
	unsigned long lineNum = 1;
	unsigned long colNum = 1;
	unsigned long approxColNum = 0;
	bool wasLastCharCR = false;

	for (; true; ++colNum)
	{
		int ch = in.get();
		if (in.eof())
		{
			reportNonAsciiRun(filePath, lineNum, approxColNum, nonAsciiRun, out);
			break;
		}
		else if (!in.good())
		{
			throw IOError("Error while reading input file");
		}
		else if (ch == '\r')
		{
			reportNonAsciiRun(filePath, lineNum, approxColNum, nonAsciiRun, out);
			colNum = 0;
			wasLastCharCR = true;
		}
		else if (ch == '\n')
		{
			reportNonAsciiRun(filePath, lineNum, approxColNum, nonAsciiRun, out);
			++lineNum;
			colNum = 0;
			wasLastCharCR = false;
		}
		else
		{
			if ((ch & k_mask) != 0)
			{
				if (nonAsciiRun.length() == 0)
				{
					approxColNum = colNum;
				}
				nonAsciiRun += string::traits_type::to_char_type(ch);
			}
			else
			{
				reportNonAsciiRun(filePath, lineNum, approxColNum, nonAsciiRun, out);
			}
			if (wasLastCharCR)
			{
				++lineNum;
				wasLastCharCR = false;
			}
		}
	}
}

void IsPlainAscii::reportNonAsciiRun(const Path& filePath, unsigned long lineNum,
	unsigned long approxColNum, string& nonAsciiRun, ostream& out)
{
	if (!nonAsciiRun.empty())
	{
		out << b::format("%1%, line %2%, approx. column %3%:  \"%4%\" (\"")
				% filePath
				% lineNum
				% approxColNum
				% nonAsciiRun;
		for (auto ch : nonAsciiRun)
		{
			out << b::format("\\x%1$02x")
				% static_cast<unsigned int>(static_cast<unsigned char>(ch));
		}
		out << "\")" << endl;
		nonAsciiRun.clear();
	}
}