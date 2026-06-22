
#if !defined(CMDLINEUTIL_TEST_MODE)
#define CMDLINEUTIL_TEST_MODE
#endif

#include "FileEnumerator.h"
#include "PathDeleter.h"
#include "Utils.h"

#include <boost/range/algorithm_ext/for_each.hpp>
#include <boost/range/algorithm/sort.hpp>
#include <boost/test/unit_test.hpp>
#include <string_view>

namespace b = ::boost;
namespace fs = ::std::filesystem;

using ::std::string_view;

BOOST_AUTO_TEST_SUITE(FileEnumeratorTestSuite)

static constexpr string_view k_matchList1[] = { "./FileEnumerator.cpp" };
static constexpr string_view k_matchList2[] = { "./FileEnumerator.cpp", "./FileEnumeratorTest.cpp" };
static constexpr string_view k_matchList3[] = { "./FileEnumerator.cpp", "./FileEnumerator.h",
	"./TempTestDir/FileEnumerator.cpp", "./TempTestDir/FileEnumerator.h" };
static constexpr string_view k_matchList4[] = { "./FileEnumerator.cpp", "./FileEnumerator.h",
	"./FileEnumeratorTest.cpp", "./TempTestDir/FileEnumerator.cpp",
	"./TempTestDir/FileEnumerator.h", "./TempTestDir/FileEnumeratorTest.cpp" };

static void copyFile(const fs::path& fromDir, const fs::path& toDir, string_view fileName)
{
	copy_file(fromDir / fileName, toDir / fileName);
}

static void checkEqual(string_view tcPath, const fs::path& appPath)
{
	BOOST_CHECK_EQUAL(tcPath, appPath.generic_string());
}

BOOST_AUTO_TEST_CASE(ExpandWildCardTest)
{
	// Setup:
	fs::path cwDir(".");
	fs::path testDir = cwDir / "TempTestDir";
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
		fe.enumerateFiles([&fileList] (const fs::path& path) { fileList.push_back(path); });

		BOOST_CHECK_EQUAL(arrayLen(k_matchList1), fileList.size());
		b::sort(fileList);
		b::for_each(b::make_iterator_range(k_matchList1), fileList, checkEqual);
	}

	// Non-recursive, wildcard:
	{
		FileEnumerator fe;
		fe.insert("FileEn*.cpp");

		FileEnumerator::PathList fileList;
		fe.enumerateFiles([&fileList] (const fs::path& path) { fileList.push_back(path); });

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
		fe.enumerateFiles([&fileList] (const fs::path& path) { fileList.push_back(path); });

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
		fe.enumerateFiles([&fileList] (const fs::path& path) { fileList.push_back(path); });

		BOOST_CHECK_EQUAL(arrayLen(k_matchList4), fileList.size());
		b::sort(fileList);
		b::for_each(b::make_iterator_range(k_matchList4), fileList, checkEqual);
	}
}

BOOST_AUTO_TEST_SUITE_END()
