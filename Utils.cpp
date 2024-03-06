
#include "Utils.h"
#include "Exceptions.h"

#include <format>

namespace fs = ::std::filesystem;

using ::std::format;
using ::std::string;

fs::path getTempPath(const fs::path& filePath)
{
	static const size_t k_maxIterations = 99999;

	string stem = filePath.stem().string();
	string ext = filePath.extension().string();
	fs::path dir(filePath.parent_path());
	for (size_t i = 0; i < k_maxIterations; ++i)
	{
		fs::path tempPath(dir);
		tempPath /= format("{0}-{1:05}{2}", stem, i,  ext);
		if (!exists(tempPath))
		{
			return tempPath;
		}
	}
	throw IOError(format("Unable to create temporary file for '{0}'",
		filePath.generic_string()));
}
