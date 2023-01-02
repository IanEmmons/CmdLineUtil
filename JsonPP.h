
#if !defined(JSONPP_H_INCLUDED)
#define JSONPP_H_INCLUDED

#include "FileEnumerator.h"
#include "Utils.h"

#include <boost/json.hpp>
#include <filesystem>
#include <gsl/gsl>
#include <iosfwd>
#include <string_view>

class JsonPP
{
public:
	static int usage(::std::ostream& strm, ::std::string_view progName, const char* pMsg);

	JsonPP(::gsl::span<const char*const> args);
	int run() const;

	JsonPP(const JsonPP&) = delete;
	JsonPP& operator=(const JsonPP&) = delete;
	JsonPP(JsonPP&&) = delete;
	JsonPP& operator=(JsonPP&&) = delete;

PRIVATE_EXCEPT_IN_TEST:
	using Path = ::std::filesystem::path;
	using JValue = ::boost::json::value;

	static void translateFile(const Path& path, bool inPlaceMode, bool minifyMode);
	static JValue parseFile(const Path& path);
	static void prettyPrint(::std::ostream& os, const JValue& jv, bool minifyMode, size_t indentLevel = 0);
	static Path getOutputPath(const Path& filePath, bool minifyMode);

	bool				m_inPlaceMode;
	bool				m_minifyMode;
	FileEnumerator	m_fileEnumerator;
};

#endif // JSONPP_H_INCLUDED
