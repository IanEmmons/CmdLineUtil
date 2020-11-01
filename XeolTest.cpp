
#if !defined(CMDLINEUTIL_TEST_MODE)
#define CMDLINEUTIL_TEST_MODE
#endif

#include "Exceptions.h"
#include "PathDeleter.h"
#include "TestUtil.h"
#include "Utils.h"
#include "Xeol.h"

#include <boost/range/algorithm_ext/for_each.hpp>
#include <boost/range/algorithm/sort.hpp>
#include <boost/test/unit_test.hpp>
#include <boost/test/data/test_case.hpp>
#include <string>
#include <sstream>
#include <vector>

namespace b = ::boost;
namespace fs = ::std::filesystem;
namespace ut = ::boost::unit_test;
namespace utd = ::boost::unit_test::data;

using ::std::begin;
using ::std::end;
using ::std::istringstream;
using ::std::ostringstream;
using ::std::string;

using PathList = ::std::vector<fs::path>;

BOOST_AUTO_TEST_SUITE(XeolTestSuite)

BOOST_AUTO_TEST_SUITE(CmdLineParseFailTestSuite)

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

static CmdLineParseFailTestCase const k_testCases[] =
{
	{ k_args00, ".*no files.*" },
	{ k_args01, "^$" },
	{ k_args02, "^$" },
	{ k_args03, "^$" },
	{ k_args04, "(.*option '-f' is allowed only if.*)|(.*no files.*)" },
	{ k_args05, ".*option '-f' is allowed only if.*" },
	{ k_args06, ".*no files.*" },
	{ k_args07, ".*no files.*" },
	{ k_args08, ".*no files.*" },
	{ k_args09, ".*no files.*" },
	{ k_args10, ".*no files.*" },
	{ k_args11, ".*no files.*" },
	{ k_args12, ".*mutually exclusive.*" },
	{ k_args13, ".*mutually exclusive.*" },
	{ k_args14, ".*mutually exclusive.*" },
	{ k_args15, ".*mutually exclusive.*" },
	{ k_args16, ".*mutually exclusive.*" },
};

BOOST_DATA_TEST_CASE(cmdLineParseFailTest, utd::make(k_testCases), tc)
{
	BOOST_CHECK_EXCEPTION(Xeol(tc.m_args), CmdLineError,
		[&tc](const CmdLineError& ex) { return tc.doesExMatch(ex); });
}

BOOST_AUTO_TEST_SUITE_END()



BOOST_AUTO_TEST_SUITE(CmdLineParseOkTestSuite)

struct CmdLineParseOkTestCase : public CmdLineParseTestCase
{
	template<::std::size_t N, ::std::size_t M>
	CmdLineParseOkTestCase(const char*const(&args)[N], bool isInQueryMode,
			Xeol::EolType targetEolType, bool forceTranslation,
			const char*const(&fileList)[M]) noexcept :
		CmdLineParseTestCase(args),
		m_isInQueryMode(isInQueryMode),
		m_targetEolType(targetEolType),
		m_forceTranslation(forceTranslation),
		m_fileList(::gsl::make_span(fileList, M))
		{}

	bool				m_isInQueryMode;
	Xeol::EolType	m_targetEolType;
	bool				m_forceTranslation;
	ArgSpan		m_fileList;
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

static CmdLineParseOkTestCase const k_testCases[] =
{
	{ k_args00, true, Xeol::EolType::INDETERMINATE, false, k_files00 },
	{ k_args01, false, Xeol::EolType::DOS, false, k_files00 },
	{ k_args02, false, Xeol::EolType::DOS, false, k_files00 },
	{ k_args03, false, Xeol::EolType::MACINTOSH, false, k_files00 },
	{ k_args04, false, Xeol::EolType::UNIX, false, k_files00 },
	{ k_args05, false, Xeol::EolType::DOS, true, k_files00 },
	{ k_args06, false, Xeol::EolType::MACINTOSH, true, k_files00 },
	{ k_args07, false, Xeol::EolType::UNIX, true, k_files00 },
	{ k_args08, true, Xeol::EolType::INDETERMINATE, false, k_files01 },
	{ k_args09, true, Xeol::EolType::INDETERMINATE, false, k_files02 },
	{ k_args10, true, Xeol::EolType::INDETERMINATE, false, k_files03 },
};

static void checkEqual(const fs::path& tcPath, const fs::path& appPath)
{
	BOOST_CHECK_EQUAL(tcPath.generic_string(), appPath.generic_string());
}

BOOST_DATA_TEST_CASE(cmdLineParseOkTest, utd::make(k_testCases), tc)
{
	Xeol app(tc.m_args);
	BOOST_CHECK_EQUAL(tc.m_isInQueryMode, app.m_isInQueryMode);
	BOOST_CHECK_EQUAL(static_cast<int>(tc.m_targetEolType), static_cast<int>(app.m_targetEolType));
	BOOST_CHECK_EQUAL(tc.m_forceTranslation, app.m_forceTranslation);
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
	char const*		m_pInput;
	char const*		m_pDosOutput;
	char const*		m_pMacOutput;
	char const*		m_pUnxOutput;
	size_t				m_dosEolCount;
	size_t				m_macEolCount;
	size_t				m_unxEolCount;
	Xeol::EolType	m_eolType;
};

static ScanFileTestCase const k_testCases[] =
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

static ::std::ostream& operator<<(::std::ostream& ostrm, ScanFileTestCase const& tc)
{
	return ostrm << "Test case with input \"" << tc.m_pInput << "\"";
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
	string input(tc.m_pInput);
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
		BOOST_CHECK_EQUAL(tc.m_pDosOutput, out.str());
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
		BOOST_CHECK_EQUAL(tc.m_pMacOutput, out.str());
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
		BOOST_CHECK_EQUAL(tc.m_pUnxOutput, out.str());
	}
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()
