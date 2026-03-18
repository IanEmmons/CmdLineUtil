
#include "FileEnumerator.h"

#include <boost/algorithm/string/replace.hpp>
#include <boost/range/algorithm_ext/push_back.hpp>
#include <format>
#include <string_view>

namespace b = ::boost;
namespace bad = ::boost::adaptors;
namespace balg = ::boost::algorithm;

using ::std::format;
using ::std::make_pair;
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

void FileEnumerator::insert(const char* pFileSpecStr)
{
	CmdLineFileSpec fs(pFileSpecStr);
	m_fileSpecMap.insert(make_pair(fs.dir(), fs));
}

void FileEnumerator::insert(const Path& fileSpecPath)
{
	CmdLineFileSpec fs(fileSpecPath);
	m_fileSpecMap.insert(make_pair(fs.dir(), fs));
}

#if defined(CMDLINEUTIL_TEST_MODE)
void FileEnumerator::getFileSpecList(PathList& fileSpecList) const
{
	b::push_back(fileSpecList, m_fileSpecMap
		| bad::map_values
		| bad::transformed([] (const CmdLineFileSpec& clfs) { return clfs.filePath(); }));
}
#endif

// Due to the context in which this is called, rootRng is guaranteed not to be empty.
string FileEnumerator::combineRegexPatterns(const RootDirRng& rootRng)
{
	string result;
	size_t numStringsConcatenated = 0;
	b::for_each(rootRng
			| bad::map_values
			| bad::transformed([] (const CmdLineFileSpec& clfs) { return clfs.wildcard(); }),
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
	RootDirRng rootRng(m_fileSpecMap.equal_range(dir));
	const regex rex(combineRegexPatterns(rootRng));
	return make_pair(dir, rex);
}
