
#if !defined(REGEXMOVE_H_INCLUDED)
#define REGEXMOVE_H_INCLUDED

#include <boost/filesystem.hpp>
#include <boost/regex.hpp>
#include <iosfwd>
#include <string>

class RegExMove
{
public:
	static int usage(::std::ostream& strm, const char* pMsg);

	RegExMove(size_t argCount, const char*const*const ppArgList);
	int run() const;

	RegExMove(const RegExMove&) = delete;
	RegExMove& operator=(const RegExMove&) = delete;
	RegExMove(RegExMove&&) = delete;
	RegExMove& operator=(RegExMove&&) = delete;

private:
	using Path = ::boost::filesystem::path;
	using DirEntry = ::boost::filesystem::directory_entry;

	void processDirectoryEntry(DirEntry const& dirEntry) const;
	void renamePath(Path const& p) const;

	bool				m_caseSensitive;
	bool				m_renameDirectories;
	bool				m_renameFiles;
	bool				m_recursiveSearch;
	bool				m_verboseOutput;
	bool				m_allowOverwriteOnNameCollision;
	Path				m_rootDir;
	::boost::regex	m_pattern;
	::std::string	m_replacement;
};

#endif // REGEXMOVE_H_INCLUDED
