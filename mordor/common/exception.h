#ifndef __EXCEPTION_H__
#define __EXCEPTION_H__
// Copyright (c) 2009 - Decho Corp.

#include <stdexcept>
#include <string>

#include "version.h"

class NestedException : public std::exception
{
public:
    NestedException(std::exception &inner)
        : m_inner(&inner)
    {}

    std::exception &inner() { return *m_inner; }
    const char *what() const throw () { return m_inner->what(); }

private:
    std::exception *m_inner;
};

class StreamError : public std::runtime_error
{
public:
    StreamError() : std::runtime_error("") {}
};

class UnexpectedEofError : public StreamError
{
public:
    UnexpectedEofError() {}
};

class BufferOverflowError : public StreamError
{
public:
    BufferOverflowError() {}
};

#ifdef WINDOWS
#include <winerror.h>
class Win32Error : public std::runtime_error
{
public:
    Win32Error(unsigned int lastError, const char *function = NULL);

    unsigned int error() const { return m_lastError; }
    const char *function() const { return m_function; }

private:
    static std::string constructMessage(unsigned int lastError);
private:
    int m_lastError;
    const char *m_function;
};
typedef Win32Error NativeError;
typedef unsigned int error_t;
#else
#include <errno.h>
class ErrnoError : public std::runtime_error
{
public:
    ErrnoError(int error, const char *function = NULL);
    ~ErrnoError() throw() {}

    int error() const { return m_error; }
    const char *function() const { return m_function; }

private:
    static std::string constructMessage(int error);
private:
    int m_error;
    const char *m_function;
};
typedef ErrnoError NativeError;
typedef int error_t;
#endif

#ifdef WINDOWS
#define CREATE_NATIVE_EXCEPTION(Name, win32error, errnoerror)                   \
    class Name ## Exception : public NativeError                                \
    {                                                                           \
    public:                                                                     \
        Name ## Exception(const char *function = NULL)                          \
        : NativeError(win32error, function) {}                                  \
    };
#else
#define CREATE_NATIVE_EXCEPTION(Name, win32error, errnoerror)                   \
    class Name ## Exception : public NativeError                                \
    {                                                                           \
    public:                                                                     \
        Name ## Exception(const char *function = NULL)                          \
        : NativeError(errnoerror, function) {}                                  \
    };
#endif

CREATE_NATIVE_EXCEPTION(FileNotFound, ERROR_FILE_NOT_FOUND, ENOENT);
CREATE_NATIVE_EXCEPTION(BadHandle, ERROR_INVALID_HANDLE, EBADF);
CREATE_NATIVE_EXCEPTION(OperationAborted, ERROR_OPERATION_ABORTED, ECANCELED);
CREATE_NATIVE_EXCEPTION(BrokenPipe, WSAESHUTDOWN, EPIPE);

#undef CREATE_NATIVE_EXCEPTION

class SocketException : public NativeError
{
public:
    SocketException(error_t lastError, const char *function = NULL)
    : NativeError(lastError, function) {}
};

#ifdef WINDOWS
#define WSA(error) WSA ## error
#else
#define WSA(error) error
#endif

#define CREATE_SOCKET_EXCEPTION(Name, errnoerror)                               \
    class Name ## Exception : public SocketException                            \
    {                                                                           \
    public:                                                                     \
        Name ## Exception(const char *function)                                 \
        : SocketException(WSA(errnoerror), function) {}                         \
    };

CREATE_SOCKET_EXCEPTION(ConnectionAborted, ECONNABORTED);
CREATE_SOCKET_EXCEPTION(ConnectionReset, ECONNRESET);
CREATE_SOCKET_EXCEPTION(TimedOut, ETIMEDOUT);

#undef CREATE_SOCKET_EXCEPTION

error_t lastError();
void throwExceptionFromLastError(const char *function = NULL);
void throwExceptionFromLastError(error_t lastError, const char *function = NULL);

#endif
