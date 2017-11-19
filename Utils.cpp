
#include "Utils.h"
#include "Exceptions.h"

#include <boost/format.hpp>

namespace bfs = ::boost::filesystem;

using ::boost::format;
using ::std::string;

bfs::path getTempPath(const bfs::path& filePath)
{
	static const size_t k_maxIterations = 99999;

	string stem = filePath.stem().string();
	string ext = filePath.extension().string();
	bfs::path dir(filePath.parent_path());
	for (size_t i = 0; i < k_maxIterations; ++i)
	{
		bfs::path tempPath(dir);
		tempPath /= str(format("%1%-%2$05d%3%") % stem % i % ext);
		if (!exists(tempPath))
		{
			return tempPath;
		}
	}
	throw IOError(format("Unable to create temporary file for \"%1%\"") % filePath);
}
