
// See https://en.cppreference.com/w/cpp/experimental/lib_extensions_2

#include "TestUtil.h"

#include <boost/test/unit_test.hpp>
#include <ostream>
#include <regex>

#if !defined(_WIN32)
#	define USE_OSTREAM_JOINER_CODE
#endif

#if defined(USE_OSTREAM_JOINER_CODE)
#	include <algorithm>
#	include <experimental/iterator>
#endif

using ::std::cbegin;
using ::std::copy;
using ::std::cend;
#if defined(USE_OSTREAM_JOINER_CODE)
using ::std::experimental::make_ostream_joiner;
#endif
using ::std::regex;
using ::std::string;

bool CmdLineParseFailTestCase::doesExMatch(CmdLineError const& ex) const
{
	auto re = regex{m_pExMsgPattern, regex::icase};
	auto result = regex_match(ex.what(), re);
	if (!result)
	{
		BOOST_TEST_MESSAGE("Testing exception message \"" << ex.what()
			<< "\" against \"" << m_pExMsgPattern << "\"");
	}
	return result;
}

::std::ostream& operator<<(::std::ostream& ostrm, CmdLineParseTestCase const& tc)
{
	ostrm << "Test case with args \"";

#if defined(USE_OSTREAM_JOINER_CODE)
	copy(cbegin(tc.m_args), cend(tc.m_args), make_ostream_joiner(ostrm, ' '));
#else
	auto begIt = cbegin(tc.m_args);
	auto endIt = cend(tc.m_args);
	for (auto argIt = begIt; argIt != endIt; ++argIt)
	{
		if (argIt != begIt)
		{
			ostrm << ' ';
		}
		ostrm << *argIt;
	}
#endif

	return ostrm << '\"';
}
