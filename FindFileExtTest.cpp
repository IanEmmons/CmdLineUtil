
#if !defined(CMDLINEUTIL_TEST_MODE)
#define CMDLINEUTIL_TEST_MODE
#endif

#include "FindFileExt.h"
#include "PathDeleter.h"
#include "TestUtil.h"
#include "Utils.h"

#include <boost/range/algorithm_ext/for_each.hpp>
#include <boost/range/algorithm/sort.hpp>
#include <boost/test/unit_test.hpp>
#include <boost/test/data/test_case.hpp>
#include <string>
#include <vector>

namespace b = ::boost;
namespace bfs = ::boost::filesystem;
namespace utd = ::boost::unit_test::data;

using ::std::string;

using PathList = ::std::vector<bfs::path>;

BOOST_AUTO_TEST_SUITE(FindFileExtTestSuite)

BOOST_AUTO_TEST_SUITE(CmdLineParseFailTestSuite)

static char const*const k_args00[] = { "findext" };
static char const*const k_args01[] = { "findext", "-?" };
static char const*const k_args02[] = { "findext", "-h" };
static char const*const k_args03[] = { "findext", "-help" };
static char const*const k_args04[] = { "findext", "-r" };
static char const*const k_args05[] = { "findext", "-d" };
static char const*const k_args06[] = { "findext", "fooble" };
static char const*const k_args07[] = { "findext", "FindFileExt.cpp" };

static CmdLineParseFailTestCase const k_testCases[] =
{
	{ k_args00, "^No directories specified$" },
	{ k_args01, "^$" },
	{ k_args02, "^$" },
	{ k_args03, "^$" },
	{ k_args04, "^No directories specified$" },
	{ k_args05, "^.* does not exist" },
	{ k_args06, "^.* does not exist$" },
	{ k_args07, "^.* is not a directory$" },
};

BOOST_DATA_TEST_CASE(cmdLineParseFailTest, utd::make(k_testCases), tc)
{
	BOOST_CHECK_EXCEPTION(FindFileExt(tc.makeArgSpan()), CmdLineError,
		[&tc](const CmdLineError& ex) { return tc.doesExMatch(ex); });
}

BOOST_AUTO_TEST_SUITE_END()



BOOST_AUTO_TEST_SUITE(CmdLineParseOkTestSuite)

struct CmdLineParseOkTestCase : public CmdLineParseTestCase
{
	template<::std::size_t N, ::std::size_t M>
	CmdLineParseOkTestCase(const char*const(&args)[N], bool isRecursive,
			bool includeCounts, bool outputAsWildcards,
			const char*const(&fileList)[M]) noexcept :
		CmdLineParseTestCase(args),
		m_isRecursive(isRecursive),
		m_includeCounts(includeCounts),
		m_outputAsWildcards(outputAsWildcards),
		m_fileList(::std::make_pair(fileList, M))
		{}

	bool			m_isRecursive;
	bool			m_includeCounts;
	bool			m_outputAsWildcards;
	ArgListPair	m_fileList;
};

static char const*const k_args00[] = { "findext", "." };
static char const*const k_args01[] = { "findext", "-r", "." };
static char const*const k_args02[] = { "findext", "-c", "." };
static char const*const k_args03[] = { "findext", "-w", "." };
static char const*const k_args04[] = { "findext", "-r", "-r", "." };
static char const*const k_args05[] = { "findext", "-r", "-c", "." };
static char const*const k_args06[] = { "findext", "-c", "-r", ".", "bb-bin" };
static char const*const k_args07[] = { "findext", "-r", "-w", "." };
static char const*const k_args08[] = { "findext", "-c", "-w", "." };
static char const*const k_args09[] = { "findext", "-c", "-r", "-w", "." };
static char const*const k_args10[] = { "findext", "-r", ".", "bb-bin" };

static char const*const k_files00[] = { "./*" };
static char const*const k_files01[] = { "./*", "bb-bin/*" };

static CmdLineParseOkTestCase const k_testCases[] =
{
	{ k_args00, false, false, false, k_files00 },
	{ k_args01, true,  false, false, k_files00 },
	{ k_args02, false, true,  false, k_files00 },
	{ k_args03, false, false, true,  k_files00 },
	{ k_args04, true,  false, false, k_files00 },
	{ k_args05, true,  true,  false, k_files00 },
	{ k_args06, true,  true,  false, k_files01 },
	{ k_args07, true,  false, true,  k_files00 },
	{ k_args08, false, true,  true,  k_files00 },
	{ k_args09, true,  true,  true,  k_files00 },
	{ k_args10, true,  false, false, k_files01 },
};

static void checkEqual(const bfs::path& tcPath, const bfs::path& appPath)
{
	BOOST_CHECK_EQUAL(tcPath.generic_string(), appPath.generic_string());
}

BOOST_DATA_TEST_CASE(cmdLineParseOkTest, utd::make(k_testCases), tc)
{
	FindFileExt app(tc.makeArgSpan());
	BOOST_CHECK_EQUAL(tc.m_isRecursive, app.m_fileEnumerator.isRecursive());
	BOOST_CHECK_EQUAL(tc.m_includeCounts, app.m_includeCounts);
	BOOST_CHECK_EQUAL(tc.m_outputAsWildcards, app.m_outputAsWildcards);
	BOOST_CHECK_EQUAL(tc.m_fileList.second, app.m_fileEnumerator.numFileSpecs());

	PathList tcList(tc.m_fileList.first, tc.m_fileList.first + tc.m_fileList.second);
	b::sort(tcList);

	PathList appList;
	app.m_fileEnumerator.getFileSpecList(appList);
	b::sort(appList);

	b::for_each(tcList, appList, checkEqual);
}

BOOST_AUTO_TEST_SUITE_END()



BOOST_AUTO_TEST_SUITE(CountFilesTestSuite)

struct CountFilesTestCase : public CmdLineParseTestCase
{
	template<::std::size_t N>
	CountFilesTestCase(const char*const(&args)[N], bool isRecursive,
		::std::size_t hCount, ::std::size_t cppCount) noexcept :
		CmdLineParseTestCase(args),
		m_isRecursive(isRecursive),
		m_hCount(hCount),
		m_cppCount(cppCount)
		{}

	bool				m_isRecursive;
	::std::size_t	m_hCount;
	::std::size_t	m_cppCount;
};

static char const*const k_args00[] = { "findext", "TempTestDir" };
static char const*const k_args01[] = { "findext", "-r", "TempTestDir" };

static CountFilesTestCase const k_testCases[] =
{
	{ k_args00, false, 1, 2 },
	{ k_args01, true,  2, 4 },
};

static void copyFile(const bfs::path& fromDir, const bfs::path& toDir, const char* pFileName)
{
	copy_file(fromDir / pFileName, toDir / pFileName);
}

static void checkEqual(const FindFileExt::StrToCountMap::value_type& expMapEntry,
	const FindFileExt::StrToCountMap::value_type& obsMapEntry)
{
	BOOST_CHECK_EQUAL(expMapEntry.first, obsMapEntry.first);
	BOOST_CHECK_EQUAL(expMapEntry.second, obsMapEntry.second);
}

BOOST_DATA_TEST_CASE(countFilesTest, utd::make(k_testCases), tc)
{
	// Setup:
	bfs::path cwDir(".");
	bfs::path testDir = cwDir / "TempTestDir";
	bfs::path testDir2 = testDir / "dir2";
	PathDeleter testPathDeleter(testDir);
	create_directories(testDir2);
	copyFile(cwDir, testDir, "FileEnumerator.cpp");
	copyFile(cwDir, testDir, "FileEnumerator.h");
	copyFile(cwDir, testDir, "FileEnumeratorTest.cpp");
	copyFile(cwDir, testDir2, "FileEnumerator.cpp");
	copyFile(cwDir, testDir2, "FileEnumerator.h");
	copyFile(cwDir, testDir2, "FileEnumeratorTest.cpp");
	testDir.make_preferred();

	FindFileExt app(tc.makeArgSpan());
	BOOST_CHECK_EQUAL(tc.m_isRecursive, app.m_fileEnumerator.isRecursive());

	app.countFiles();

	FindFileExt::StrToCountMap expExtToCountMap;	// expected result
	expExtToCountMap.insert(make_pair(string(".cpp"), tc.m_cppCount));
	expExtToCountMap.insert(make_pair(string(".h"), tc.m_hCount));

	BOOST_CHECK_EQUAL(expExtToCountMap.size(), app.m_extToCountMap.size());
	b::for_each(expExtToCountMap, app.m_extToCountMap, checkEqual);
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()
