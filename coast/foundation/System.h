/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _SYSTEM_H
#define _SYSTEM_H

#include "config_foundation.h"
#include "Anything.h"

#if defined(ONLY_STD_IOSTREAM)
#include <iostream>
using namespace std;
#else
#include <iostream.h>
#endif

#include <time.h> // for LocalTime parameters struct tm
#include <sys/stat.h>

#if defined(WIN32)
typedef long pid_t;
// following values were taken from linux <bits/poll.h> file
#define POLLIN		0x001
#define POLLPRI		0x002
#define POLLOUT 	0x004
#define POLLERR		0x008
#define POLLHUP		0x010
#define POLLNVAL	0x020
#else
#include <unistd.h>
#endif

//---- System --------------------------------------------------------------------------
//! API to access system level services like open iostreams to files and reading directory entries
class EXPORTDECL_FOUNDATION System
{
public:
	//! access errno in a portable way, wraps WSAxxerror on windows instead
	static int GetSystemError() ;
	//! determine if system call returned because of a signal interrupt and should be tried again
	static bool SyscallWasInterrupted();

	//! avoid that fd is shared by forked processes
	static void SetCloseOnExec(int fd);

	//!wrapper facade for select(or poll) useful only for single file descriptor
	//! \param fd the file descriptor to be checked (>=0)
	//! \param timeout wait at most timeout miliseconds, 0 return immediatly, <0 block
	//! \param bRead set to true to check for readability
	//! \param bWrite set to true to check for writability
	//! \return 1 if able to read/write, 0 - if timeout, <0 if error
	static int  DoSingleSelect(int fd, long timeout, bool bRead, bool bWrite);

	static bool IsReadyForReading(int fd, long timeout) {
		return 0 < DoSingleSelect(fd, timeout, true, false);
	}
	static bool IsReadyForWriting(int fd, long timeout) {
		return 0 < DoSingleSelect(fd, timeout, false, true);
	}

	//!wrapper facade for using select/Sleep to sleep with microsecond precision
	//! \param sleepTime the number of microseconds
	//! \return false if some error occured, true if slept well
	static bool MicroSleep(long sleepTime);

	//! opens an iostream, uses search path
	//! it takes a filename and an extension. It tries to open an iostream in the given mode. It uses the
	//! fgRootDir and fgPathList variables to search for the file unless it is an absolute filename
	//! \param name the filename, it can be relative or absolute
	//! \param extension the extension of the file
	//! \param mode ios mode flags
	//! \param trace flag if true traces the operation with SysLog::Debug messages
	//! \return the pointer to the open iostream or NULL, the client is responsible for destruction
	static iostream *OpenStream(const char *name, const char *extension, int mode = (ios::in), bool trace = false);

	//! opens an istream for reading, no search path is used
	//! \param name the filename, it can be relative or absolute
	//! \param extension the extension of the file
	//! \param mode ios mode flags
	//! \param trace flag if true traces the operation with SysLog::Debug messages
	//! \return the pointer to the open iostream or NULL, the client is responsible for destruction
	static iostream *OpenIStream(const char *name, const char *extension, int mode = (ios::in), bool trace = false);

	//! opens an ostream for writing, no search path is used
	//! \param name the filename, it can be relative or absolute
	//! \param extension the extension of the file
	//! \param mode ios mode flags
	//! \param trace flag if true traces the operation with SysLog::Debug messages
	//! \return the pointer to the open iostream or NULL, the client is responsible for destruction
	static iostream *OpenOStream(const char *name, const char *extension, int mode = ios::out | ios::trunc, bool trace = false);

	//! returns the path and filename.extension of a file found on the search path
	static String GetFilePath(const char *name, const char *extension);

	//! returns directory entries as anything, entries are filtered by extension filter
	static Anything DirFileList(const char *dir, const char *filter = "html");

	//! loads a config file as anything, returns resolved path
	static bool LoadConfigFile(Anything &config, const char *name, const char *ext, String &realfilename);
	//! load a config file as anything and do not care where from
	static bool LoadConfigFile(Anything &config, const char *name, const char *ext = "any");

	//! this sets the GLOBAL root directory where the search starts
	//! \param rootdir the root directory
	//! \param print if true prints the new settings to cerr
	static void SetRootDir(const char *rootdir, bool print = false);

	//! this sets the GLOBAL pathlist, a ':' delimited list of relative paths ( relative to rootdir )
	//! \param pathlist the pathlist e.g. config:src:test
	//! \param print if true prints the new settings to cerr
	static void SetPathList(const char *pathlist, bool print = false);

	//! returns the current rootdir for this process
	static const char *GetRootDir();

	//! returns the current pathlist for this process
	static const char *GetPathList();

	//! returns the system's path seperator e.g. '/' on unix
	static const char Sep() {
		return System::cSep;
	}

	//!determines if path is a regular file
	//! \param path fully qualified path to file which should be checked
	//! \return true if file is a regular file
	static bool IsRegularFile(const char *path);

	//!determines if path points to a directory
	//! \param path fully qualified path to check
	//! \return true if path is a directory
	static bool IsDirectory(const char *path);

	//!determines the size of a file if it exists, works only for files less than 2^31 bytes for now!
	/*! \param path fully qualified path to file
		\param ulFileSize parameter receiving the file size in bytes
		\return false if the underlying system call failed */
	static bool GetFileSize(const char *path, ul_long &ulFileSize);

	//!determines the number of free blocks on the filesystem pointed to by pFsPath
	/*! \param pFsPath any path within the filesystem to get the free blocks
		\param ulFreeBlocks parameter receiving the number of free blocks
		\param lBlkSize parameter receiving the block size in bytes
		\return false if the underlying system call failed */
	static bool BlocksLeftOnFS(const char *pFsPath, ul_long &ulFreeBlocks, unsigned long &lBlkSize);

	//! resolve full pathname to the given filename
	//! \param fullPathName - contains the full pathname including filename to the given file if successful
	//! \param file - filename including extension of the file to be found
	//! \param path - searchpath to be used for finding the file
	//! \return success of finding the file
	static bool FindFile(String &fullPathName, const char *file, const char *path = 0);

	//!cleans the given path by resolving relative movements
	//! WIN32 special: also corrects drive notation
	//! \param path resolves given path, strips ./ sections, resolves relative movements and unifies slashes
	//! \return path returned through path parameter
	static void ResolvePath(String &path);

	//!returns the drive letter for WIN32
	//! \param name path to get drive letter from
	//! \param drive returned drive letter if any
	//! \return true if the path contained a drive letter
	static bool GetDriveLetter(const char *name, char &drive);

	//!remove drive letter from path
	//! \param path path from which the drive letter including the colon should be removed
	static void StripDriveLetter(String &path);

	//!checks path whether it is absolute or not
	//! \param path fully qualified path to check if absolute
	//! \return true if given path is absolute
	static bool IsAbsolutePath(const char *path);

	//!obtain current working directory
	//! \param cwd variable which takes the current working directory
	static void GetCWD(String &cwd);
	//!set current working directory
	//! \param dir value with the new current working directory
	//! \return true if success
	static bool ChangeDir(const String &dir);

	//!change file or directory permission
	//! \param filename fully qualified path to file whose mode should be adjusted
	//! \param pmode BIT-combination of new mode (see <sys/stat.h> for bitvalues)
	//! \return 0 upon success, -1 otherwise
	static int Chmod(const char *filename, int pmode);

	//!convert given time to local time, *thread safe method (*WIN32 is not 100% safe)
	//! \param timer time value to be converted
	//! \param res pointer to tm variable used for multithreading safety
	//! \return pointer to converted time value, actual it is a pointer to the given res variable
	static struct tm *LocalTime( const time_t *timer, struct tm *res );

	//!convert given time to GMT time, *thread safe method (*WIN32 is not 100% safe)
	//! \param timer time value to be converted
	//! \param res pointer to tm variable used for multithreading safety
	//! \return pointer to converted time value, actual it is a pointer to the given res variable
	static struct tm *GmTime( const time_t *timer, struct tm *res );

	//!convert given time to time string
	//! \param time time value to be converted
	//! \param strTime converted time string
	static void AscTime( const struct tm *time, String &strTime );

	//! get variable of process environment
	//! \param variable environment variable whose value has to be get
	//! \return value of given variable, NULL if variable not found in environment
	static String EnvGet(const char *variable);

	//!auxiliary method to copy the environment from the current process
	//! <B>note:</B> it is considered very unsafe for CGI to use the
	//! environment of the running server, better a sandbox environment
	//! is used
	//! \param anyEnv - the result Anything (we do not use return value, because of possible allocators used)
	static void GetProcessEnvironment(Anything &anyEnv);

	//!initialization routine that sets fgRootDir and fgPathList, does not take parameters ;-) Michael
	//! since OpenStream uses fgRootDir and fgPathList we have to make sure
	//! that they contain reasonable defaults whenever they are used.<P>
	//! fgRootDir is initialized to the environment variable WD_ROOT if set, to '.' otherwise<p>
	//! fgPathList is initialized to the environment variable WD_PATH if set, to ".:config:src" if root is set and to ".:../config:../src:" if root is not set
	//! \param resultPath the location of the iostream opened
	//! \param name the filename, it can be relative or absolute, it contains the extension
	//! \param mode the mode of the stream to be opened e.g. ios::in, mode flags can be combined by the | operation
	//! \return an open iostream or NULL if the open fails
	static void InitPath(const char *root = 0, const char *path = 0);

	//!create new directory with given permissions, works for relative or absolute path names and also
	//! recursive if specified.
	//! \note Unless bRecurse is true, the parent directory must already exist and only one directory level will be created
	//! \param path relative or absolute path to create new directory
	//! \param pmode permission of new directory, octal number
	//! \param bRecurse set to true if nonexisting parent directories should be created
	//! \return true if new directory was created - all other cases will return false
	static bool MakeDirectory(const char *path, int pmode = 0755, bool bRecurse = false);

	//!remove given directory - relative directories can be deleted recursively
	//! \param path relative or absolute path of directory to be deleted
	//! \param bRecurse true if relative directory should be deleted recusrively
	//! \return true if directory could be removed
	static bool RemoveDirectory(const char *path, bool bRecurse = false);

	static bool Uname(Anything &anyInfo);
	static bool HostName(String &name);

	// fork() on Linux and fork1() on Solaris, on Windows return -1 and do nothing
	static long Fork();

	//!get current process id
	//! \return process id of current process
	static pid_t getpid();

	// io specific inner class
	// first some definitions
#if !defined (R_OK)
#define R_OK    04      /* Test for Read permission. */
#endif /* R_OK */
#if !defined (W_OK)
#define W_OK    02      /* Test for Write permission. */
#endif /* W_OK */
#if !defined (X_OK)
#define X_OK    01      /* Test for eXecute permission. */
#endif /* X_OK */
#if !defined (F_OK)
#define F_OK    0       /* Test for existence of File. */
#endif /* F_OK */

#if !defined (S_IRWXU)
#define S_IRWXU		( _S_IREAD | _S_IWRITE | _S_IEXEC )	/* read, write and execute for user */
#endif /* S_IRWXU */

	//! This class is a general wrapper for IO specific functions.
	/*! It is not intended to extend the system specific functionality to handle unsafe conditions or whatever but to present an API which hides
		system specific differences or irregularities. */
	class EXPORTDECL_FOUNDATION IO
	{
	public:
		IO() {}
		~IO() {}

		//! check if a given file or directory is accessible using the given mode
		/*! \param path file or directory path
			\param amode permission setting, one or combination of [F_OK|X_OK|W_OK|R_OK]
			\return 0 in case of success, -1 if file dows not exist or is not accessible in the given mode, check System::GetSystemError */
		static int access(const char *path, int amode);

		//! make a directory, parent directory must exist already
		/*! \param path path to be created, must not contain trailing slash
			\param pmode permission mask of new directory
			\return 0 in case of success, -1 if something went wrong, check System::GetSystemError */
		static int mkdir(const char *path, int pmode);

		//! remove a directory
		/*! \param path specify path to delete
			\return 0 in case of success, -1 if something went wrong, check System::GetSystemError */
		static int rmdir(const char *path);

		//! remove a file or link
		/*! \param path specify filename and path of file to delete
			\return 0 in case of success, -1 if something went wrong, check System::GetSystemError */
		static int unlink(const char *path);

		//! rename a file, path or link
		/*! \param oldfilename specify name of old file, path or link
			\param newfilename specify name of new file, path or link
			\return 0 in case of success, -1 if something went wrong, check System::GetSystemError */
		static int rename(const char *oldfilename, const char *newfilename);
	};

private:
	friend class SystemTest;

	//!checks existence of a path using stat
	static bool CheckPath(const char *path, struct stat *stbuf);

	//!internal method that opens a stream if possible
	//! \param resultPath the location of the iostream opened
	//! \param search flag if true searches pathlist until a stream can be opened
	//! \param name the filename, it can be relative or absolute
	//! \param extension the extension of the file
	//! \param mode the mode of the stream to be opened e.g. ios::in, mode flags can be combined by the | operation
	//! \param trace if true writes messages to SysLog
	//! \return an open iostream or NULL if the open fails
	static iostream *IntOpenStream(String &resultPath, const char *name, const char *extension, bool search, int mode, bool trace = false);

	//!internal method that opens a stream if possible, it searches the pathlist for the correct location
	//! \param resultPath the location of the iostream opened
	//! \param pathlist the pathlist to be searched,relative to the root directory
	//! \param name the filename, it can be relative or absolute, it contains the extension
	//! \param mode the mode of the stream to be opened e.g. ios::in, mode flags can be combined by the | operation
	//! \param trace if true writes messages to SysLog
	//! \return an open iostream or NULL if the open fails
	static iostream *IntOpenStreamBySearch(String &resultPath, const char *name, const char *pathlist, int mode, bool trace = false);

	//!bottleneck routine that opens the stream if possible it returns null if not successful
	//! \param resultPath the location of the iostream opened
	//! \param name the filename, it can be relative or absolute, it contains the extension
	//! \param mode the mode of the stream to be opened e.g. ios::in, mode flags can be combined by the | operation
	//! \param trace if true writes messages to SysLog
	//! \return an open iostream or NULL if the open fails
	static iostream *DoOpenStream(String &resultPath, const char *name, int mode, bool trace = false);

	//!internal method to create new directory with given permissions, works for relative or absolute path names and also
	//! recursive if specified.
	//! \note Unless bRecurse is true, the parent directory must already exist and only one directory level will be created
	//! \param path relative or absolute path to create new directory
	//! \param pmode permission of new directory, octal number
	//! \param bRecurse set to true if nonexisting parent directories should be created
	//! \return true if new directory was created - all other cases will return false
	static bool IntMakeDirectory(String dir, int pmode, bool bRecurse);

	//!internal method to remove given directory - relative directories can be deleted recursively
	//! \param dir relative or absolute path of directory to be deleted
	//! \param bRecurse true if relative directory should be deleted recusrively
	//! \param bAbsDir if true we can not recurse to delete directories
	//! \return true if directory could be removed
	static bool IntRemoveDirectory(String dir, bool bRecurse, bool bAbsDir);

	//!contains the root directory path that is used to locate files, it can be relative or absolute
	static String fgRootDir;

	//!contains a search path list that is ':' delimited, it is used to search for files
	static String fgPathList;

	//!contains the systems path seperator
	static const char cSep;

	//!defines maximal size of path
	static const long cPATH_MAX;
};

#endif
