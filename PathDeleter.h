
#if !defined(FILEDELETER_H_INCLUDED)
#define FILEDELETER_H_INCLUDED

#include <boost/filesystem.hpp>
#include <boost/format.hpp>
#include <iostream>

class PathDeleter
{
public:
	using Path = ::boost::filesystem::path;

	PathDeleter(const Path& p)
		: m_path(p), m_ownershipReleased(false) {}

	~PathDeleter()
	{
		if (!m_ownershipReleased && exists(m_path))
		{
			try
			{
				if (is_directory(m_path))
				{
					remove_all(m_path);
				}
				else
				{
					remove(m_path);
				}
			}
			catch (const ::boost::filesystem::filesystem_error& ex)
			{
				if (ex.code().value() != ::boost::system::errc::no_such_file_or_directory)
				{
					::boost::format fmt("Unable to delete \"%1%\":  %2% (%3%)");
					fmt % m_path.string() % ex.what() % ex.code().value();
					::std::cout << fmt << ::std::endl;
				}
			}
		}
	}

	void releaseOwnership()
		{ m_ownershipReleased = true; }

	PathDeleter(const PathDeleter&) = delete;
	PathDeleter& operator=(const PathDeleter&) = delete;
	PathDeleter(PathDeleter&&) = delete;
	PathDeleter& operator=(PathDeleter&&) = delete;

private:
	const Path	m_path;
	bool			m_ownershipReleased;
};

#endif // FILEDELETER_H_INCLUDED
