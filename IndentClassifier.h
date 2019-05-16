
#if !defined(INDENTCLASSIFIER_H_INCLUDED)
#define INDENTCLASSIFIER_H_INCLUDED

#include "FileEnumerator.h"
#include "Utils.h"

#include <boost/filesystem.hpp>
#include <gsl/gsl>
#include <iosfwd>
#include <map>
#include <string>

/// \brief IndentType contains the enum constants that indicate the type of
/// indentation for a line of text or a text file.
enum class IndentType
{
	space, ///< Indents consist entirely of spaces
	tab, ///< Indents consist entirely of tabs
	javadocTab,	///< Indents are entirely tabs or tabs followed by exactly one space and asterisk, as in a tab-indented JavaDoc comment
	javadocLeft,	///< Indented exactly one space followed by an asterisk, as in an unindented JavaDoc comment (single-line only)
	mixed, ///< Indents are mixed tabs and spaces, or some are tabs and others are spaces
	indeterminate ///< Refers to lines with no whitespace indent
};

using LineTypeCounts = ::std::map<IndentType, size_t>;

/// \brief The same as lineTypeCounts.at(indentType) except that it returns zero
/// if the map does not contain an entry for indentType.
size_t get(const LineTypeCounts& lineTypeCounts, IndentType indentType);

class IndentClassifier
{
public:
	static int usage(::std::ostream& strm, const ::std::string& progName, const char* pMsg);

	IndentClassifier(::gsl::span<const char*const> args);
	int run() const;

	IndentClassifier(const IndentClassifier&) = delete;
	IndentClassifier& operator=(const IndentClassifier&) = delete;
	IndentClassifier(IndentClassifier&&) = delete;
	IndentClassifier& operator=(IndentClassifier&&) = delete;

PRIVATE_EXCEPT_IN_TEST:
	using Path = ::boost::filesystem::path;

	void processFile(const Path& p) const;
	static char indicatorLetter(IndentType iType);
	static ::std::string displayPath(const Path& p);

	/// \brief Scans the input stream and counts lines of the various indent types.
	static LineTypeCounts scanFile(const Path& p);
	static LineTypeCounts scanFile(::std::istream& in, bool isJavaFile);
	static IndentType classifyLine(const ::std::string& line, bool isJavaFile);
	static IndentType classifyFile(const LineTypeCounts& lineTypeCounts);

	FileEnumerator m_fileEnumerator;
};

#endif // INDENTCLASSIFIER_H_INCLUDED
