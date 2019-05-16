
#if !defined(REGEXMOVE_H_INCLUDED)
#define REGEXMOVE_H_INCLUDED

#include "Utils.h"

#include <boost/filesystem.hpp>
#include <gsl/gsl>
#include <iosfwd>
#include <regex>
#include <string>

class RegExMove
{
public:
	static int usage(::std::ostream& strm, const ::std::string& progName,
		const char* pMsg);

	RegExMove(::gsl::span<const char*const> args);
	int run() const;

	RegExMove(const RegExMove&) = delete;
	RegExMove& operator=(const RegExMove&) = delete;
	RegExMove(RegExMove&&) = delete;
	RegExMove& operator=(RegExMove&&) = delete;

PRIVATE_EXCEPT_IN_TEST:
	using Path = ::boost::filesystem::path;
	using DirEntry = ::boost::filesystem::directory_entry;

	template <typename DirIter> void processDirectoryEntries() const;
	void processDirectoryEntry(DirEntry const& dirEntry) const;
	void renamePath(Path const& p) const;

	bool				m_caseSensitive;
	bool				m_renameDirectories;
	bool				m_renameFiles;
	bool				m_recursiveSearch;
	bool				m_verboseOutput;
	bool				m_allowOverwriteOnNameCollision;
	Path				m_rootDir;
	::std::string	m_patternStr;
	::std::regex	m_pattern;
	::std::string	m_replacement;
};

#endif // REGEXMOVE_H_INCLUDED
