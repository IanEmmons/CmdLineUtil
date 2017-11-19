
#if !defined(XEOL_H_INCLUDED)
#define XEOL_H_INCLUDED

#include "FileEnumerator.h"
#include "Utils.h"

#include <boost/filesystem.hpp>
#include <iosfwd>
#include <string>

class Xeol
{
public:
	static int usage(::std::ostream& strm, const char* pMsg);

	Xeol(size_t argCount, const char*const*const ppArgList);
	int run() const;

	Xeol(const Xeol&) = delete;
	Xeol& operator=(const Xeol&) = delete;
	Xeol(Xeol&&) = delete;
	Xeol& operator=(Xeol&&) = delete;

PRIVATE_EXCEPT_IN_TEST:
	/// \brief EolType contains the enum constants that indicate the type of
	/// end-of-line for a text file.
	enum class EolType
	{
		INDETERMINATE, ///< File contains no ends-of-line
		MIXED, ///< Inconsistent end-of-line convention, may be binary
		DOS, ///< End-of-line is "\r\n"
		MACINTOSH, ///< End-of-line is "\r" (now archaic)
		UNIX ///< End-of-line is "\n"
	};

	using Path = ::boost::filesystem::path;

	void queryFile(const Path& p) const;
	void translateFile(const Path& p) const;
	static char getIndicatorLetter(EolType eolType);
	static ::std::string toEolString(EolType eolType);
	static ::std::string displayPath(const Path& p);

	/// \brief Scans the input file, counts the end-of-line types, and
	/// optionally translates the ends-of-line into the provided stream.
	///
	/// If pOut is null, then the method simply counts the end-of-line types and
	/// returns the corresponding EolType constant.  In this case, the
	/// targetEolType parameter is ignored.
	///
	/// If pOut is not null, then the method additionally translates the file
	/// into *pOut.  In this case, targetEolType must be one of DOS, MACINTOSH,
	/// or UNIX.
	static EolType scanFile(::std::istream& in,
		/* out */ size_t& numDosEols, /* out */ size_t& numMacEols,
		/* out */ size_t& numUnixEols, /* out */ size_t& totalEols,
		::std::ostream* pOut = nullptr, EolType targetEolType = EolType::INDETERMINATE);

	static void replaceOriginalFileWithTemp(const Path& originalPath,
		const Path& tempPath);

	bool				m_isInQueryMode;
	EolType			m_targetEolType;
	bool				m_forceTranslation;
	FileEnumerator	m_fileEnumerator;
};

#endif // XEOL_H_INCLUDED
