
#if !defined(CMDLINEUTIL_TEST_MODE)
#define CMDLINEUTIL_TEST_MODE
#endif

#include "Exceptions.h"
#include "IsPlainAscii.h"
#include "main.h"
#include "Utils.h"
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

using PathList = ::std::vector<bfs::path>;



BOOST_AUTO_TEST_SUITE(IsPlainAsciiTestSuite)

BOOST_AUTO_TEST_SUITE(CmdLineParseFailTestSuite)

struct TestCase
{
	char const*const*const	m_testArgs;
	size_t							m_testArgsCount;
	char const*					m_exceptionMsgTestPattern;
};

static char const*const k_args00[] = { "isplainascii" };
static char const*const k_args01[] = { "isplainascii", "-?" };
static char const*const k_args02[] = { "isplainascii", "-h" };
static char const*const k_args03[] = { "isplainascii", "-help" };

static TestCase const k_testCases[] =
{
#define MAKE_TC(args, pattern) \
	{ args, arrayLen(args), pattern },

	MAKE_TC(k_args00, ".*no files.*")
	MAKE_TC(k_args01, "^$")
	MAKE_TC(k_args02, "^$")
	MAKE_TC(k_args03, "^$")

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
	::std::string m_pattern;
	b::regex m_rex;
};

BOOST_DATA_TEST_CASE(cmdLineParseFailTest, utd::make(k_testCases), tc)
{
	CmdLineErrorPatternMatch isMatch(tc.m_exceptionMsgTestPattern);
	BOOST_CHECK_EXCEPTION(IsPlainAscii(makeArgsSpan(tc)), CmdLineError, isMatch);
}

BOOST_AUTO_TEST_SUITE_END()



BOOST_AUTO_TEST_SUITE(CmdLineParseOkTestSuite)

struct TestCase
{
	char const*const*const	m_testArgs;
	size_t						m_testArgsCount;
	char const*const*const	m_fileList;
	size_t						m_fileListLen;
};

static char const*const k_args00[] = { "isplainascii", "IsPlainAscii.cpp" };
static char const*const k_args01[] = { "isplainascii", "IsPlainAscii.cpp", "IsPlainAscii.h", "IsPlainAsciiTest.cpp" };
static char const*const k_args02[] = { "isplainascii", "IsPlainAscii*.h" };
static char const*const k_args03[] = { "isplainascii", "IsPlainAscii*.h", "IsPlainAscii*.cpp" };

static char const*const k_files00[] = { "IsPlainAscii.cpp" };
static char const*const k_files01[] = { "IsPlainAscii.cpp", "IsPlainAscii.h", "IsPlainAsciiTest.cpp" };
static char const*const k_files02[] = { "IsPlainAscii*.h" };
static char const*const k_files03[] = { "IsPlainAscii*.h", "IsPlainAscii*.cpp" };

static TestCase const k_testCases[] =
{
#define MAKE_TC(args, files) \
	{ args, arrayLen(args), files, arrayLen(files) },

	MAKE_TC(k_args00, k_files00)
	MAKE_TC(k_args01, k_files01)
	MAKE_TC(k_args02, k_files02)
	MAKE_TC(k_args03, k_files03)

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
	IsPlainAscii app(makeArgsSpan(tc));
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
};

static TestCase const k_testCases[] =
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

static ::std::ostream& operator<<(::std::ostream& ostrm, TestCase const& tc)
{
	return ostrm << "Test case #" << (&tc - k_testCases);
}

BOOST_DATA_TEST_CASE(scanFileTest, utd::make(k_testCases), tc)
{
	if (true)
	{
		BOOST_TEST_MESSAGE("Testing scanFile2 with input \"" << tc.m_input << "\"");
	}

	bfs::path filePath("TestInputStream");
	std::string input(tc.m_input);
	std::istringstream in(input);
	std::ostringstream out;
	IsPlainAscii::scanFile2(filePath, in, out);
	BOOST_CHECK_EQUAL(tc.m_output, out.str());
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()
