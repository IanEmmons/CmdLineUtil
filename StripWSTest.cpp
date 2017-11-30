
#if !defined(CMDLINEUTIL_TEST_MODE)
#define CMDLINEUTIL_TEST_MODE
#endif

#include "Exceptions.h"
#include "main.h"
#include "StripWS.h"
#include "Utils.h"

#include <algorithm>
#include <boost/range/algorithm_ext/for_each.hpp>
#include <boost/range/algorithm/sort.hpp>
#include <boost/regex.hpp>
#include <boost/test/unit_test.hpp>
#include <boost/test/data/test_case.hpp>
#include <sstream>
#include <vector>

namespace b = ::boost;
namespace bfs = ::boost::filesystem;
namespace ut = ::boost::unit_test;
namespace utd = ::boost::unit_test::data;

using ::std::istringstream;
using ::std::ostringstream;
using ::std::string;

using PathList = ::std::vector<bfs::path>;

BOOST_AUTO_TEST_SUITE(StripWSTestSuite)

BOOST_AUTO_TEST_SUITE(CmdLineParseFailTestSuite)

struct TestCase
{
	char const*const*const	m_testArgs;
	size_t							m_testArgsCount;
	char const*					m_exceptionMsgTestPattern;
};

static char const*const k_args00[] = { "stripws" };
static char const*const k_args01[] = { "stripws", "-?" };
static char const*const k_args02[] = { "stripws", "-h" };
static char const*const k_args03[] = { "stripws", "-help" };
static char const*const k_args04[] = { "stripws", "-s" };

static TestCase const k_testCases[] =
{
#define MAKE_TC(args, pattern) \
	{ args, arrayLen(args), pattern },

	MAKE_TC(k_args00, ".*no files.*")
	MAKE_TC(k_args01, "^$")
	MAKE_TC(k_args02, "^$")
	MAKE_TC(k_args03, "^$")
	MAKE_TC(k_args04, ".*no files.*")

#undef MAKE_TC
};

static ::std::ostream& operator<<(::std::ostream& ostrm, TestCase const& tc)
{
	return ostrm << "Test case #" << (&tc - k_testCases);
}

struct CmdLineErrorPatternMatch
{
	CmdLineErrorPatternMatch(char const* exceptionMsgTestPattern) :
		m_pattern(exceptionMsgTestPattern),
		m_rex(exceptionMsgTestPattern, b::regex::normal | b::regex::icase) {}

	bool operator()(CmdLineError const& ex)
	{
		if (false)
		{
			BOOST_TEST_MESSAGE("Testing exception message \"" << ex.what()
				<< "\" against \"" << m_pattern << "\"");
		}
		return regex_match(ex.what(), m_rex);
	}

private:
	string m_pattern;
	b::regex m_rex;
};

BOOST_DATA_TEST_CASE(cmdLineParseFailTest, utd::make(k_testCases), tc)
{
	CmdLineErrorPatternMatch isMatch(tc.m_exceptionMsgTestPattern);
	BOOST_CHECK_EXCEPTION(StripWS(makeArgsSpan(tc)), CmdLineError, isMatch);
}

BOOST_AUTO_TEST_SUITE_END()



BOOST_AUTO_TEST_SUITE(CmdLineParseOkTestSuite)

struct TestCase
{
	char const*const*const	m_testArgs;
	size_t							m_testArgsCount;
	bool							m_isInQueryMode;
	char const*const*const	m_fileList;
	size_t							m_fileListLen;
};

static char const*const k_args00[] = { "stripws", "StripWS.cpp" };
static char const*const k_args01[] = { "stripws", "-s", "StripWS.cpp" };
static char const*const k_args02[] = { "stripws", "-s", "-s", "StripWS.cpp" };
static char const*const k_args03[] = { "stripws", "StripWS.cpp", "StripWS.h", "StripWSTest.cpp" };
static char const*const k_args04[] = { "stripws", "Strip*.h" };
static char const*const k_args05[] = { "stripws", "Strip*.h", "Strip*.cpp" };

static char const*const k_files00[] = { "StripWS.cpp" };
static char const*const k_files01[] = { "StripWS.cpp", "StripWS.h", "StripWSTest.cpp" };
static char const*const k_files02[] = { "Strip*.h" };
static char const*const k_files03[] = { "Strip*.h", "Strip*.cpp" };

static TestCase const k_testCases[] =
{
#define MAKE_TC(args, isInQueryMode, files) \
	{ args, arrayLen(args), isInQueryMode, files, arrayLen(files) },

	MAKE_TC(k_args00, true, k_files00)
	MAKE_TC(k_args01, false, k_files00)
	MAKE_TC(k_args02, false, k_files00)
	MAKE_TC(k_args03, true, k_files01)
	MAKE_TC(k_args04, true, k_files02)
	MAKE_TC(k_args05, true, k_files03)

#undef MAKE_TC
};

static ::std::ostream& operator<<(::std::ostream& ostrm, TestCase const& tc)
{
	return ostrm << "Test case #" << (&tc - k_testCases);
}

static void checkEqual(const bfs::path& tcPath, const bfs::path& appPath)
{
	BOOST_CHECK_EQUAL(tcPath.generic_string(), appPath.generic_string());
}

BOOST_DATA_TEST_CASE(cmdLineParseOkTest, utd::make(k_testCases), tc)
{
	StripWS app(makeArgsSpan(tc));
	BOOST_CHECK_EQUAL(tc.m_isInQueryMode, app.m_isInQueryMode);
	BOOST_CHECK_EQUAL(tc.m_fileListLen, app.m_fileEnumerator.numFileSpecs());

	PathList tcList(tc.m_fileList, tc.m_fileList + tc.m_fileListLen);
	b::sort(tcList);
	PathList appList;
	app.m_fileEnumerator.getFileSpecList(appList);
	b::sort(appList);
	b::for_each(tcList, appList, checkEqual);
}

BOOST_AUTO_TEST_SUITE_END()



BOOST_AUTO_TEST_SUITE(ScanFileTestSuite)

struct TestCase
{
	char const*	m_input;
	char const*	m_output;
	size_t			m_numLinesAffected;
	size_t			m_numSpacesStripped;
	size_t			m_numTabsStripped;
};

static TestCase const k_testCases[] =
{
	// input, output, numLinesAffected, numSpacesStripped, numTabsStripped
	{
		"",
		"",
		0, 0, 0
	},
	{
		"This one has no white space to strip at all",
		"This one has no white space to strip at all",
		0, 0, 0
	},
	{
		"This one has spaces to strip at the end of the file ",
		"This one has spaces to strip at the end of the file",
		1, 1, 0
	},
	{
		"This one has tabs to strip at the end of the file\t",
		"This one has tabs to strip at the end of the file",
		1, 0, 1
	},
	{
		" \nHere we see\t\rthree lines to strip    \r\n",
		"\nHere we see\rthree lines to strip\r\n",
		3, 5, 1
	},
	{
		" \t \nHere  \t\t  \t\t  we see\t\t\t\rthree more lines to strip  \t\t  \t\t  \n",
		"\nHere  \t\t  \t\t  we see\rthree more lines to strip\n",
		3, 8, 8
	}
};

static ::std::ostream& operator<<(::std::ostream& ostrm, TestCase const& tc)
{
	return ostrm << "Test case #" << (&tc - k_testCases);
}

static void dumpHex(string const& s)
{
	if (false)
	{
		ostringstream strm;
		strm << "Output: ";
		for (auto ch : s)
		{
			strm << ' ' << static_cast<unsigned int>(static_cast<unsigned char>(ch));
		}
		BOOST_TEST_MESSAGE(strm.str());
	}
}

BOOST_DATA_TEST_CASE(scanFileTest, utd::make(k_testCases), tc)
{
	if (true)
	{
		BOOST_TEST_MESSAGE("Testing scanFile with input \"" << tc.m_input << "\"");
	}
	string input(tc.m_input);
	size_t numLinesAffected;
	size_t numSpacesStripped;
	size_t numTabsStripped;
	{
		istringstream in(input);
		StripWS::scanFile(in, numLinesAffected, numSpacesStripped, numTabsStripped);
		BOOST_CHECK_EQUAL(tc.m_numLinesAffected, numLinesAffected);
		BOOST_CHECK_EQUAL(tc.m_numSpacesStripped, numSpacesStripped);
		BOOST_CHECK_EQUAL(tc.m_numTabsStripped, numTabsStripped);
	}

	{
		istringstream in(input);
		ostringstream out;
		StripWS::scanFile(in, numLinesAffected, numSpacesStripped, numTabsStripped, &out);
		BOOST_CHECK_EQUAL(tc.m_numLinesAffected, numLinesAffected);
		BOOST_CHECK_EQUAL(tc.m_numSpacesStripped, numSpacesStripped);
		BOOST_CHECK_EQUAL(tc.m_numTabsStripped, numTabsStripped);
		dumpHex(out.str());
		BOOST_CHECK_EQUAL(tc.m_output, out.str());
	}
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()
