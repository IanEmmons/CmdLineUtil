
#if !defined(EXCEPTIONS_H_INCLUDED)
#define EXCEPTIONS_H_INCLUDED

#include <stdexcept>

class CmdLineError : public ::std::logic_error
{
public:
	CmdLineError() : ::std::logic_error("") {}
	CmdLineError(const ::std::string& what) : ::std::logic_error(what) {}
};

class IOError : public ::std::runtime_error
{
public:
	IOError() : ::std::runtime_error("") {}
	IOError(const ::std::string& what) : ::std::runtime_error(what) {}
};

class SyntaxError : public ::std::runtime_error
{
public:
	SyntaxError() : ::std::runtime_error("") {}
	SyntaxError(const ::std::string& what) : ::std::runtime_error(what) {}
};

class WindowsError : public ::std::runtime_error
{
public:
	WindowsError() : ::std::runtime_error("") {}
	WindowsError(const ::std::string& what) : ::std::runtime_error(what) {}
};

#endif // EXCEPTIONS_H_INCLUDED
