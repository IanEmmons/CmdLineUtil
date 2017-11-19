
#if !defined(CMDLINEUTIL_TEST_MODE)
#define CMDLINEUTIL_TEST_MODE
#endif

#include "FileEnumerator.h"
#include "PathDeleter.h"
#include "Utils.h"

#include <boost/range/algorithm_ext/for_each.hpp>
#include <boost/range/algorithm/sort.hpp>
#include <boost/test/unit_test.hpp>
#include <functional>

namespace b = ::boost;
namespace bfs = ::boost::filesystem;

BOOST_AUTO_TEST_SUITE(FileEnumeratorTestSuite)

static char const*const k_matchList1[] = { "./FileEnumerator.cpp" };
static char const*const k_matchList2[] = { "./FileEnumerator.cpp", "./FileEnumeratorTest.cpp" };
static char const*const k_matchList3[] = { "./FileEnumerator.cpp", "./FileEnumerator.h",
	"./TempTestDir/FileEnumerator.cpp", "./TempTestDir/FileEnumerator.h" };
static char const*const k_matchList4[] = { "./FileEnumerator.cpp", "./FileEnumerator.h",
	"./FileEnumeratorTest.cpp", "./TempTestDir/FileEnumerator.cpp",
	"./TempTestDir/FileEnumerator.h", "./TempTestDir/FileEnumeratorTest.cpp" };

static void copyFile(const bfs::path& fromDir, const bfs::path& toDir, const char* pFileName)
{
	copy_file(fromDir / pFileName, toDir / pFileName);
}

static void checkEqual(char const* tcPath, const bfs::path& appPath)
{
	BOOST_CHECK_EQUAL(tcPath, appPath.generic_string());
}

class PathListInserter : public ::std::unary_function<bfs::path, void>
{
public:
	PathListInserter(FileEnumerator::PathList& pathList)
		: m_pPathList(&pathList) {}
	void operator()(const bfs::path& path) const
		{ m_pPathList->push_back(path); }

private:
	// Note that PathListInserter instances do not own their path list!!!
	FileEnumerator::PathList* m_pPathList;
};

BOOST_AUTO_TEST_CASE(ExpandWildCardTest)
{
	// Setup:
	bfs::path cwDir(".");
	bfs::path testDir = cwDir / "TempTestDir";
	PathDeleter testPathDeleter(testDir);
	create_directories(testDir);
	copyFile(cwDir, testDir, "FileEnumerator.cpp");
	copyFile(cwDir, testDir, "FileEnumerator.h");
	copyFile(cwDir, testDir, "FileEnumeratorTest.cpp");

	// Non-recursive, non-wildcard:
	{
		FileEnumerator fe;
		fe.insert("FileEnumerator.cpp");

		FileEnumerator::PathList fileList;
		fe.enumerateFiles(PathListInserter(fileList));

		BOOST_CHECK_EQUAL(arrayLen(k_matchList1), fileList.size());
		b::sort(fileList);
		b::for_each(b::make_iterator_range(k_matchList1), fileList, checkEqual);
	}

	// Non-recursive, wildcard:
	{
		FileEnumerator fe;
		fe.insert("FileEn*.cpp");

		FileEnumerator::PathList fileList;
		fe.enumerateFiles(PathListInserter(fileList));

		BOOST_CHECK_EQUAL(arrayLen(k_matchList2), fileList.size());
		b::sort(fileList);
		b::for_each(b::make_iterator_range(k_matchList2), fileList, checkEqual);
	}

	// Recursive, non-wildcard:
	{
		FileEnumerator fe;
		fe.setRecursive();
		fe.insert("FileEnumerator.cpp");
		fe.insert("FileEnumerator.h");

		FileEnumerator::PathList fileList;
		fe.enumerateFiles(PathListInserter(fileList));

		BOOST_CHECK_EQUAL(arrayLen(k_matchList3), fileList.size());
		b::sort(fileList);
		b::for_each(b::make_iterator_range(k_matchList3), fileList, checkEqual);
	}

	// Recursive, wildcard:
	{
		FileEnumerator fe;
		fe.setRecursive();
		fe.insert("FileEn*.cpp");
		fe.insert("FileEn*.h");

		FileEnumerator::PathList fileList;
		fe.enumerateFiles(PathListInserter(fileList));

		BOOST_CHECK_EQUAL(arrayLen(k_matchList4), fileList.size());
		b::sort(fileList);
		b::for_each(b::make_iterator_range(k_matchList4), fileList, checkEqual);
	}
}

BOOST_AUTO_TEST_SUITE_END()
