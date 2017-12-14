#if !defined(FINDFILEEXT_H_INCLUDED)
#define FINDFILEEXT_H_INCLUDED

#include "FileEnumerator.h"
#include "Utils.h"

#include <boost/filesystem.hpp>
#include <gsl/gsl>
#include <iosfwd>
#include <map>
#include <string>
#include <vector>

class FindFileExt
{
public:
	static int usage(::std::ostream& strm, const ::std::string& progName,
		const char* pMsg);

	FindFileExt(::gsl::span<const char*const> args);
	int run();

	FindFileExt(const FindFileExt&) = delete;
	FindFileExt& operator=(const FindFileExt&) = delete;
	FindFileExt(FindFileExt&&) = delete;
	FindFileExt& operator=(FindFileExt&&) = delete;

PRIVATE_EXCEPT_IN_TEST:
	using Path = ::boost::filesystem::path;
	using StrToCountMap = ::std::map< ::std::string, size_t >;
	using PathList = ::std::vector<Path>;

	void countFiles();
	void reportExtensions();

	bool				m_includeCounts;
	bool				m_outputAsWildcards;
	FileEnumerator	m_fileEnumerator;
	StrToCountMap	m_extToCountMap;
	PathList			m_noExtList;
};

#endif // FINDFILEEXT_H_INCLUDED
