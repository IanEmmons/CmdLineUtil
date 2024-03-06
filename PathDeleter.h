
#if !defined(FILEDELETER_H_INCLUDED)
#define FILEDELETER_H_INCLUDED

#include <filesystem>
#include <format>
#include <iostream>

class PathDeleter
{
public:
	using Path = ::std::filesystem::path;

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
			catch (const ::std::filesystem::filesystem_error& ex)
			{
				if (ex.code() != make_error_code(::std::errc::no_such_file_or_directory))
				{
					::std::cout << ::std::format("Unable to delete \"{0}\":  {1} ({2})",
						m_path.string(), ex.what(), ex.code().value()) << ::std::endl;
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
