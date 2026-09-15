
#if !defined(FILEENUMERATOR_H_INCLUDED)
#define FILEENUMERATOR_H_INCLUDED

#include <algorithm>
#include <filesystem>
#include <map>
#include <ranges>
#include <regex>
#include <set>
#include <string>
#include <utility>
#include <vector>

class CmdLineFileSpec
{
public:
	using Path = ::std::filesystem::path;

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
	using Path = ::std::filesystem::path;
	using PathList = ::std::vector<Path>;

	FileEnumerator()
		: m_isRecursive(false), m_fileSpecMap() {}

	void setRecursive(bool newRecursiveValue = true)
		{ m_isRecursive = newRecursiveValue; }

	void insert(const Path& fileSpecPath);

	bool isRecursive() const
		{ return m_isRecursive; }
	size_t numFileSpecs() const
		{ return m_fileSpecMap.size(); }
	PathList getSortedFileSpecList() const;

	// FileProcessingFunctor takes a single parameter of type
	// "const std::filesystem::path&" and returns "void".
	template<typename FileProcessingFunctor>
	void enumerateFiles(FileProcessingFunctor functor) const
		{
			auto filePathView = m_fileSpecMap
				| ::std::views::keys;
			::std::set<Path> uniqueDirs{begin(filePathView), end(filePathView)};
			::std::ranges::for_each(uniqueDirs
					| ::std::views::transform(
						[this] (const Path& dir) { return dirToDirPlusRegex(dir); }),
				[this, functor] (const DirPlusRegex& dirPlusRegex) { processRootDir(dirPlusRegex, functor); });
		}

private:
	using FileSpecMap = ::std::multimap<Path, CmdLineFileSpec>;
	using RootDirRange = ::std::ranges::subrange<FileSpecMap::const_iterator>;
	using DirPlusRegex = ::std::pair<Path, ::std::regex>;
	using DirEntry = ::std::filesystem::directory_entry;
	using DirIter = ::std::filesystem::directory_iterator;
	using RecDirIter = ::std::filesystem::recursive_directory_iterator;

	// If the wildcard patterns for the FileSpecs in the range are
	// pattern1, pattern2, pattern3, then this method produces a
	// single pattern that looks like this:
	//    ^(?:pattern1)|(?:pattern2)|(?:pattern3)$
	static ::std::string combineRegexPatterns(const RootDirRange& rootRng);

	DirPlusRegex dirToDirPlusRegex(const Path& dir) const;
	static bool isFile(const Path& p)	// canonical() resolves symlinks
		{ return exists(p) && is_regular_file(canonical(p)); }
	static bool matchesWildcard(const Path& p, const ::std::regex& rex)
		{ return regex_match(p.filename().string(), rex); }

	template<typename DirIterType, typename FileProcessingFunctor>
	void processRootDirHelper(const DirPlusRegex& dirPlusRegex, FileProcessingFunctor functor) const
	{
		::std::ranges::for_each(::std::ranges::subrange(
					DirIterType(dirPlusRegex.first), DirIterType())
				| ::std::views::transform(
					[] (const DirEntry& de) { return de.path(); })
				| ::std::views::filter(
					[regex = dirPlusRegex.second] (const Path& p)
						{ return isFile(p) && matchesWildcard(p, regex); }),
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
