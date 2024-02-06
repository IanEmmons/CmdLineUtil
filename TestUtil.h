
#if !defined(TESTUTIL_H_INCLUDED)
#define TESTUTIL_H_INCLUDED

#include "Exceptions.h"

#include <iosfwd>
#include <span>

// ===========================================================================
//
// Test case structs
//
// ===========================================================================

using ArgSpan = ::std::span<const char*const>;

struct CmdLineParseTestCase
{
	template<::std::size_t N>
	CmdLineParseTestCase(const char*const(&args)[N]) noexcept
		: m_args(::std::span{args + 1, N - 1}) {}

	ArgSpan m_args;
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
