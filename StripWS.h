
#if !defined(STRIPWS_H_INCLUDED)
#define STRIPWS_H_INCLUDED

#include "FileEnumerator.h"
#include "Utils.h"

#include <filesystem>
#include <gsl/gsl>
#include <iosfwd>
#include <string>
#include <string_view>

class StripWS
{
public:
	static int usage(::std::ostream& strm, ::std::string_view progName, const char* pMsg);

	StripWS(::gsl::span<const char*const> args);
	int run() const;

	StripWS(const StripWS&) = delete;
	StripWS& operator=(const StripWS&) = delete;
	StripWS(StripWS&&) = delete;
	StripWS& operator=(StripWS&&) = delete;

PRIVATE_EXCEPT_IN_TEST:
	using Path = ::std::filesystem::path;

	void queryFile(const Path& p) const;
	void translateFile(const Path& p) const;

	/// \brief Scans the input file, counts the occurrences of white space at
	/// the end of a line, and optionally strips them into the provided stream.
	///
	/// If pOut is null, then the method simply counts the occurrences of white
	/// space at the end of a line.
	///
	/// If pOut is not null, then the method additionally translates the file
	/// into *pOut.
	static void scanFile(::std::istream& in, /* out */ size_t& numLinesAffected,
		/* out */ size_t& numSpacesStripped, /* out */ size_t& numTabsStripped,
		::std::ostream* pOut = nullptr);

	static void updateCounts(::std::string const& wsRun,
		/* in-out */ size_t& numLinesAffected,
		/* in-out */ size_t& numSpacesStripped,
		/* in-out */ size_t& numTabsStripped);
	static void replaceOriginalFileWithTemp(const Path& originalPath,
		const Path& tempPath);

	bool				m_isInQueryMode;
	FileEnumerator	m_fileEnumerator;
};

#endif // STRIPWS_H_INCLUDED
