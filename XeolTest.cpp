
#if !defined(CMDLINEUTIL_TEST_MODE)
#define CMDLINEUTIL_TEST_MODE
#endif

#include "Exceptions.h"
#include "main.h"
#include "PathDeleter.h"
#include "Utils.h"
#include "Xeol.h"

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

BOOST_AUTO_TEST_SUITE(XeolTestSuite)

BOOST_AUTO_TEST_SUITE(CmdLineParseFailTestSuite)

struct TestCase
{
	char const*const*const	m_testArgs;
	size_t							m_testArgsCount;
	char const*					m_exceptionMsgTestPattern;
};

static char const*const k_args00[] = { "xeol" };
static char const*const k_args01[] = { "xeol", "-?" };
static char const*const k_args02[] = { "xeol", "-h" };
static char const*const k_args03[] = { "xeol", "-help" };
static char const*const k_args04[] = { "xeol", "-f" };
static char const*const k_args05[] = { "xeol", "-f", "Xeol.cpp" };
static char const*const k_args06[] = { "xeol", "-d" };
static char const*const k_args07[] = { "xeol", "-m" };
static char const*const k_args08[] = { "xeol", "-u" };
static char const*const k_args09[] = { "xeol", "-d", "-f" };
static char const*const k_args10[] = { "xeol", "-m", "-f" };
static char const*const k_args11[] = { "xeol", "-u", "-f" };
static char const*const k_args12[] = { "xeol", "-d", "-m", "Xeol.cpp" };
static char const*const k_args13[] = { "xeol", "-d", "-u", "Xeol.cpp" };
static char const*const k_args14[] = { "xeol", "-m", "-u", "Xeol.cpp" };
static char const*const k_args15[] = { "xeol", "-d", "-m", "-f", "Xeol.cpp" };
static char const*const k_args16[] = { "xeol", "-d", "-m", "-u", "Xeol.cpp" };

static TestCase const k_testCases[] =
{
#define MAKE_TC(args, pattern) \
	{ args, arrayLen(args), pattern },

	MAKE_TC(k_args00, ".*no files.*")
	MAKE_TC(k_args01, "^$")
	MAKE_TC(k_args02, "^$")
	MAKE_TC(k_args03, "^$")
	MAKE_TC(k_args04, "(.*option '-f' is allowed only if.*)|(.*no files.*)")
	MAKE_TC(k_args05, ".*option '-f' is allowed only if.*")
	MAKE_TC(k_args06, ".*no files.*")
	MAKE_TC(k_args07, ".*no files.*")
	MAKE_TC(k_args08, ".*no files.*")
	MAKE_TC(k_args09, ".*no files.*")
	MAKE_TC(k_args10, ".*no files.*")
	MAKE_TC(k_args11, ".*no files.*")
	MAKE_TC(k_args12, ".*mutually exclusive.*")
	MAKE_TC(k_args13, ".*mutually exclusive.*")
	MAKE_TC(k_args14, ".*mutually exclusive.*")
	MAKE_TC(k_args15, ".*mutually exclusive.*")
	MAKE_TC(k_args16, ".*mutually exclusive.*")

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
	BOOST_CHECK_EXCEPTION(Xeol(makeArgsSpan(tc)), CmdLineError, isMatch);
}

BOOST_AUTO_TEST_SUITE_END()



BOOST_AUTO_TEST_SUITE(CmdLineParseOkTestSuite)

struct TestCase
{
	char const*const*const	m_testArgs;
	size_t							m_testArgsCount;
	bool							m_isInQueryMode;
	Xeol::EolType				m_targetEolType;
	bool							m_forceTranslation;
	char const*const*const	m_fileList;
	size_t							m_fileListLen;
};

static char const*const k_args00[] = { "xeol", "Xeol.cpp" };
static char const*const k_args01[] = { "xeol", "-d", "Xeol.cpp" };
static char const*const k_args02[] = { "xeol", "-d", "-d", "Xeol.cpp" };
static char const*const k_args03[] = { "xeol", "-m", "Xeol.cpp" };
static char const*const k_args04[] = { "xeol", "-u", "Xeol.cpp" };
static char const*const k_args05[] = { "xeol", "-d", "-f", "Xeol.cpp" };
static char const*const k_args06[] = { "xeol", "-m", "-f", "Xeol.cpp" };
static char const*const k_args07[] = { "xeol", "-u", "-f", "Xeol.cpp" };
static char const*const k_args08[] = { "xeol", "Xeol.cpp", "Xeol.h", "XeolTest.cpp" };
static char const*const k_args09[] = { "xeol", "Eol*.h" };
static char const*const k_args10[] = { "xeol", "Eol*.h", "Eol*.cpp" };

static char const*const k_files00[] = { "Xeol.cpp" };
static char const*const k_files01[] = { "Xeol.cpp", "Xeol.h", "XeolTest.cpp" };
static char const*const k_files02[] = { "Eol*.h" };
static char const*const k_files03[] = { "Eol*.h", "Eol*.cpp" };

static TestCase const k_testCases[] =
{
#define MAKE_TC(args, isInQueryMode, targetEolType, force, files) \
	{ args, arrayLen(args), isInQueryMode, Xeol::EolType::targetEolType, force, files, arrayLen(files) },

	MAKE_TC(k_args00, true, INDETERMINATE, false, k_files00)
	MAKE_TC(k_args01, false, DOS, false, k_files00)
	MAKE_TC(k_args02, false, DOS, false, k_files00)
	MAKE_TC(k_args03, false, MACINTOSH, false, k_files00)
	MAKE_TC(k_args04, false, UNIX, false, k_files00)
	MAKE_TC(k_args05, false, DOS, true, k_files00)
	MAKE_TC(k_args06, false, MACINTOSH, true, k_files00)
	MAKE_TC(k_args07, false, UNIX, true, k_files00)
	MAKE_TC(k_args08, true, INDETERMINATE, false, k_files01)
	MAKE_TC(k_args09, true, INDETERMINATE, false, k_files02)
	MAKE_TC(k_args10, true, INDETERMINATE, false, k_files03)

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
	Xeol app(makeArgsSpan(tc));
	BOOST_CHECK_EQUAL(tc.m_isInQueryMode, app.m_isInQueryMode);
	BOOST_CHECK_EQUAL(static_cast<int>(tc.m_targetEolType), static_cast<int>(app.m_targetEolType));
	BOOST_CHECK_EQUAL(tc.m_forceTranslation, app.m_forceTranslation);
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
	char const*		m_input;
	char const*		m_dosOutput;
	char const*		m_macOutput;
	char const*		m_unxOutput;
	size_t				m_dosEolCount;
	size_t				m_macEolCount;
	size_t				m_unxEolCount;
	Xeol::EolType	m_eolType;
};

static TestCase const k_testCases[] =
{
	// input, dosOutput, macOutput, unxOutput, dosEolCount, macEolCount, unxEolCount, eolType
	{
		"",
		"",
		"",
		"",
		0, 0, 0, Xeol::EolType::INDETERMINATE
	},
	{
		"This one has no newlines at all",
		"This one has no newlines at all",
		"This one has no newlines at all",
		"This one has no newlines at all",
		0, 0, 0, Xeol::EolType::INDETERMINATE
	},
	{
		"\nHere we see\nthree UNIX newlines\n",
		"\r\nHere we see\r\nthree UNIX newlines\r\n",
		"\rHere we see\rthree UNIX newlines\r",
		"\nHere we see\nthree UNIX newlines\n",
		0, 0, 3, Xeol::EolType::UNIX
	},
	{
		"\rHere we see\rthree Macintosh newlines\r",
		"\r\nHere we see\r\nthree Macintosh newlines\r\n",
		"\rHere we see\rthree Macintosh newlines\r",
		"\nHere we see\nthree Macintosh newlines\n",
		0, 3, 0, Xeol::EolType::MACINTOSH
	},
	{
		"\r\nHere we see\r\nthree DOS newlines\r\n",
		"\r\nHere we see\r\nthree DOS newlines\r\n",
		"\rHere we see\rthree DOS newlines\r",
		"\nHere we see\nthree DOS newlines\n",
		3, 0, 0, Xeol::EolType::DOS
	},
	{
		"z\nz\rz\r\nz\n\rz\r\rz\n\nz\r\r\rz\r\r\nz\r\n\rz\n\r\rz\r\n\nz\n\r\nz\n\n\rz\n\n\nz",
		"z\r\nz\r\nz\r\nz\r\n\r\nz\r\n\r\nz\r\n\r\nz\r\n\r\n\r\nz\r\n\r\nz\r\n\r\nz\r\n\r\n\r\nz\r\n\r\nz\r\n\r\nz\r\n\r\n\r\nz\r\n\r\n\r\nz",
		"z\rz\rz\rz\r\rz\r\rz\r\rz\r\r\rz\r\rz\r\rz\r\r\rz\r\rz\r\rz\r\r\rz\r\r\rz",
		"z\nz\nz\nz\n\nz\n\nz\n\nz\n\n\nz\n\nz\n\nz\n\n\nz\n\nz\n\nz\n\n\nz\n\n\nz",
		5, 12, 12, Xeol::EolType::MIXED
	}
};

static ::std::ostream& operator<<(::std::ostream& ostrm, TestCase const& tc)
{
	return ostrm << "Test case #" << (&tc - k_testCases);
}

static void dumpHex(string const& s)
{
	if (false)
	{
		ostringstream strm;
		strm << "Output: ";
		for (auto ch : s)
		{
			strm << ' ' << static_cast<unsigned int>(static_cast<unsigned char>(ch));
		}
		BOOST_TEST_MESSAGE(strm.str());
	}
}

BOOST_DATA_TEST_CASE(scanFileTest, utd::make(k_testCases), tc)
{
	string input(tc.m_input);
	size_t numDosEols;
	size_t numMacEols;
	size_t numUnixEols;
	size_t totalEols;
	{
		istringstream in(input);
		Xeol::EolType eolType = Xeol::scanFile(in, numDosEols, numMacEols, numUnixEols, totalEols);
		BOOST_CHECK_EQUAL(tc.m_dosEolCount, numDosEols);
		BOOST_CHECK_EQUAL(tc.m_macEolCount, numMacEols);
		BOOST_CHECK_EQUAL(tc.m_unxEolCount, numUnixEols);
		BOOST_CHECK_EQUAL(tc.m_dosEolCount + tc.m_macEolCount + tc.m_unxEolCount, totalEols);
		BOOST_CHECK_EQUAL(static_cast<int>(tc.m_eolType), static_cast<int>(eolType));
	}

	{
		istringstream in(input);
		ostringstream out;
		Xeol::EolType eolType = Xeol::scanFile(in, numDosEols, numMacEols, numUnixEols, totalEols,
			&out, Xeol::EolType::DOS);
		BOOST_CHECK_EQUAL(tc.m_dosEolCount, numDosEols);
		BOOST_CHECK_EQUAL(tc.m_macEolCount, numMacEols);
		BOOST_CHECK_EQUAL(tc.m_unxEolCount, numUnixEols);
		BOOST_CHECK_EQUAL(tc.m_dosEolCount + tc.m_macEolCount + tc.m_unxEolCount, totalEols);
		BOOST_CHECK_EQUAL(static_cast<int>(tc.m_eolType), static_cast<int>(eolType));
		dumpHex(out.str());
		BOOST_CHECK_EQUAL(tc.m_dosOutput, out.str());
	}

	{
		istringstream in(input);
		ostringstream out;
		Xeol::EolType eolType = Xeol::scanFile(in, numDosEols, numMacEols, numUnixEols, totalEols,
			&out, Xeol::EolType::MACINTOSH);
		BOOST_CHECK_EQUAL(tc.m_dosEolCount, numDosEols);
		BOOST_CHECK_EQUAL(tc.m_macEolCount, numMacEols);
		BOOST_CHECK_EQUAL(tc.m_unxEolCount, numUnixEols);
		BOOST_CHECK_EQUAL(tc.m_dosEolCount + tc.m_macEolCount + tc.m_unxEolCount, totalEols);
		BOOST_CHECK_EQUAL(static_cast<int>(tc.m_eolType), static_cast<int>(eolType));
		dumpHex(out.str());
		BOOST_CHECK_EQUAL(tc.m_macOutput, out.str());
	}

	{
		istringstream in(input);
		ostringstream out;
		Xeol::EolType eolType = Xeol::scanFile(in, numDosEols, numMacEols, numUnixEols, totalEols,
			&out, Xeol::EolType::UNIX);
		BOOST_CHECK_EQUAL(tc.m_dosEolCount, numDosEols);
		BOOST_CHECK_EQUAL(tc.m_macEolCount, numMacEols);
		BOOST_CHECK_EQUAL(tc.m_unxEolCount, numUnixEols);
		BOOST_CHECK_EQUAL(tc.m_dosEolCount + tc.m_macEolCount + tc.m_unxEolCount, totalEols);
		BOOST_CHECK_EQUAL(static_cast<int>(tc.m_eolType), static_cast<int>(eolType));
		dumpHex(out.str());
		BOOST_CHECK_EQUAL(tc.m_unxOutput, out.str());
	}
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()
