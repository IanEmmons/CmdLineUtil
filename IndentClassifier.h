
#if !defined(INDENTCLASSIFIER_H_INCLUDED)
#define INDENTCLASSIFIER_H_INCLUDED

#include "FileEnumerator.h"
#include "Utils.h"

#include <boost/filesystem.hpp>
#include <iosfwd>
#include <string>

class IndentClassifier
{
public:
	static int usage(::std::ostream& strm, const char* pMsg);

	IndentClassifier(size_t argCount, const char*const*const ppArgList);
	int run() const;

	IndentClassifier(const IndentClassifier&) = delete;
	IndentClassifier& operator=(const IndentClassifier&) = delete;
	IndentClassifier(IndentClassifier&&) = delete;
	IndentClassifier& operator=(IndentClassifier&&) = delete;

PRIVATE_EXCEPT_IN_TEST:
	using Path = ::boost::filesystem::path;

	/// \brief IndentType contains the enum constants that indicate the type of
	/// indentation for a line of text or a text file.
	enum class IndentType
	{
		SPACE, ///< Indents consist entirely of spaces
		TAB, ///< Indents consist entirely of tabs
		MIXED, ///< Indents are mixed tabs and spaces, or some indents are tabs and others are spaces
		INDETERMINATE ///< Refers to lines with no whitespace indent
	};

	void queryFile(const Path& p) const;
	static char getIndicatorLetter(IndentType iType);
	static ::std::string displayPath(const Path& p);

	/// \brief Scans the input stream, counts the indent types, and
	/// returns the corresponding IndentType constant.
	static IndentType scanFile(::std::istream& in, /* out */ size_t& numSpaceLines,
		/* out */ size_t& numTabLines, /* out */ size_t& numMixedLines,
		/* out */ size_t& numIndLines, /* out */ size_t& totalLines);

	FileEnumerator m_fileEnumerator;
};

#endif // INDENTCLASSIFIER_H_INCLUDED
