
#if !defined(CMDLINEUTIL_TEST_MODE)
#define CMDLINEUTIL_TEST_MODE
#endif

#include "FindFileExt.h"
#include "PathDeleter.h"
#include "Utils.h"
#include <boost/range/algorithm_ext/for_each.hpp>
#include <boost/test/unit_test.hpp>

namespace b = ::boost;
namespace bfs = ::boost::filesystem;

using ::std::pair;
using ::std::string;

BOOST_AUTO_TEST_SUITE(FindFileExtTestSuite)

static void copyFile(const bfs::path& fromDir, const bfs::path& toDir, const char* pFileName)
{
	copy_file(fromDir / pFileName, toDir / pFileName);
}

static void checkEqual(const pair<string, size_t>& expMapEntry,
	const pair<string, size_t>& obsMapEntry)
{
	BOOST_CHECK_EQUAL(expMapEntry.first, obsMapEntry.first);
	BOOST_CHECK_EQUAL(expMapEntry.second, obsMapEntry.second);
}

BOOST_AUTO_TEST_CASE(FileExtCompilationTest)
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

	// Non-recursive:
	{
		const char* args[] = { "findext", nullptr };
		string testDirStr = testDir.string();
		args[arrayLen(args) - 1] = testDirStr.c_str();
		FindFileExt app(arrayLen(args), args);
		app.countFiles();

		FindFileExt::StrToCountMap expExtToCountMap;	// expected result
		expExtToCountMap.insert(make_pair(string(".cpp"), 2));
		expExtToCountMap.insert(make_pair(string(".h"), 1));

		BOOST_CHECK_EQUAL(expExtToCountMap.size(), app.m_extToCountMap.size());
		b::for_each(expExtToCountMap, app.m_extToCountMap, checkEqual);
	}

	// Recursive:
	{
		const char* args[] = { "findext", "-r", nullptr };
		string testDirStr = testDir.string();
		args[arrayLen(args) - 1] = testDirStr.c_str();
		FindFileExt app(arrayLen(args), args);
		app.countFiles();

		FindFileExt::StrToCountMap expExtToCountMap;	// expected result
		expExtToCountMap.insert(make_pair(string(".cpp"), 4));
		expExtToCountMap.insert(make_pair(string(".h"), 2));

		BOOST_CHECK_EQUAL(expExtToCountMap.size(), app.m_extToCountMap.size());
		b::for_each(expExtToCountMap, app.m_extToCountMap, checkEqual);
	}
}

BOOST_AUTO_TEST_SUITE_END()
