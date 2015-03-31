/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _SYSTEMBASE_H
#define _SYSTEMBASE_H

class String;
class Anything;

#include <time.h> //lint !e451//lint !e537// for LocalTime parameters struct tm
#include <sys/types.h>
#if defined(WIN32)
	typedef long uid_t;
	// following values were taken from linux <bits/poll.h> file
	#define POLLIN		0x001
	#define POLLPRI		0x002
	#define POLLOUT 	0x004
	#define POLLERR		0x008
	#define POLLHUP		0x010
	#define POLLNVAL	0x020
	// the following don't exist on windows
	#define	S_IXGRP		_S_IEXEC
	#define	S_IWGRP		_S_IWRITE
	#define	S_IRGRP		_S_IREAD
	#define	S_IXOTH		_S_IEXEC
	#define	S_IWOTH		_S_IWRITE
	#define	S_IROTH		_S_IREAD
#endif

namespace coast {
	namespace system {
		//! access errno in a portable way, wraps WSAxxerror on windows instead
		/*! \return error code which should not be used to compare with error constants
		            because on windows the values from errno and GetLastError are overlapping. */
		int GetSystemError();

		//! determine if system call returned because of a signal interrupt and should be tried again
		bool SyscallWasInterrupted();

		//! avoid that fd is shared by forked processes
		void SetCloseOnExec(int fd);

		//!wrapper facade for select(or poll) useful only for single file descriptor
		/*! \param fd the file descriptor to be checked (>=0)
			\param timeout wait at most timeout miliseconds, 0 return immediatly, <0 block
			\param bRead set to true to check for readability
			\param bWrite set to true to check for writability
			\return 1 if able to read/write, 0 - if timeout, <0 if error */
		int  DoSingleSelect(int fd, long timeout, bool bRead, bool bWrite);

		bool IsReadyForReading(int fd, long timeout);
		bool IsReadyForWriting(int fd, long timeout);

		//! wrapper facade for using select/Sleep to sleep with microsecond precision
		/*! \param sleepTime the number of microseconds
			\return false if some error occured, true if slept well */
		bool MicroSleep(long sleepTime);

		//! convert given time to local time, *thread safe method (*WIN32 is not 100% safe)
		/*! \param timer time value to be converted
			\param res pointer to tm variable used for multithreading safety
			\return pointer to converted time value, actual it is a pointer to the given res variable */
		struct tm *LocalTime( const time_t *timer, struct tm *res );

		//! convert given time to GMT time, *thread safe method (*WIN32 is not 100% safe)
		/*! \param timer time value to be converted
			\param res pointer to tm variable used for multithreading safety
			\return pointer to converted time value, actual it is a pointer to the given res variable */
		struct tm *GmTime( const time_t *timer, struct tm *res );

		//! convert given time to time string
		/*! \param time time value to be converted
			\param strTime converted time string */
		void AscTime( const struct tm *time, String &strTime );

		//! get variable of process environment
		/*! \param variable environment variable whose value has to be get
			\return value of given variable, NULL if variable not found in environment */
		String EnvGet(const char *variable);

		//! auxiliary method to copy the environment from the current process
		/*! \note: it is considered very unsafe for CGI to use the environment of the running server, better a sandbox environment is used
			\param anyEnv - the result Anything (we do not use return value, because of possible allocators used) */
		void GetProcessEnvironment(Anything &anyEnv);

		bool Uname(Anything &anyInfo);
		bool HostName(String &name);

		//! fork() on Linux and fork1() on Solaris, on Windows return -1 and do nothing
		long Fork();

		//! get current process id
		/*! \return process id of current process */
		pid_t getpid();

		//! get current user id - only for unix/linux implemented!
		/*! \return user id of current process */
		uid_t getuid();

		//! Writes the results to a character string buffer.
		/*! \note At most buf_size - 1 characters are written. The resulting character string
		 * will always be terminated with a null character, unless buf_size is zero.
		 * \param buf pointer to a character string to write to
		 * \param buf_size up to buf_size - 1 characters may be written, plus the null terminator
		 * \param format pointer to a null-terminated multibyte string specifying how to interpret the data
		 * \return Upon successful return, these functions return the number of characters printed
		 * (excluding the null byte used to end output to strings). If the output was truncated due to the buf_size limit,
		 * then the return value is the number of characters (excluding the terminating null byte)
		 * which would have been written to the final string if enough space had been available.
		 * Thus, a return value of size or more means that the output was truncated.
		 */
		int SnPrintf(char * buf, size_t buf_size, const char *format, ...);

		bool StrToL(long & value, const char* str, int base = 10);
		bool StrToUL(unsigned long & value, const char* str, int base = 10);
		bool StrToLL(long long & value, const char* str, int base = 10);
		bool StrToULL(unsigned long long & value, const char* str, int base = 10);
		bool StrToD(double& value, const char* str);

#if !defined(WIN32)
		//! Get the state of the lock file.
		/*! \param lockFileName file name to get lock state of
			\return false=not locked, true=locked. If there was an error, the file is considered to be locked! You must remove the lockfile with coast::system::unlink(lockFileName) after you're done. */
		bool GetLockFileState(const char *lockFileName);
#endif
	}
}
#endif /* _SYSTEMBASE_H */
