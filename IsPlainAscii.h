
#if !defined(ISPLAINASCII_H_INCLUDED)
#define ISPLAINASCII_H_INCLUDED

#include "FileEnumerator.h"
#include "Utils.h"

#include <filesystem>
#include <iosfwd>
#include <span>
#include <string>
#include <string_view>

class IsPlainAscii
{
public:
	static int usage(::std::ostream& strm, ::std::string_view progName, const char* pMsg);

	IsPlainAscii(::std::span<const char*const> args);
	int run() const;

	IsPlainAscii(const IsPlainAscii&) = delete;
	IsPlainAscii& operator=(const IsPlainAscii&) = delete;
	IsPlainAscii(IsPlainAscii&&) = delete;
	IsPlainAscii& operator=(IsPlainAscii&&) = delete;

PRIVATE_EXCEPT_IN_TEST:
	using Path = ::std::filesystem::path;

	static void scanFile(const Path& filePath);
	static void scanFile2(const Path& filePath, ::std::istream& in,
		::std::ostream& out);
	static void reportNonAsciiRun(const Path& filePath, unsigned long lineNum,
		unsigned long approxColNum, ::std::string& nonAsciiRun, ::std::ostream& out);

	FileEnumerator m_fileEnumerator;
};

#endif // ISPLAINASCII_H_INCLUDED
