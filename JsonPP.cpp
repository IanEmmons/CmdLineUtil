
#include "JsonPP.h"
#include "main.h"

#include <boost/format.hpp>
#include <fstream>
#include <string>

namespace b = ::boost;
namespace json = ::boost::json;

using ::std::endl;
using ::std::error_code;
using ::std::ifstream;
using ::std::ios_base;
using ::std::ofstream;
using ::std::ostream;
using ::std::string;
using ::std::string_view;

#if !defined(CMDLINEUTIL_TEST_MODE)
int main(int argCount, const char*const*const argList)
{
	return commonMain<JsonPP>(argCount, argList);
}
#endif

int JsonPP::usage(ostream& out, string_view progName, const char* pMsg)
{
	int exitCode = EXIT_SUCCESS;
	if (pMsg != nullptr && *pMsg != '\0')
	{
		exitCode = EXIT_FAILURE;
		out << endl << pMsg << endl;
	}
	out << "\n"
		"Usage:  " << progName << " [-ip] [-m] [-r] <json-file1> <json-file2> ...\n"
		"\n"
		"Pretty-prints each JSON input file, indenting with tab characters. The\n"
		"output is placed in a file whose name is the same as the original with\n"
		"\"-pretty\" or \"-minified\" appended to it, unless the -ip option is given.\n"
		"\n"
		"Options:\n"
		"\n"
		"   -ip Pretty-prints the file in-place, i.e., overwrites the\n"
		"       original file with the pretty-printed version.\n"
		"\n"
		"   -m  Minifies the JSON, instead of pretty-printing it.\n"
		"\n"
		"   -r  Search for files in sub-directories recursively.\n"
		<< endl;

	return exitCode;
}

JsonPP::JsonPP(::std::span<const char*const> args) :
	m_inPlaceMode(false),
	m_minifyMode(false),
	m_fileEnumerator()
{
	for (auto pArg : args)
	{
		if (isIEqual(pArg, "-?") || isIEqual(pArg, "-h") || isIEqual(pArg, "-help"))
		{
			throw CmdLineError();
		}
		else if (isIEqual(pArg, "-ip"))
		{
			m_inPlaceMode = true;
		}
		else if (isIEqual(pArg, "-m"))
		{
			m_minifyMode = true;
		}
		else if (isIEqual(pArg, "-r"))
		{
			m_fileEnumerator.setRecursive();
		}
		else
		{
			m_fileEnumerator.insert(pArg);
		}
	}

	if (m_fileEnumerator.numFileSpecs() <= 0)
	{
		throw CmdLineError("No files specified");
	}
}

int JsonPP::run() const
{
	m_fileEnumerator.enumerateFiles([this] (const Path& p)
		{ translateFile(p, m_inPlaceMode, m_minifyMode); });
	return EXIT_SUCCESS;
}

void JsonPP::translateFile(const Path& path, bool inPlaceMode, bool minifyMode)
{
	auto const jsonValue = JsonPP::parseFile(path);
	const auto outputPath = getOutputPath(path, minifyMode);

	{
		ofstream out(outputPath, ios_base::out | ios_base::trunc);
		prettyPrint(out, jsonValue, minifyMode);
	}

	if (inPlaceMode)
	{
		rename(outputPath, path);
	}
}

JsonPP::JValue JsonPP::parseFile(const Path& path)
{
	ifstream in(path, ios_base::in | ios_base::binary);
	if (!in)
	{
		throw ::std::ios_base::failure(str(b::format("Unable to open file '%1%'")
			% path.generic_string()));
	}

	string line;
	json::stream_parser p;
	error_code ec;
	unsigned long lineNum = 0;
	do
	{
		++lineNum;
		getline(in, line);
		line += '\n';	// ensure equivalent whitespace
		p.write(line, ec);
		if (ec)
		{
			throw SyntaxError(b::format("Parse error near line %1%: %2% (%3%:%4%)")
				% lineNum % ec.message() % ec.category().name() % ec.value());
		}
	} while(in && !in.eof());

	if (!in && !in.eof())
	{
		throw ::std::ios_base::failure(str(b::format("Unable to read file '%1%'")
			% path.generic_string()));
	}

	p.finish();
	return p.release();
}

void JsonPP::prettyPrint(ostream& os, JValue const& jv, bool minifyMode, size_t indentLevel /* = 0 */)
{
	switch(jv.kind())
	{
	case json::kind::object:
	{
		os << (minifyMode ? "{" : "{\n");
		auto const& obj = jv.get_object();
		if(!obj.empty())
		{
			for(auto it = obj.begin();;)
			{
				if (!minifyMode)
				{
					os << string(indentLevel + 1, '\t');
				}
				os << json::serialize(it->key()) << (minifyMode ? ":" : " : ");
				prettyPrint(os, it->value(), minifyMode, indentLevel + 1);
				if(++it == obj.end())
				{
					break;
				}
				os << (minifyMode ? "," : ",\n");
			}
		}
		if (!minifyMode)
		{
			os << "\n" << string(indentLevel, '\t');
		}
		os << "}";
		break;
	}

	case json::kind::array:
	{
		os << (minifyMode ? "[" : "[\n");
		auto const& arr = jv.get_array();
		if(!arr.empty())
		{
			for(auto it = arr.begin();;)
			{
				if (!minifyMode)
				{
					os << string(indentLevel + 1, '\t');
				}
				prettyPrint(os, *it, minifyMode, indentLevel + 1);
				if(++it == arr.end())
				{
					break;
				}
				os << (minifyMode ? "," : ",\n");
			}
		}
		if (!minifyMode)
		{
			os << "\n" << string(indentLevel, '\t');
		}
		os << "]";
		break;
	}

	case json::kind::string:
		os << json::serialize(jv.get_string());
		break;

	case json::kind::uint64:
		os << jv.get_uint64();
		break;

	case json::kind::int64:
		os << jv.get_int64();
		break;

	case json::kind::double_:
		os << jv.get_double();
		break;

	case json::kind::bool_:
		os << (jv.get_bool() ? "true" : "false");
		break;

	case json::kind::null:
		os << "null";
		break;
	}

	if(!minifyMode && indentLevel == 0)
	{
		os << "\n";
	}
}

JsonPP::Path JsonPP::getOutputPath(const Path& filePath, bool minifyMode)
{
	auto stem = filePath.stem().string();
	auto ext = filePath.extension().string();
	auto dir = filePath.parent_path();
	auto fmt = (minifyMode ? "%1%-minified%2%" : "%1%-pretty%2%");
	return dir /= str(b::format(fmt) % stem % ext);
}
