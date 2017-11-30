
#if !defined(CMDLINEUTIL_TEST_MODE)
#define CMDLINEUTIL_TEST_MODE
#endif

#include "Exceptions.h"
#include "IndentClassifier.h"
#include "main.h"
#include "PathDeleter.h"
#include "Utils.h"

#include <boost/range/algorithm_ext/for_each.hpp>
#include <boost/range/algorithm/sort.hpp>
#include <boost/regex.hpp>
#include <boost/test/unit_test.hpp>
#include <boost/test/data/test_case.hpp>
#include <string>
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



BOOST_AUTO_TEST_SUITE(IndentClassifierTestSuite)

BOOST_AUTO_TEST_SUITE(CmdLineParseFailTestSuite)

struct TestCase
{
	char const*const*const	m_testArgs;
	size_t							m_testArgsCount;
	char const*					m_exceptionMsgTestPattern;
};

static char const*const k_args00[] = { "indents" };
static char const*const k_args01[] = { "indents", "-?" };
static char const*const k_args02[] = { "indents", "-h" };
static char const*const k_args03[] = { "indents", "-help" };
static char const*const k_args04[] = { "indents", "-r" };

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
	BOOST_CHECK_EXCEPTION(IndentClassifier(makeArgsSpan(tc)), CmdLineError, isMatch);
}

BOOST_AUTO_TEST_SUITE_END()



BOOST_AUTO_TEST_SUITE(CmdLineParseOkTestSuite)

struct TestCase
{
	char const*const*const	m_testArgs;
	size_t							m_testArgsCount;
	bool							m_isRecursive;
	char const*const*const	m_fileList;
	size_t							m_fileListLen;
};

static char const*const k_args00[] = { "indents", "IndentClassifier.cpp" };
static char const*const k_args01[] = { "indents", "-r", "IndentClassifier*.h", "IndentClassifier*.cpp" };

static char const*const k_files00[] = { "IndentClassifier.cpp" };
static char const*const k_files01[] = { "IndentClassifier*.h", "IndentClassifier*.cpp" };

static TestCase const k_testCases[] =
{
#define MAKE_TC(args, isRecursive, files) \
	{ args, arrayLen(args), isRecursive, files, arrayLen(files) },

	MAKE_TC(k_args00, false, k_files00)
	MAKE_TC(k_args01, true, k_files01)

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
	IndentClassifier app(makeArgsSpan(tc));
	BOOST_CHECK_EQUAL(tc.m_isRecursive, app.m_fileEnumerator.isRecursive());
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
	char const*							m_input;
	size_t									m_numSpaceLines;
	size_t									m_numTabLines;
	size_t									m_numMixedLines;
	size_t									m_numIndLines;
	IndentClassifier::IndentType	m_iType;
};

static TestCase const k_testCases[] =
{
	// input, numSpaceLines, numTabLines, numMixedLines, numIndLines, iType
	{
		"",
		0, 0, 0, 1, IndentClassifier::IndentType::INDETERMINATE
	},
	{
		"This one has no whitespace at the beginning of the only line.",
		0, 0, 0, 1, IndentClassifier::IndentType::INDETERMINATE
	},
	{
		"This one has no whitespace\n"
		"at the beginning\n"
		"of any of the lines.\n",
		0, 0, 0, 4, IndentClassifier::IndentType::INDETERMINATE
	},
	{
		"   This one has spaces\n"
		"      at the beginning\n"
		" of all of the lines.\n",
		3, 0, 0, 1, IndentClassifier::IndentType::SPACE
	},
	{
		"foo\n"
		"   This one has spaces\n"
		"      at the beginning\n"
		" of all of the lines.\n",
		3, 0, 0, 2, IndentClassifier::IndentType::SPACE
	},
	{
		"\tThis one has tabs\n"
		"\t\t\tat the beginning\n"
		"\tof all of the lines.\n",
		0, 3, 0, 1, IndentClassifier::IndentType::TAB
	},
	{
		"foo\n"
		"\tThis one has tabs\n"
		"\t\t\tat the beginning\n"
		"\tof all of the lines.\n",
		0, 3, 0, 2, IndentClassifier::IndentType::TAB
	},
	{
		"\tThis one has tabs\n"
		"   on some lines\n"
		"\t\tand spaces on others.\n",
		1, 2, 0, 1, IndentClassifier::IndentType::MIXED
	},
	{
		" \tThis one has tabs\n"
		" \t\t and spaces\n"
		"\t   \ton each line.\n",
		0, 0, 3, 1, IndentClassifier::IndentType::MIXED
	},
	{
		"This one has no whitespace\n"
		"at the beginning\n"
		"of any of the lines.\n"
		"\tThis one has tabs\n"
		"   on some lines\n"
		"\t\tand spaces on others.\n"
		" \tThis one has tabs\n"
		" \t\t and spaces\n"
		"\t   \ton each line.\n",
		1, 2, 3, 4, IndentClassifier::IndentType::MIXED
	}
};

static ::std::ostream& operator<<(::std::ostream& ostrm, TestCase const& tc)
{
	return ostrm << "Test case #" << (&tc - k_testCases);
}

BOOST_DATA_TEST_CASE(scanFileTest, utd::make(k_testCases), tc)
{
	string input(tc.m_input);
	if (true)
	{
		BOOST_TEST_MESSAGE(input);
	}
	size_t numSpaceLines;
	size_t numTabLines;
	size_t numMixedLines;
	size_t numIndLines;
	size_t totalEols;
	{
		istringstream in(input);
		IndentClassifier::IndentType iType = IndentClassifier::IndentType::MIXED;
		BOOST_CHECK_NO_THROW(
			iType = IndentClassifier::scanFile(in, numSpaceLines, numTabLines, numMixedLines,
				numIndLines, totalEols));
		BOOST_CHECK_EQUAL(tc.m_numSpaceLines, numSpaceLines);
		BOOST_CHECK_EQUAL(tc.m_numTabLines, numTabLines);
		BOOST_CHECK_EQUAL(tc.m_numMixedLines, numMixedLines);
		BOOST_CHECK_EQUAL(tc.m_numIndLines, numIndLines);
		BOOST_CHECK_EQUAL(
			tc.m_numSpaceLines + tc.m_numTabLines + tc.m_numMixedLines + tc.m_numIndLines,
			totalEols);
		BOOST_CHECK_EQUAL(static_cast<int>(tc.m_iType), static_cast<int>(iType));
	}
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()
