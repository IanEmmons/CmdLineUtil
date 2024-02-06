#if !defined(FINDFILEEXT_H_INCLUDED)
#define FINDFILEEXT_H_INCLUDED

#include "FileEnumerator.h"
#include "Utils.h"

#include <filesystem>
#include <iosfwd>
#include <map>
#include <span>
#include <string>
#include <string_view>
#include <vector>

class FindFileExt
{
public:
	static int usage(::std::ostream& strm, ::std::string_view progName, const char* pMsg);

	FindFileExt(::std::span<const char*const> args);
	int run();

	FindFileExt(const FindFileExt&) = delete;
	FindFileExt& operator=(const FindFileExt&) = delete;
	FindFileExt(FindFileExt&&) = delete;
	FindFileExt& operator=(FindFileExt&&) = delete;

PRIVATE_EXCEPT_IN_TEST:
	using Path = ::std::filesystem::path;
	using StrToCountMap = ::std::map< ::std::string, size_t >;
	using PathList = ::std::vector<Path>;

	void countFiles();
	void reportExtension(const StrToCountMap::value_type& extToCountMapping);

	bool				m_includeCounts;
	bool				m_outputAsWildcards;
	FileEnumerator	m_fileEnumerator;
	StrToCountMap	m_extToCountMap;
	PathList			m_noExtList;
};

#endif // FINDFILEEXT_H_INCLUDED
