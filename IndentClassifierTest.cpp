
#if !defined(CMDLINEUTIL_TEST_MODE)
#define CMDLINEUTIL_TEST_MODE
#endif

#include "Exceptions.h"
#include "IndentClassifier.h"
#include "PathDeleter.h"
#include "TestUtil.h"
#include "Utils.h"

#include <boost/range/algorithm_ext/for_each.hpp>
#include <boost/range/algorithm/sort.hpp>
#include <boost/test/unit_test.hpp>
#include <boost/test/data/test_case.hpp>
#include <numeric>
#include <string>
#include <sstream>
#include <vector>

namespace b = ::boost;
namespace fs = ::std::filesystem;
namespace utd = ::boost::unit_test::data;

using ::std::begin;
using ::std::end;
using ::std::istringstream;
using ::std::string;

using PathList = ::std::vector<fs::path>;



BOOST_AUTO_TEST_SUITE(IndentClassifierTestSuite)

BOOST_AUTO_TEST_SUITE(CmdLineParseFailTestSuite)

static char const*const k_args00[] = { "indents" };
static char const*const k_args01[] = { "indents", "-?" };
static char const*const k_args02[] = { "indents", "-h" };
static char const*const k_args03[] = { "indents", "-help" };
static char const*const k_args04[] = { "indents", "-r" };

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
	BOOST_CHECK_EXCEPTION(IndentClassifier(tc.m_args), CmdLineError,
		[&tc](const CmdLineError& ex) { return tc.doesExMatch(ex); });
}

BOOST_AUTO_TEST_SUITE_END()



BOOST_AUTO_TEST_SUITE(CmdLineParseOkTestSuite)

struct CmdLineParseOkTestCase : public CmdLineParseTestCase
{
	template<::std::size_t N, ::std::size_t M>
	CmdLineParseOkTestCase(const char*const(&args)[N], bool isRecursive,
			const char*const(&fileList)[M]) noexcept :
		CmdLineParseTestCase(args),
		m_isRecursive(isRecursive),
		m_fileList(::gsl::make_span(fileList, M))
		{}

	bool			m_isRecursive;
	ArgListPair	m_fileList;
};

static char const*const k_args00[] = { "indents", "IndentClassifier.cpp" };
static char const*const k_args01[] = { "indents", "-r", "IndentClassifier*.h", "IndentClassifier*.cpp" };

static char const*const k_files00[] = { "IndentClassifier.cpp" };
static char const*const k_files01[] = { "IndentClassifier*.h", "IndentClassifier*.cpp" };

static CmdLineParseOkTestCase const k_testCases[] =
{
	{ k_args00, false, k_files00 },
	{ k_args01, true, k_files01 },
};

static void checkEqual(const fs::path& tcPath, const fs::path& appPath)
{
	BOOST_CHECK_EQUAL(tcPath.generic_string(), appPath.generic_string());
}

BOOST_DATA_TEST_CASE(cmdLineParseOkTest, utd::make(k_testCases), tc)
{
	IndentClassifier app(tc.m_args);
	BOOST_CHECK_EQUAL(tc.m_isRecursive, app.m_fileEnumerator.isRecursive());
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
	char const*const	m_pInput;
	bool					m_isJavaFile;
	size_t				m_numSpaceLines;
	size_t				m_numTabLines;
	size_t				m_numJavadocTabLines;
	size_t				m_numJavadocLeftLines;
	size_t				m_numMixedLines;
	size_t				m_numIndLines;
	IndentType			m_fileType;
};

static ScanFileTestCase const k_testCases[] =
{
	// input, numSpaceLines, numTabLines, numMixedLines, numIndLines, fileType
	{
		"",
		false, 0, 0, 0, 0, 0, 1, IndentType::indeterminate
	},
	{
		"This one has no whitespace at the beginning of the only line.",
		false, 0, 0, 0, 0, 0, 1, IndentType::indeterminate
	},
	{
		"This one has no whitespace\n"
		"at the beginning\n"
		"of any of the lines.\n",
		false, 0, 0, 0, 0, 0, 4, IndentType::indeterminate
	},
	{
		"   \n"
		"   This one has spaces\n"
		"      at the beginning\n"
		" of all of the lines.\n",
		false, 4, 0, 0, 0, 0, 1, IndentType::space
	},
	{
		"foo\n"
		"   This one has spaces\n"
		"      at the beginning\n"
		" of all of the lines but one.\n",
		false, 3, 0, 0, 0, 0, 2, IndentType::space
	},
	{
		"\tThis one has tabs\n"
		"\t\t\tat the beginning\n"
		"\tof all of the lines.\n",
		false, 0, 3, 0, 0, 0, 1, IndentType::tab
	},
	{
		"foo\n"
		"\tThis one has tabs\n"
		"\t\t\tat the beginning\n"
		"\tof all of the lines.\n",
		false, 0, 3, 0, 0, 0, 2, IndentType::tab
	},
	{
		"\tThis one has tabs\n"
		"   on some lines\n"
		"\t\tand spaces on others.\n",
		false, 1, 2, 0, 0, 0, 1, IndentType::mixed
	},
	{
		" \tThis one has tabs\n"
		" \t\t and spaces\n"
		"\t   \ton each line.\n",
		false, 0, 0, 0, 0, 3, 1, IndentType::mixed
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
		false, 1, 2, 0, 0, 3, 4, IndentType::mixed
	},
	{
		"/**\n"
		" * A comment\n"
		" */\n"
		"public interface Foo {\n"
		"}",
		false, 2, 0, 0, 0, 0, 3, IndentType::space
	},
	{
		"/**\n"
		" * A comment\n"
		" */\n"
		"public interface Foo {\n"
		"}",
		true, 0, 0, 0, 2, 0, 3, IndentType::javadocTab
	},
	{
		"/**\n"
		" * A comment\n"
		" */\n"
		"public interface Foo {\n"
		"\tvoid aMethod();\n"
		"}",
		true, 0, 1, 0, 2, 0, 3, IndentType::javadocTab
	},
	{
		"/**\n"
		" * A comment\n"
		" */\n"
		"public interface Foo {\n"
		"\t/**\n"
		"\t * Another comment\n"
		"\t */\n"
		"\tvoid aMethod();\n"
		"}",
		true, 0, 2, 2, 2, 0, 3, IndentType::javadocTab
	}
};

static ::std::ostream& operator<<(::std::ostream& ostrm, const ScanFileTestCase& tc)
{
	return ostrm << "Test case with input \"" << tc.m_pInput << "\"";
}

BOOST_DATA_TEST_CASE(scanFileTest, utd::make(k_testCases), tc)
{
	string input(tc.m_pInput);
	if (false)
	{
		BOOST_TEST_MESSAGE(input);
	}

	istringstream in(input);
	auto lineTypeCounts{IndentClassifier::scanFile(in, tc.m_isJavaFile)};
	auto fileType{IndentClassifier::classifyFile(lineTypeCounts)};
	BOOST_CHECK_EQUAL(tc.m_numSpaceLines, get(lineTypeCounts, IndentType::space));
	BOOST_CHECK_EQUAL(tc.m_numTabLines, get(lineTypeCounts, IndentType::tab));
	BOOST_CHECK_EQUAL(tc.m_numMixedLines, get(lineTypeCounts, IndentType::mixed));
	BOOST_CHECK_EQUAL(tc.m_numIndLines, get(lineTypeCounts, IndentType::indeterminate));

	auto totalLines{::std::accumulate(begin(lineTypeCounts), end(lineTypeCounts), size_t{0},
		[](size_t value, const LineTypeCounts::value_type& mapEntry){ return value + mapEntry.second; })};
	BOOST_CHECK_EQUAL(
		tc.m_numSpaceLines + tc.m_numTabLines + tc.m_numJavadocTabLines
			+ tc.m_numJavadocLeftLines + tc.m_numMixedLines + tc.m_numIndLines,
		totalLines);
	BOOST_CHECK_EQUAL(static_cast<int>(tc.m_fileType), static_cast<int>(fileType));
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()
