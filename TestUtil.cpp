
#include "TestUtil.h"

#include <boost/range/numeric.hpp>
#include <boost/test/unit_test.hpp>
#include <ostream>
#include <regex>

using ::boost::accumulate;
using ::gsl::make_span;
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
	return ostrm << "Test case with args \""
		<< accumulate(tc.m_args, string(), StringJoinOp(" ")) << "\"";
}
