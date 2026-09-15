
#include "FileEnumerator.h"

#include <boost/algorithm/string/replace.hpp>
#include <format>

namespace balg = ::boost::algorithm;
namespace views = ::std::views;

using ::std::format;
using ::std::make_pair;
using ::std::ranges::for_each;
using ::std::ranges::sort;
using ::std::regex;
using ::std::runtime_error;
using ::std::string;
using ::std::string_view;

static constexpr auto k_wildcardSeparator = string_view{")|(?:"};

// ============================ CmdLineFileSpec ============================

string CmdLineFileSpec::wildcard() const
{
	string name = fname();
	if (name.empty())
	{
		throw runtime_error(format("No file name in path '{0}'", m_path.string()));
	}

	balg::replace_all(name, "\\", "\\\\");
	balg::replace_all(name, ".", "\\.");
	balg::replace_all(name, "[", "\\[");
	balg::replace_all(name, "{", "\\{");
	balg::replace_all(name, "}", "\\}");
	balg::replace_all(name, "(", "\\(");
	balg::replace_all(name, ")", "\\)");
	balg::replace_all(name, "+", "\\+");
	balg::replace_all(name, "|", "\\|");
	balg::replace_all(name, "^", "\\^");
	balg::replace_all(name, "$", "\\$");
	balg::replace_all(name, "*", ".*");
	balg::replace_all(name, "?", ".");
	//name = '^' + name + '$';
	return name;
}

// ============================ FileEnumerator ============================

void FileEnumerator::insert(const Path& fileSpecPath)
{
	CmdLineFileSpec fs(fileSpecPath);
	m_fileSpecMap.insert(make_pair(fs.dir(), fs));
}

FileEnumerator::PathList FileEnumerator::getSortedFileSpecList() const
{
	auto filePathView = m_fileSpecMap
		| views::values
		| views::transform([] (const CmdLineFileSpec& clfs) { return clfs.filePath(); });
#ifdef __cpp_lib_containers_ranges
#  warning "NOTICE: Construction of vectors from ranges is supported -- consider removing older code"
	// Starting with C++23, we can do this instead:
	using ::std::from_range;
	PathList result{from_range, filePathView};
#else
	PathList result{begin(filePathView), end(filePathView)};
#endif
	sort(result);
	return result;
}

// Due to the context in which this is called, rootRng is guaranteed not to be empty.
string FileEnumerator::combineRegexPatterns(const RootDirRange& rootRng)
{
	string result;
	size_t numStringsConcatenated = 0;
	for_each(rootRng
			| views::values
			| views::transform([] (const CmdLineFileSpec& clfs) { return clfs.wildcard(); }),
		[&] (string_view str)
		{
			if (numStringsConcatenated > 0)
			{
				result += k_wildcardSeparator;
			}
			result += str;
			++numStringsConcatenated;
		});
	return (numStringsConcatenated > 1)
		? "^(?:" + result + ")$"
		: "^" + result + "$";
}

FileEnumerator::DirPlusRegex FileEnumerator::dirToDirPlusRegex(const Path& dir) const
{
	auto iterPair = m_fileSpecMap.equal_range(dir);
	RootDirRange rootDirRng{iterPair.first, iterPair.second};
	const regex rex{combineRegexPatterns(rootDirRng)};
	return make_pair(dir, rex);
}
