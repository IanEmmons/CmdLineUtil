
#if !defined(UTILS_H_INCLUDED)
#define UTILS_H_INCLUDED

#include <algorithm>
#include <filesystem>
#include <gsl/gsl>
#include <locale>

#if defined(CMDLINEUTIL_TEST_MODE)
#	define PRIVATE_EXCEPT_IN_TEST public
#else
#	define PRIVATE_EXCEPT_IN_TEST private
#endif

template<typename T, ::std::size_t N>
constexpr ::std::size_t arrayLen(T(&)[N]) noexcept
{
	return N;
}

inline bool isIEqual(const char* pStr1, const char* pStr2)
{
	::std::locale loc;
	auto isIEqual = [&loc](char c1, char c2)
	{
		return ::std::tolower(c1, loc) == ::std::tolower(c2, loc);
	};

	auto span1 = ::gsl::ensure_z(pStr1);
	auto span2 = ::gsl::ensure_z(pStr2);

	return ::std::equal(
		::std::cbegin(span1), ::std::cend(span1),
		::std::cbegin(span2), ::std::cend(span2),
		isIEqual);
}

::std::filesystem::path getTempPath(const ::std::filesystem::path& filePath);

#endif // UTILS_H_INCLUDED
