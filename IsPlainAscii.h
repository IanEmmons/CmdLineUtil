
#if !defined(ISPLAINASCII_H_INCLUDED)
#define ISPLAINASCII_H_INCLUDED

#include "FileEnumerator.h"
#include "Utils.h"

#include <boost/filesystem.hpp>
#include <gsl/gsl>
#include <iosfwd>
#include <string>

class IsPlainAscii
{
public:
	static int usage(::std::ostream& strm, const char* pMsg);

	IsPlainAscii(::gsl::span<const char*const> args);
	int run() const;

	IsPlainAscii(const IsPlainAscii&) = delete;
	IsPlainAscii& operator=(const IsPlainAscii&) = delete;
	IsPlainAscii(IsPlainAscii&&) = delete;
	IsPlainAscii& operator=(IsPlainAscii&&) = delete;

PRIVATE_EXCEPT_IN_TEST:
	using Path = ::boost::filesystem::path;

	static void scanFile(const Path& filePath);
	static void scanFile2(const Path& filePath, ::std::istream& in,
		::std::ostream& out);
	static void reportNonAsciiRun(const Path& filePath, unsigned long lineNum,
		unsigned long approxColNum, ::std::string& nonAsciiRun, ::std::ostream& out);

	FileEnumerator m_fileEnumerator;
};

#endif // ISPLAINASCII_H_INCLUDED
