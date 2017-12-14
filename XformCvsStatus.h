
#if !defined(XFORMCVSSTATUS_H_INCLUDED)
#define XFORMCVSSTATUS_H_INCLUDED

#include <boost/filesystem.hpp>
#include <gsl/gsl>
#include <iosfwd>
#include <map>
#include <string>

class XformCvsStatus
{
public:
	static int usage(::std::ostream& strm, const ::std::string& progName,
		const char* pMsg);

	XformCvsStatus(::gsl::span<const char*const> args);
	int run();

	XformCvsStatus(const XformCvsStatus&) = delete;
	XformCvsStatus& operator=(const XformCvsStatus&) = delete;
	XformCvsStatus(XformCvsStatus&&) = delete;
	XformCvsStatus& operator=(XformCvsStatus&&) = delete;

private:
	using Path = ::boost::filesystem::path;
	using FileToStatusMap = ::std::map<Path, ::std::string>;

	void insertInMap(const ::std::string& file, const ::std::string& status);
	Path buildPath(const ::std::string& file) const;
	static ::std::string xlateStatus(const ::std::string& status);
	static ::std::string pathStringToConsoleString(const Path::string_type& str);

	bool					m_suppressNew;
	bool					m_suppressUpToDate;
	bool					m_suppressLocal;
	Path					m_lastWorkingDir;
	FileToStatusMap		m_map;
};

#endif // XFORMCVSSTATUS_H_INCLUDED
