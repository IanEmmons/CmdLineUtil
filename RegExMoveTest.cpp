
#if !defined(CMDLINEUTIL_TEST_MODE)
#define CMDLINEUTIL_TEST_MODE
#endif

#include "PathDeleter.h"
#include "TestUtil.h"
#include "RegExMove.h"

#include <algorithm>
#include <array>
#include <boost/test/data/test_case.hpp>
#include <boost/test/unit_test.hpp>
#include <filesystem>
#include <fstream>
#include <string>
#include <string_view>
#include <vector>

using namespace ::std::literals;

namespace utd = ::boost::unit_test::data;
namespace fs = ::std::filesystem;

using ::std::array;
using ::std::begin;
using ::std::end;
using ::std::endl;
using ::std::ofstream;
using ::std::sort;
using ::std::string;
using ::std::string_view;
using ::std::vector;

BOOST_AUTO_TEST_SUITE(RegExMoveTestSuite)

BOOST_AUTO_TEST_SUITE(CmdLineParseFailTestSuite)

static char const*const k_args00[] = { "regexmv" };
static char const*const k_args01[] = { "regexmv", "-?" };
static char const*const k_args02[] = { "regexmv", "-h" };
static char const*const k_args03[] = { "regexmv", "-help" };
static char const*const k_args04[] = { "regexmv", "-a" };
static char const*const k_args05[] = { "regexmv", "." };
static char const*const k_args06[] = { "regexmv", ".", "foo" };
static char const*const k_args07[] = { "regexmv", "foo", "bar", "baz" };
static char const*const k_args08[] = { "regexmv", "Xeol.h", "bar", "baz" };
static char const*const k_args09[] = { "regexmv", ".", "*", "baz" };
static char const*const k_args10[] = { "regexmv", "-i", ".", "*", "baz" };

static CmdLineParseFailTestCase const k_testCases[] =
{
	{ k_args00, "^All of the arguments .* are required$" },
	{ k_args01, "^$" },
	{ k_args02, "^$" },
	{ k_args03, "^$" },
	{ k_args04, "^All of the arguments .* are required$" },
	{ k_args05, "^All of the arguments .* are required$" },
	{ k_args06, "^All of the arguments .* are required$" },
	{ k_args07, "^The directory 'foo' does not exist$" },
	{ k_args08, "^'Xeol.h' is not a directory$" },
	{ k_args09, "^'.*' is not a valid regular expression \\(.*\\)$" },
	{ k_args10, "^'.*' is not a valid regular expression \\(.*\\)$" },
};

BOOST_DATA_TEST_CASE(cmdLineParseFailTest, utd::make(k_testCases), tc)
{
	BOOST_CHECK_EXCEPTION(RegExMove(tc.m_args), CmdLineError,
		[&tc](const CmdLineError& ex) { return tc.doesExMatch(ex); });
}

BOOST_AUTO_TEST_SUITE_END()



BOOST_AUTO_TEST_SUITE(CmdLineParseOkTestSuite)

struct CmdLineParseOkTestCase : public CmdLineParseTestCase
{
	template<::std::size_t N>
	CmdLineParseOkTestCase(const char*const(&args)[N], bool caseSensitive,
			bool renameDirectories, bool renameFiles, bool recursiveSearch,
			bool verboseOutput, bool allowOverwriteOnNameCollision,
			char const* pRootDir, char const* pPattern,
			char const* pReplacement) noexcept :
		CmdLineParseTestCase(args),
		m_caseSensitive(caseSensitive),
		m_renameDirectories(renameDirectories),
		m_renameFiles(renameFiles),
		m_recursiveSearch(recursiveSearch),
		m_verboseOutput(verboseOutput),
		m_allowOverwriteOnNameCollision(allowOverwriteOnNameCollision),
		m_rootDir(pRootDir),
		m_pattern(pPattern),
		m_replacement(pReplacement)
		{}

	bool	m_caseSensitive;
	bool	m_renameDirectories;
	bool	m_renameFiles;
	bool	m_recursiveSearch;
	bool	m_verboseOutput;
	bool	m_allowOverwriteOnNameCollision;
	string	m_rootDir;
	string	m_pattern;
	string	m_replacement;
};

static char const*const k_args00[] = { "regexmv", ".", "foo-([0-9]+)\\.txt", "bar-\\1.txt" };
static char const*const k_args01[] = { "regexmv", "-i", ".", "^foo-([0-9]+)\\.txt$", "bar-\\1.txt" };
static char const*const k_args02[] = { "regexmv", "-d", "-d", ".", "bar", "baz" };
static char const*const k_args03[] = { "regexmv", "-dd", ".", "bar", "baz" };
static char const*const k_args04[] = { "regexmv", "-i", "-r", ".", "bar", "baz" };
static char const*const k_args05[] = { "regexmv", "-r", "-v", ".", "bar", "baz" };
static char const*const k_args06[] = { "regexmv", "-i", "-r", "-v", "-y", ".", "bar", "baz" };
static char const*const k_args07[] = { "regexmv", "-dd", "-r", "-v", "-y", ".", "bar", "baz" };

static CmdLineParseOkTestCase const k_testCases[] =
{
	{ k_args00, true,  false, true,  false, false, false, ".", "^foo-([0-9]+)\\.txt$", "bar-\\1.txt" },
	{ k_args01, false, false, true,  false, false, false, ".", "^foo-([0-9]+)\\.txt$", "bar-\\1.txt" },
	{ k_args02, true,  true,  true,  false, false, false, ".", "^bar$", "baz" },
	{ k_args03, true,  true,  false, false, false, false, ".", "^bar$", "baz" },
	{ k_args04, false, false, true,  true,  false, false, ".", "^bar$", "baz" },
	{ k_args05, true,  false, true,  true,  true,  false, ".", "^bar$", "baz" },
	{ k_args06, false, false, true,  true,  true,  true,  ".", "^bar$", "baz" },
	{ k_args07, true,  true,  false, true,  true,  true,  ".", "^bar$", "baz" },
};

BOOST_DATA_TEST_CASE(cmdLineParseOkTest, utd::make(k_testCases), tc)
{
	RegExMove app(tc.m_args);
	BOOST_CHECK_EQUAL(tc.m_caseSensitive, app.m_caseSensitive);
	BOOST_CHECK_EQUAL(tc.m_renameDirectories, app.m_renameDirectories);
	BOOST_CHECK_EQUAL(tc.m_renameFiles, app.m_renameFiles);
	BOOST_CHECK_EQUAL(tc.m_recursiveSearch, app.m_recursiveSearch);
	BOOST_CHECK_EQUAL(tc.m_verboseOutput, app.m_verboseOutput);
	BOOST_CHECK_EQUAL(tc.m_allowOverwriteOnNameCollision, app.m_allowOverwriteOnNameCollision);
	BOOST_CHECK_EQUAL(tc.m_rootDir, app.m_rootDir);
	BOOST_CHECK_EQUAL(tc.m_pattern, app.m_patternStr);
	BOOST_CHECK_EQUAL(tc.m_replacement, app.m_replacement);
}

BOOST_AUTO_TEST_SUITE_END()



BOOST_AUTO_TEST_SUITE(FileMoveTestSuite)

static constexpr array k_testCases =
{
	"regexmvtest/file0.tex"sv,
	"regexmvtest/file0.txt"sv,
	"regexmvtest/file1.txt"sv,
	"regexmvtest/file2.txt"sv,
	"regexmvtest/File3.txt"sv,
	"regexmvtest/file4.txt"sv,
	"regexmvtest/subdir/File5.tex"sv,
	"regexmvtest/subdir/file5.txt"sv,
	"regexmvtest/subdir/file6.txt"sv,
	"regexmvtest/subdir/file7.txt"sv,
	"regexmvtest/subdir/file8.txt"sv,
	"regexmvtest/file9.txt"sv,
	"regexmvtest/file10.txt"sv,
	"regexmvtest/file11.txt"sv,
	"regexmvtest/file12.txt"sv,
};

static constexpr string_view k_expectedResults = R"(
regexmvtest/file0.tex
regexmvtest/file00.txt
regexmvtest/file01.txt
regexmvtest/file02.txt
regexmvtest/file03.txt
regexmvtest/file04.txt
regexmvtest/file09.txt
regexmvtest/file10.txt
regexmvtest/file11.txt
regexmvtest/file12.txt
regexmvtest/subdir
regexmvtest/subdir/File5.tex
regexmvtest/subdir/file05.txt
regexmvtest/subdir/file06.txt
regexmvtest/subdir/file07.txt
regexmvtest/subdir/file08.txt
)";

static char const*const k_cmdLineArgs[] = { "-i", "-r", "regexmvtest", "file([0-9])\\.txt", "file0$1.txt" };

BOOST_AUTO_TEST_CASE(fileMoveTest)
{
	// Assumes top-level dir of all test cases is the same:
	auto firstTC = k_testCases.front();
	auto firstSlashPos = firstTC.find('/');
	BOOST_CHECK(firstSlashPos != string_view::npos);
	auto rootDir = fs::path{firstTC.substr(0, firstSlashPos)};
	PathDeleter deleter(rootDir);

	for (auto const& tc : k_testCases)
	{
		auto tcPath = fs::path{tc};
		create_directories(tcPath.parent_path());
		auto stream = ofstream{tcPath};
		stream << tc << endl;
	}

	RegExMove app(k_cmdLineArgs);
	app.run();

	auto firstEntry = fs::recursive_directory_iterator{rootDir};
	auto lastEntry = fs::recursive_directory_iterator{};
	vector<string> actualResultsVec;
	for (auto it = firstEntry; it != lastEntry; ++it)
	{
		auto pathStr = it->path().generic_string();
		actualResultsVec.push_back(pathStr);
		BOOST_TEST_MESSAGE("Found file: " << pathStr);
	}
	sort(begin(actualResultsVec), end(actualResultsVec));
	auto actualResults = string{"\n"};
	for (auto const& result : actualResultsVec)
	{
		actualResults += result;
		actualResults += "\n";
	}
	BOOST_CHECK_EQUAL(k_expectedResults, actualResults);
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()
