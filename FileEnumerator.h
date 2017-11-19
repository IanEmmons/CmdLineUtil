
#if !defined(FILEENUMERATOR_H_INCLUDED)
#define FILEENUMERATOR_H_INCLUDED

#include <boost/filesystem.hpp>
#include <boost/range/adaptor/filtered.hpp>
#include <boost/range/adaptor/map.hpp>
#include <boost/range/adaptor/transformed.hpp>
#include <boost/range/adaptor/uniqued.hpp>
#include <boost/range/algorithm/for_each.hpp>
#include <boost/range/iterator_range.hpp>
#include <boost/regex.hpp>
#include <map>
#include <string>
#include <utility>
#if defined(CMDLINEUTIL_TEST_MODE)
#include <vector>
#endif

class CmdLineFileSpec
{
public:
	using Path = ::boost::filesystem::path;

	CmdLineFileSpec(const char* pFileSpecStr) : m_path(pFileSpecStr) {}
	CmdLineFileSpec(const ::std::string& fileSpecStr) : m_path(fileSpecStr) {}
	CmdLineFileSpec(const Path& fileSpecPath) : m_path(fileSpecPath) {}

	bool hasWildcard() const
		{ return fname().find_first_of("*?", 0) != ::std::string::npos; }
	const Path& filePath() const
		{ return m_path; }
	Path dir() const
		{ return m_path.has_parent_path() ? m_path.parent_path() : "."; }
	::std::string fname() const
		{ return m_path.filename().string(); }
	::std::string wildcard() const;

private:
	Path m_path;
};

class FileEnumerator
{
public:
	using Path = ::boost::filesystem::path;

	FileEnumerator()
		: m_isRecursive(false), m_fileSpecMap() {}

	void setRecursive(bool newRecursiveValue = true)
		{ m_isRecursive = newRecursiveValue; }

	void insert(const char* pFileSpecStr);
	void insert(const ::std::string& fileSpecStr);
	void insert(const Path& fileSpecPath);

	bool isRecursive() const
		{ return m_isRecursive; }
	size_t numFileSpecs() const
		{ return m_fileSpecMap.size(); }

	// FileProcessingFunctor takes a single parameter of type
	// "const boost::filesystem::path&" and returns "void".
	template<typename FileProcessingFunctor>
	void enumerateFiles(FileProcessingFunctor functor) const
		{
			::boost::for_each(m_fileSpecMap
					| ::boost::adaptors::map_keys
					| ::boost::adaptors::uniqued
					| ::boost::adaptors::transformed(
						[this] (const Path& dir) { return dirToDirPlusRegex(dir); }),
				[=] (const DirPlusRegex& dirPlusRegex) { processRootDir(dirPlusRegex, functor); });
		}

#if defined(CMDLINEUTIL_TEST_MODE)
	// Testing facilities:
	using PathList = ::std::vector<Path>;
	void getFileSpecList(PathList& fileSpecList) const;
#endif

private:
	using FileSpecMap = ::std::multimap<Path, CmdLineFileSpec>;
	using RootDirRng = ::boost::iterator_range<FileSpecMap::const_iterator>;
	using DirPlusRegex = ::std::pair<Path, ::boost::regex>;
	using DirEntry = ::boost::filesystem::directory_entry;
	using DirIter = ::boost::filesystem::directory_iterator;
	using RecDirIter = ::boost::filesystem::recursive_directory_iterator;

	// If the wildcard patterns for the FileSpecs in the range are
	// pattern1, pattern2, pattern3, then this method produces a
	// single pattern that looks like this:
	//    ^(?:pattern1)|(?:pattern2)|(?:pattern3)$
	static ::std::string combineRegexPatterns(const RootDirRng& rootRng);

	DirPlusRegex dirToDirPlusRegex(const Path& dir) const;
	static bool isFile(const Path& p)	// canonical() resolves symlinks
		{ return exists(p) && is_regular_file(canonical(p)); }
	static bool matchesWildcard(const Path& p, const ::boost::regex& rex)
		{ return regex_match(p.filename().string(), rex); }

	template<typename DirIterType, typename FileProcessingFunctor>
	void processRootDirHelper(const DirPlusRegex& dirPlusRegex, FileProcessingFunctor functor) const
	{
		::boost::for_each(::boost::make_iterator_range(
					DirIterType(dirPlusRegex.first), DirIterType())
				| ::boost::adaptors::transformed(
					[] (const DirEntry& de) { return de.path(); })
				| ::boost::adaptors::filtered(
					[&] (const Path& p)
						{ return isFile(p) && matchesWildcard(p, dirPlusRegex.second); }),
			functor);
	}

	template<typename FileProcessingFunctor>
	void processRootDir(const DirPlusRegex& dirPlusRegex, FileProcessingFunctor functor) const
	{
		if (m_isRecursive)
		{
			processRootDirHelper<RecDirIter>(dirPlusRegex, functor);
		}
		else
		{
			processRootDirHelper<DirIter>(dirPlusRegex, functor);
		}
	}

	bool			m_isRecursive;
	FileSpecMap	m_fileSpecMap;
};

#endif // FILEENUMERATOR_H_INCLUDED
