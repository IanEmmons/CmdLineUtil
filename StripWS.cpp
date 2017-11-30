
#include "StripWS.h"
#include "PathDeleter.h"
#include "main.h"
#include "Utils.h"

#include <boost/algorithm/string/replace.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/format.hpp>

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
	return commonMain<StripWS>(argCount, argList);
}
#endif

int StripWS::usage(ostream& out, const char* pMsg)
{
	int exitCode = EXIT_SUCCESS;
	if (pMsg != nullptr && *pMsg != '\0')
	{
		exitCode = EXIT_FAILURE;
		out << endl << pMsg << endl;
	}
	out << "\n"
		"Usage:  stripws [-s] [-r] <file1> <file2> ...\n"
		"\n"
		"Strips white space (tabs and spaces) from the ends of lines and\n"
		"at the end of the file, if the file is not terminated by an\n"
		"end-of-line character.\n"
		"\n"
		"With no options operates in query-only mode, i.e., report on the\n"
		"white space that could be stripped, but do not change the file.\n"
		"\n"
		"Options:\n"
		"\n"
		"   -s Strip white space, i.e., actually alter the file\n"
		"\n"
		"   -r Search for files in sub-directories recursively\n"
		<< endl;

	return exitCode;
}

StripWS::StripWS(::gsl::span<const char*const> args) :
	m_isInQueryMode(true),
	m_fileEnumerator()
{
	for (auto pArg : args)
	{
		if (isIEqual(pArg, "-?") || isIEqual(pArg, "-h") || isIEqual(pArg, "-help"))
		{
			throw CmdLineError();
		}
		else if (isIEqual(pArg, "-s"))
		{
			m_isInQueryMode = false;
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

int StripWS::run() const
{
	m_fileEnumerator.enumerateFiles([this] (const Path& p)
		{ m_isInQueryMode ? queryFile(p) : translateFile(p); });
	return EXIT_SUCCESS;
}

void StripWS::queryFile(const Path& p) const
{
	size_t numLinesAffected = 0;
	size_t numSpacesStripped = 0;
	size_t numTabsStripped = 0;
	bfs::ifstream in(p, ios_base::in | ios_base::binary);
	scanFile(in, numLinesAffected, numSpacesStripped, numTabsStripped);

	if (numLinesAffected > 0)
	{
		cout << b::format("   %1% -- %4% lines end in %2% spaces and %3% tabs")
				% p
				% numSpacesStripped
				% numTabsStripped
				% numLinesAffected
			<< endl;
	}
}

void StripWS::translateFile(const Path& p) const
{
	size_t numLinesAffected = 0;
	size_t numSpacesStripped = 0;
	size_t numTabsStripped = 0;

	bfs::ifstream in(p, ios_base::in | ios_base::binary);

	Path tempPath(getTempPath(p));
	PathDeleter tempPathDeleter(tempPath);
	bfs::ofstream out(tempPath, ios_base::out | ios_base::trunc | ios_base::binary);

	scanFile(in, numLinesAffected, numSpacesStripped, numTabsStripped, &out);

	in.close();
	out.close();

	if (numLinesAffected > 0)
	{
		replaceOriginalFileWithTemp(p, tempPath);
		cout << b::format("   %1% -- %2% spaces and %3% tabs stripped from %4% lines")
				% p
				% numSpacesStripped
				% numTabsStripped
				% numLinesAffected
			<< endl;
	}
}

void StripWS::scanFile(istream& in, /* out */ size_t& numLinesAffected,
	/* out */ size_t& numSpacesStripped, /* out */ size_t& numTabsStripped, ostream* pOut)
{
	numLinesAffected = 0;
	numSpacesStripped = 0;
	numTabsStripped = 0;

	for (string wsRun; true;)
	{
		int ch = in.get();
		if (in.eof())
		{
			updateCounts(wsRun, numLinesAffected, numSpacesStripped, numTabsStripped);
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
		else if (ch == ' ' || ch == '\t')
		{
			wsRun += string::traits_type::to_char_type(ch);
		}
		else if (ch == '\r' || ch == '\n')
		{
			updateCounts(wsRun, numLinesAffected, numSpacesStripped, numTabsStripped);
			if (pOut != nullptr)
			{
				(*pOut) << istream::traits_type::to_char_type(ch);
			}
			wsRun.clear();
		}
		else
		{
			if (pOut != nullptr)
			{
				(*pOut) << wsRun << istream::traits_type::to_char_type(ch);
			}
			wsRun.clear();
		}
	}
}

void StripWS::updateCounts(string const& wsRun, /* in-out */ size_t& numLinesAffected,
	/* in-out */ size_t& numSpacesStripped, /* in-out */ size_t& numTabsStripped)
{
	if (wsRun.length() > 0)
	{
		++numLinesAffected;
		for (auto ch : wsRun)
		{
			if (ch == ' ')
			{
				++numSpacesStripped;
			}
			if (ch == '\t')
			{
				++numTabsStripped;
			}
		}
	}
}

void StripWS::replaceOriginalFileWithTemp(const Path& originalPath, const Path& tempPath)
{
	Path savedOriginalPath(getTempPath(originalPath));
	rename(originalPath, savedOriginalPath);
	rename(tempPath, originalPath);
	PathDeleter savedOriginalPathDeleter(savedOriginalPath);
}
