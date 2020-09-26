
#if !defined(CMDLINEUTIL_TEST_MODE)
#define CMDLINEUTIL_TEST_MODE
#endif

#include "Exceptions.h"
#include "IsPlainAscii.h"
#include "TestUtil.h"
#include "Utils.h"

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

using PathList = ::std::vector<fs::path>;



BOOST_AUTO_TEST_SUITE(IsPlainAsciiTestSuite)

BOOST_AUTO_TEST_SUITE(CmdLineParseFailTestSuite)

static char const*const k_args00[] = { "isplainascii" };
static char const*const k_args01[] = { "isplainascii", "-?" };
static char const*const k_args02[] = { "isplainascii", "-h" };
static char const*const k_args03[] = { "isplainascii", "-help" };

static CmdLineParseFailTestCase const k_testCases[] =
{
	{ k_args00, ".*no files.*" },
	{ k_args01, "^$" },
	{ k_args02, "^$" },
	{ k_args03, "^$" },
};

BOOST_DATA_TEST_CASE(cmdLineParseFailTest, utd::make(k_testCases), tc)
{
	BOOST_CHECK_EXCEPTION(IsPlainAscii(tc.m_args), CmdLineError,
		[&tc](const CmdLineError& ex) { return tc.doesExMatch(ex); });
}

BOOST_AUTO_TEST_SUITE_END()



BOOST_AUTO_TEST_SUITE(CmdLineParseOkTestSuite)

struct CmdLineParseOkTestCase : public CmdLineParseTestCase
{
	template<::std::size_t N, ::std::size_t M>
	CmdLineParseOkTestCase(const char*const(&args)[N],
			const char*const(&fileList)[M]) noexcept :
		CmdLineParseTestCase(args),
		m_fileList(::gsl::make_span(fileList, M))
		{}

	ArgListPair	m_fileList;
};

static char const*const k_args00[] = { "isplainascii", "IsPlainAscii.cpp" };
static char const*const k_args01[] = { "isplainascii", "IsPlainAscii.cpp", "IsPlainAscii.h", "IsPlainAsciiTest.cpp" };
static char const*const k_args02[] = { "isplainascii", "IsPlainAscii*.h" };
static char const*const k_args03[] = { "isplainascii", "IsPlainAscii*.h", "IsPlainAscii*.cpp" };

static char const*const k_files00[] = { "IsPlainAscii.cpp" };
static char const*const k_files01[] = { "IsPlainAscii.cpp", "IsPlainAscii.h", "IsPlainAsciiTest.cpp" };
static char const*const k_files02[] = { "IsPlainAscii*.h" };
static char const*const k_files03[] = { "IsPlainAscii*.h", "IsPlainAscii*.cpp" };

static CmdLineParseOkTestCase const k_testCases[] =
{
	{ k_args00, k_files00 },
	{ k_args01, k_files01 },
	{ k_args02, k_files02 },
	{ k_args03, k_files03 },
};

static void checkEqual(const fs::path& tcPath, const fs::path& appPath)
{
	BOOST_CHECK_EQUAL(tcPath.generic_string(), appPath.generic_string());
}

BOOST_DATA_TEST_CASE(cmdLineParseOkTest, utd::make(k_testCases), tc)
{
	IsPlainAscii app(tc.m_args);
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
};

static ScanFileTestCase const k_testCases[] =
{
	// input, output
	{
		"",
		""
	},
	{
		"This one is plain ASCII",
		""
	},
	{
		"This \none \ris \r\nplain ASCII also",
		""
	},
	{
		"\xef\xbb\xbfThis one has a UTF-8 BOM",
		"\"TestInputStream\", line 1, approx. column 1:  \"\xef\xbb\xbf\" (\"\\xef\\xbb\\xbf\")\n"
	},
	{
		"Line 1\n"
		"Line 2\r"
		"Line 3\r\n"
		"We often start by saying \xe2\x80\x9cHello world!\xe2\x80\x9d, even "
		"though that\xe2\x80\x99s silly\xe2\x80\xa6",

		"\"TestInputStream\", line 4, approx. column 26:  \"\xe2\x80\x9c\" (\"\\xe2\\x80\\x9c\")\n"
		"\"TestInputStream\", line 4, approx. column 41:  \"\xe2\x80\x9d\" (\"\\xe2\\x80\\x9d\")\n"
		"\"TestInputStream\", line 4, approx. column 62:  \"\xe2\x80\x99\" (\"\\xe2\\x80\\x99\")\n"
		"\"TestInputStream\", line 4, approx. column 72:  \"\xe2\x80\xa6\" (\"\\xe2\\x80\\xa6\")\n"
	},
};

static ::std::ostream& operator<<(::std::ostream& ostrm, ScanFileTestCase const& tc)
{
	return ostrm << "Test case with input \"" << tc.m_pInput << "\"";
}

BOOST_DATA_TEST_CASE(scanFileTest, utd::make(k_testCases), tc)
{
	if (false)
	{
		BOOST_TEST_MESSAGE("Testing scanFile2 with input \"" << tc.m_pInput << "\"");
	}

	fs::path filePath("TestInputStream");
	std::string input(tc.m_pInput);
	std::istringstream in(input);
	std::ostringstream out;
	IsPlainAscii::scanFile2(filePath, in, out);
	BOOST_CHECK_EQUAL(tc.m_pOutput, out.str());
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()
