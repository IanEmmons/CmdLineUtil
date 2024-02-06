
#if !defined(CMDLINEUTIL_TEST_MODE)
#define CMDLINEUTIL_TEST_MODE
#endif

#include "Exceptions.h"
#include "StripWS.h"
#include "TestUtil.h"
#include "Utils.h"

#include <algorithm>
#include <boost/range/algorithm_ext/for_each.hpp>
#include <boost/range/algorithm/sort.hpp>
#include <boost/test/unit_test.hpp>
#include <boost/test/data/test_case.hpp>
#include <sstream>
#include <vector>

namespace b = ::boost;
namespace fs = ::std::filesystem;
namespace ut = ::boost::unit_test;
namespace utd = ::boost::unit_test::data;

using ::std::begin;
using ::std::end;
using ::std::istringstream;
using ::std::ostringstream;
using ::std::string;

using PathList = ::std::vector<fs::path>;

BOOST_AUTO_TEST_SUITE(StripWSTestSuite)

BOOST_AUTO_TEST_SUITE(CmdLineParseFailTestSuite)

static char const*const k_args00[] = { "stripws" };
static char const*const k_args01[] = { "stripws", "-?" };
static char const*const k_args02[] = { "stripws", "-h" };
static char const*const k_args03[] = { "stripws", "-help" };
static char const*const k_args04[] = { "stripws", "-s" };

static CmdLineParseFailTestCase const k_testCases[] =
{
	{ k_args00, ".*no files.*" },
	{ k_args01, "^$" },
	{ k_args02, "^$" },
	{ k_args03, "^$" },
	{ k_args04, ".*no files.*" },
};

BOOST_DATA_TEST_CASE(cmdLineParseFailTest, utd::make(k_testCases), tc)
{
	BOOST_CHECK_EXCEPTION(StripWS(tc.m_args), CmdLineError,
		[&tc](const CmdLineError& ex) { return tc.doesExMatch(ex); });
}

BOOST_AUTO_TEST_SUITE_END()



BOOST_AUTO_TEST_SUITE(CmdLineParseOkTestSuite)

struct CmdLineParseOkTestCase : public CmdLineParseTestCase
{
	template<::std::size_t N, ::std::size_t M>
	CmdLineParseOkTestCase(const char*const(&args)[N], bool isInQueryMode,
			const char*const(&fileList)[M]) noexcept :
		CmdLineParseTestCase(args),
		m_isInQueryMode(isInQueryMode),
		m_fileList(::std::span{fileList, M})
		{}

	bool			m_isInQueryMode;
	ArgSpan	m_fileList;
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

static CmdLineParseOkTestCase const k_testCases[] =
{
	{ k_args00, true, k_files00 },
	{ k_args01, false, k_files00 },
	{ k_args02, false, k_files00 },
	{ k_args03, true, k_files01 },
	{ k_args04, true, k_files02 },
	{ k_args05, true, k_files03 },
};

static void checkEqual(const fs::path& tcPath, const fs::path& appPath)
{
	BOOST_CHECK_EQUAL(tcPath.generic_string(), appPath.generic_string());
}

BOOST_DATA_TEST_CASE(cmdLineParseOkTest, utd::make(k_testCases), tc)
{
	StripWS app(tc.m_args);
	BOOST_CHECK_EQUAL(tc.m_isInQueryMode, app.m_isInQueryMode);
	BOOST_CHECK_EQUAL(tc.m_fileList.size(), app.m_fileEnumerator.numFileSpecs());

	PathList tcList(begin(tc.m_fileList), end(tc.m_fileList));
	b::sort(tcList);
	PathList appList;
	app.m_fileEnumerator.getFileSpecList(appList);
	b::sort(appList);
	b::for_each(tcList, appList, checkEqual);
}

BOOST_AUTO_TEST_SUITE_END()



BOOST_AUTO_TEST_SUITE(ScanFileTestSuite)

struct ScanFileTestCase
{
	char const*	m_pInput;
	char const*	m_pOutput;
	size_t			m_numLinesAffected;
	size_t			m_numSpacesStripped;
	size_t			m_numTabsStripped;
};

static ScanFileTestCase const k_testCases[] =
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

static ::std::ostream& operator<<(::std::ostream& ostrm, ScanFileTestCase const& tc)
{
	return ostrm << "Test case with input \"" << tc.m_pInput << "\"";
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
	if (false)
	{
		BOOST_TEST_MESSAGE("Testing scanFile with input \"" << tc.m_pInput << "\"");
	}
	string input(tc.m_pInput);
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
		BOOST_CHECK_EQUAL(tc.m_pOutput, out.str());
	}
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()
