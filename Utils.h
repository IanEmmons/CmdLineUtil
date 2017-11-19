
#if !defined(UTILS_H_INCLUDED)
#define UTILS_H_INCLUDED

#include <boost/filesystem.hpp>
#include <cstddef>
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

::boost::filesystem::path getTempPath(const ::boost::filesystem::path& filePath);



// ===========================================================================
// Some C++14 items that should have been in C++11
// ===========================================================================

template<typename T, typename... Ts>
::std::unique_ptr<T> makeUnique(Ts&&... params)
{
	return ::std::unique_ptr<T>(new T(::std::forward<Ts>(params)...));
}

template<typename C>
auto cBegin(const C& container) -> decltype(::std::begin(container))
{
	return ::std::begin(container);
}

template<typename C>
auto cEnd(const C& container) -> decltype(::std::end(container))
{
	return ::std::end(container);
}

#endif // UTILS_H_INCLUDED
