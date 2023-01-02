
#if !defined(CMDLINEUTIL_TEST_MODE)
#define CMDLINEUTIL_TEST_MODE
#endif

#include "JsonPP.h"
#include "PathDeleter.h"
#include "TestUtil.h"
#include "Utils.h"

#include <boost/range/algorithm_ext/for_each.hpp>
#include <boost/range/algorithm/sort.hpp>
#include <boost/test/unit_test.hpp>
#include <boost/test/data/test_case.hpp>
#include <fstream>
#include <string>
#include <string_view>
#include <vector>

namespace b = ::boost;
namespace utd = ::boost::unit_test::data;

using ::std::begin;
using ::std::end;
using ::std::ifstream;
using ::std::ios_base;
using ::std::string;
using ::std::string_view;

using PathList = ::std::vector<JsonPP::Path>;

BOOST_AUTO_TEST_SUITE(JsonPPTestSuite)

BOOST_AUTO_TEST_SUITE(CmdLineParseFailTestSuite)

static char const*const k_args00[] = { "jsonpp" };
static char const*const k_args01[] = { "jsonpp", "-?" };
static char const*const k_args02[] = { "jsonpp", "-h" };
static char const*const k_args03[] = { "jsonpp", "-help" };
static char const*const k_args04[] = { "jsonpp", "-ip" };
static char const*const k_args05[] = { "jsonpp", "-m" };
static char const*const k_args06[] = { "jsonpp", "-ip", "-m", "-r" };

static CmdLineParseFailTestCase const k_testCases[] =
{
	{ k_args00, ".*no files.*" },
	{ k_args01, "^$" },
	{ k_args02, "^$" },
	{ k_args03, "^$" },
	{ k_args04, ".*no files.*" },
	{ k_args05, ".*no files.*" },
	{ k_args06, ".*no files.*" },
};

BOOST_DATA_TEST_CASE(cmdLineParseFailTest, utd::make(k_testCases), tc)
{
	BOOST_CHECK_EXCEPTION(JsonPP(tc.m_args), CmdLineError,
		[&tc](const CmdLineError& ex) { return tc.doesExMatch(ex); });
}

BOOST_AUTO_TEST_SUITE_END()



BOOST_AUTO_TEST_SUITE(CmdLineParseOkTestSuite)

struct CmdLineParseOkTestCase : public CmdLineParseTestCase
{
	template<::std::size_t N, ::std::size_t M>
	CmdLineParseOkTestCase(const char*const(&args)[N], bool inPlaceMode, bool minifyMode,
			bool isRecursive, const char*const(&fileList)[M]) noexcept :
		CmdLineParseTestCase(args),
		m_inPlaceMode(inPlaceMode),
		m_minifyMode(minifyMode),
		m_isRecursive(isRecursive),
		m_fileList(::gsl::make_span(fileList, M))
		{}

	bool		m_inPlaceMode;
	bool		m_minifyMode;
	bool		m_isRecursive;
	ArgSpan	m_fileList;
};

static char const*const k_args00[] = { "jsonpp", "JsonPPTestInput.json" };
static char const*const k_args01[] = { "jsonpp", "-m", "*Input.json" };
static char const*const k_args02[] = { "jsonpp", "-ip", "-r", "*Input-temp.json" };

static char const*const k_files00[] = { "JsonPPTestInput.json" };
static char const*const k_files01[] = { "*Input.json" };
static char const*const k_files02[] = { "*Input-temp.json" };

static CmdLineParseOkTestCase const k_testCases[] =
{
	{ k_args00, false, false, false, k_files00 },
	{ k_args01, false, true, false, k_files01 },
	{ k_args02, true, false, true, k_files02 },
};

static void checkEqual(const JsonPP::Path& tcPath, const JsonPP::Path& appPath)
{
	BOOST_CHECK_EQUAL(tcPath.generic_string(), appPath.generic_string());
}

BOOST_DATA_TEST_CASE(cmdLineParseOkTest, utd::make(k_testCases), tc)
{
	JsonPP app(tc.m_args);
	BOOST_CHECK_EQUAL(tc.m_inPlaceMode, app.m_inPlaceMode);
	BOOST_CHECK_EQUAL(tc.m_minifyMode, app.m_minifyMode);
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



BOOST_AUTO_TEST_SUITE(TranslateFileTestSuite)

struct PrettyPrintTestCase
{
	JsonPP::Path	m_inputFile;
	JsonPP::Path	m_expectedOutputFile;
	bool				m_minifyMode;
};

static PrettyPrintTestCase const k_testCases[] =
{
	{
		"JsonPPTestInput.json",
		"JsonPPTestOutputPretty.json",
		false
	},
	{
		"JsonPPTestInput.json",
		"JsonPPTestOutputMinified.json",
		true
	}
};

static ::std::ostream& operator<<(::std::ostream& ostrm, PrettyPrintTestCase const& tc)
{
	return ostrm << "Test case with output " << tc.m_expectedOutputFile;
}

static string readFile(const JsonPP::Path& path)
{
	ifstream in(path, ios_base::in | ios_base::binary);
	if (!in)
	{
		throw ::std::ios_base::failure(str(b::format("Unable to open file '%1%'")
			% path.generic_string()));
	}

	string contents;
	for (; in && !in.eof();)
	{
		char buffer[4096];
		in.read(buffer, arrayLen(buffer));
		contents += string_view(buffer, in.gcount());
	}

	if (!in && !in.eof())
	{
		throw ::std::ios_base::failure(str(b::format("Unable to read file '%1%'")
			% path.generic_string()));
	}

	return contents;
}

BOOST_DATA_TEST_CASE(translateFileTest, utd::make(k_testCases), tc)
{
	auto const actualOutputFile = JsonPP::getOutputPath(tc.m_inputFile, tc.m_minifyMode);
	PathDeleter deleter(actualOutputFile);
	JsonPP::translateFile(tc.m_inputFile, false, tc.m_minifyMode);
	auto const expectedContent = readFile(tc.m_expectedOutputFile);
	auto const actualContent = readFile(actualOutputFile);
	BOOST_CHECK_EQUAL(expectedContent, actualContent);
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()
