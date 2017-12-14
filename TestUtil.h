
#if !defined(TESTUTIL_H_INCLUDED)
#define TESTUTIL_H_INCLUDED

#include "Exceptions.h"

#include <gsl/gsl>
#include <iosfwd>
#include <string>

// ===========================================================================
// Boost-related conveniences:
// ===========================================================================

class StringJoinOp
{
public:
	StringJoinOp() : m_separator() {}
	StringJoinOp(const char* pSep) : m_separator(pSep) {}
	StringJoinOp(const ::std::string& sep) : m_separator(sep) {}

	::std::string operator()(::std::string lhs, const char* pRhs)
	{
		if (!lhs.empty())
		{
			lhs += m_separator;
		}
		return lhs += pRhs;
	}

	::std::string operator()(::std::string lhs, const ::std::string& rhs)
	{
		if (!lhs.empty())
		{
			lhs += m_separator;
		}
		return lhs += rhs;
	}

private:
	::std::string m_separator;
};



// ===========================================================================
//
// Test case structs
//
// ===========================================================================

using ArgListPair = ::gsl::span<const char*const>;

struct CmdLineParseTestCase
{
	template<::std::size_t N>
	CmdLineParseTestCase(const char*const(&args)[N]) noexcept
		: m_args(::gsl::make_span(args + 1, N - 1)) {}

	ArgListPair m_args;
};

struct CmdLineParseFailTestCase : public CmdLineParseTestCase
{
	template<::std::size_t N>
	CmdLineParseFailTestCase(const char*const(&args)[N], char const* pExMsgPattern) noexcept :
		CmdLineParseTestCase(args),
		m_pExMsgPattern(pExMsgPattern)
		{}
	bool doesExMatch(CmdLineError const& ex) const;

	char const*	m_pExMsgPattern;
};

::std::ostream& operator<<(::std::ostream& ostrm, CmdLineParseTestCase const& tc);

#endif // TESTUTIL_H_INCLUDED
