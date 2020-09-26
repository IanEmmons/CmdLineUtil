
#if !defined(UTILS_H_INCLUDED)
#define UTILS_H_INCLUDED

#include <boost/algorithm/string/predicate.hpp>
#include <cstddef>
#include <filesystem>
#include <gsl/gsl>
#include <iterator>
#include <memory>
#include <utility>



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
	return ::boost::algorithm::iequals(pStr1, pStr2);
}

::std::filesystem::path getTempPath(const ::std::filesystem::path& filePath);

#endif // UTILS_H_INCLUDED
