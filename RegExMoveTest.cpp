
#if !defined(CMDLINEUTIL_TEST_MODE)
#define CMDLINEUTIL_TEST_MODE
#endif

#include "Exceptions.h"
#include "PathDeleter.h"
#include "Utils.h"
#include "TestUtil.h"
#include "RegExMove.h"

#include <boost/range/algorithm_ext/for_each.hpp>
#include <boost/range/algorithm/sort.hpp>
#include <boost/test/data/test_case.hpp>
#include <boost/test/unit_test.hpp>
#include <sstream>
#include <string>
#include <vector>

namespace b = ::boost;
namespace ut = ::boost::unit_test;
namespace utd = ::boost::unit_test::data;

using ::std::begin;
using ::std::end;
using ::std::istringstream;
using ::std::ostringstream;
using ::std::string;

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
	{ k_args07, "^The directory \"foo\" does not exist$" },
	{ k_args08, "^\"Xeol.h\" is not a directory$" },
	{ k_args09, "^\".*\" is not a valid regular expression \\(.*\\)$" },
	{ k_args10, "^\".*\" is not a valid regular expression \\(.*\\)$" },
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



//BOOST_AUTO_TEST_SUITE(ScanFileTestSuite)
//
//struct TestCase
//{
//	char const*		m_pInput;
//	char const*		m_pDosOutput;
//	char const*		m_pMacOutput;
//	char const*		m_pUnxOutput;
//	size_t				m_dosEolCount;
//	size_t				m_macEolCount;
//	size_t				m_unxEolCount;
//	Xeol::EolType	m_eolType;
//};
//
//static TestCase const k_testCases[] =
//{
//	// input, dosOutput, macOutput, unxOutput, dosEolCount, macEolCount, unxEolCount, eolType
//	{
//		"",
//		"",
//		"",
//		"",
//		0, 0, 0, Xeol::EolType::INDETERMINATE
//	},
//	{
//		"This one has no newlines at all",
//		"This one has no newlines at all",
//		"This one has no newlines at all",
//		"This one has no newlines at all",
//		0, 0, 0, Xeol::EolType::INDETERMINATE
//	},
//	{
//		"\nHere we see\nthree UNIX newlines\n",
//		"\r\nHere we see\r\nthree UNIX newlines\r\n",
//		"\rHere we see\rthree UNIX newlines\r",
//		"\nHere we see\nthree UNIX newlines\n",
//		0, 0, 3, Xeol::EolType::UNIX
//	},
//	{
//		"\rHere we see\rthree Macintosh newlines\r",
//		"\r\nHere we see\r\nthree Macintosh newlines\r\n",
//		"\rHere we see\rthree Macintosh newlines\r",
//		"\nHere we see\nthree Macintosh newlines\n",
//		0, 3, 0, Xeol::EolType::MACINTOSH
//	},
//	{
//		"\r\nHere we see\r\nthree DOS newlines\r\n",
//		"\r\nHere we see\r\nthree DOS newlines\r\n",
//		"\rHere we see\rthree DOS newlines\r",
//		"\nHere we see\nthree DOS newlines\n",
//		3, 0, 0, Xeol::EolType::DOS
//	},
//	{
//		"z\nz\rz\r\nz\n\rz\r\rz\n\nz\r\r\rz\r\r\nz\r\n\rz\n\r\rz\r\n\nz\n\r\nz\n\n\rz\n\n\nz",
//		"z\r\nz\r\nz\r\nz\r\n\r\nz\r\n\r\nz\r\n\r\nz\r\n\r\n\r\nz\r\n\r\nz\r\n\r\nz\r\n\r\n\r\nz\r\n\r\nz\r\n\r\nz\r\n\r\n\r\nz\r\n\r\n\r\nz",
//		"z\rz\rz\rz\r\rz\r\rz\r\rz\r\r\rz\r\rz\r\rz\r\r\rz\r\rz\r\rz\r\r\rz\r\r\rz",
//		"z\nz\nz\nz\n\nz\n\nz\n\nz\n\n\nz\n\nz\n\nz\n\n\nz\n\nz\n\nz\n\n\nz\n\n\nz",
//		5, 12, 12, Xeol::EolType::MIXED
//	}
//};
//
//static ::std::ostream& operator<<(::std::ostream& ostrm, TestCase const& tc)
//{
//	return ostrm << "Test case with input \"" << tc.m_pInput << "\"";
//}
//
//static void dumpHex(string const& s)
//{
//	if (false)
//	{
//		ostringstream strm;
//		strm << "Output: ";
//		for (auto ch : s)
//		{
//			strm << ' ' << static_cast<unsigned int>(static_cast<unsigned char>(ch));
//		}
//		BOOST_TEST_MESSAGE(strm.str());
//	}
//}
//
//BOOST_DATA_TEST_CASE(scanFileTest, utd::make(k_testCases), tc)
//{
//	string input(tc.m_pInput);
//	size_t numDosEols;
//	size_t numMacEols;
//	size_t numUnixEols;
//	size_t totalEols;
//	{
//		istringstream in(input);
//		Xeol::EolType eolType = Xeol::scanFile(in, numDosEols, numMacEols, numUnixEols, totalEols);
//		BOOST_CHECK_EQUAL(tc.m_dosEolCount, numDosEols);
//		BOOST_CHECK_EQUAL(tc.m_macEolCount, numMacEols);
//		BOOST_CHECK_EQUAL(tc.m_unxEolCount, numUnixEols);
//		BOOST_CHECK_EQUAL(tc.m_dosEolCount + tc.m_macEolCount + tc.m_unxEolCount, totalEols);
//		BOOST_CHECK_EQUAL(static_cast<int>(tc.m_eolType), static_cast<int>(eolType));
//	}
//
//	{
//		istringstream in(input);
//		ostringstream out;
//		Xeol::EolType eolType = Xeol::scanFile(in, numDosEols, numMacEols, numUnixEols, totalEols,
//			&out, Xeol::EolType::DOS);
//		BOOST_CHECK_EQUAL(tc.m_dosEolCount, numDosEols);
//		BOOST_CHECK_EQUAL(tc.m_macEolCount, numMacEols);
//		BOOST_CHECK_EQUAL(tc.m_unxEolCount, numUnixEols);
//		BOOST_CHECK_EQUAL(tc.m_dosEolCount + tc.m_macEolCount + tc.m_unxEolCount, totalEols);
//		BOOST_CHECK_EQUAL(static_cast<int>(tc.m_eolType), static_cast<int>(eolType));
//		dumpHex(out.str());
//		BOOST_CHECK_EQUAL(tc.m_pDosOutput, out.str());
//	}
//
//	{
//		istringstream in(input);
//		ostringstream out;
//		Xeol::EolType eolType = Xeol::scanFile(in, numDosEols, numMacEols, numUnixEols, totalEols,
//			&out, Xeol::EolType::MACINTOSH);
//		BOOST_CHECK_EQUAL(tc.m_dosEolCount, numDosEols);
//		BOOST_CHECK_EQUAL(tc.m_macEolCount, numMacEols);
//		BOOST_CHECK_EQUAL(tc.m_unxEolCount, numUnixEols);
//		BOOST_CHECK_EQUAL(tc.m_dosEolCount + tc.m_macEolCount + tc.m_unxEolCount, totalEols);
//		BOOST_CHECK_EQUAL(static_cast<int>(tc.m_eolType), static_cast<int>(eolType));
//		dumpHex(out.str());
//		BOOST_CHECK_EQUAL(tc.m_pMacOutput, out.str());
//	}
//
//	{
//		istringstream in(input);
//		ostringstream out;
//		Xeol::EolType eolType = Xeol::scanFile(in, numDosEols, numMacEols, numUnixEols, totalEols,
//			&out, Xeol::EolType::UNIX);
//		BOOST_CHECK_EQUAL(tc.m_dosEolCount, numDosEols);
//		BOOST_CHECK_EQUAL(tc.m_macEolCount, numMacEols);
//		BOOST_CHECK_EQUAL(tc.m_unxEolCount, numUnixEols);
//		BOOST_CHECK_EQUAL(tc.m_dosEolCount + tc.m_macEolCount + tc.m_unxEolCount, totalEols);
//		BOOST_CHECK_EQUAL(static_cast<int>(tc.m_eolType), static_cast<int>(eolType));
//		dumpHex(out.str());
//		BOOST_CHECK_EQUAL(tc.m_pUnxOutput, out.str());
//	}
//}
//
//BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()
