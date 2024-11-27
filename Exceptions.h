
#if !defined(EXCEPTIONS_H_INCLUDED)
#define EXCEPTIONS_H_INCLUDED

#include <boost/format.hpp>

#include <stdexcept>

class CmdLineError : public ::std::logic_error
{
public:
	CmdLineError() : ::std::logic_error("") {}
	CmdLineError(const ::std::string& what) : ::std::logic_error(what) {}
	CmdLineError(const ::boost::format& what) : ::std::logic_error(what.str()) {}
};

class IOError : public ::std::runtime_error
{
public:
	IOError() : ::std::runtime_error("") {}
	IOError(const ::std::string& what) : ::std::runtime_error(what) {}
	IOError(const ::boost::format& what) : ::std::runtime_error(what.str()) {}
};

class SyntaxError : public ::std::runtime_error
{
public:
	SyntaxError() : ::std::runtime_error("") {}
	SyntaxError(const ::std::string& what) : ::std::runtime_error(what) {}
	SyntaxError(const ::boost::format& what) : ::std::runtime_error(what.str()) {}
};

class WindowsError : public ::std::runtime_error
{
public:
	WindowsError() : ::std::runtime_error("") {}
	WindowsError(const ::std::string& what) : ::std::runtime_error(what) {}
	WindowsError(const ::boost::format& what) : ::std::runtime_error(what.str()) {}
};

#endif // EXCEPTIONS_H_INCLUDED
