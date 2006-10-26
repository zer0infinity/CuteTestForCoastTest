/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

//--- interface include --------------------------------------------------------
#include "System.h"
#include "MmapStream.h"

#if defined(ONLY_STD_IOSTREAM)
#include <fstream>
using namespace std;
#else
#include <fstream.h>
#endif

//--- standard modules used ----------------------------------------------------
#include "SysLog.h"
#include "Dbg.h"

//--- c-library modules used ---------------------------------------------------
#include <stdio.h>
#include <limits.h>
#include <errno.h>

#if defined(WIN32)
#include <io.h>
#include <direct.h>
#else
#include <fcntl.h>
#include <dirent.h>  // directory access
#include <sys/time.h>
#include <string.h>
#include <sys/utsname.h>
#if defined(__linux__)
#include <sys/vfs.h>
#elif defined(__sun)
#include <sys/statvfs.h>
#endif
#endif

//---- System --------------------------------------------------------------------------
const char System::cSep = '/';
#if defined(WIN32) && !defined(_POSIX_)
#define PATH_MAX	512
#endif
#if defined(_POSIX_)
const long System::cPATH_MAX = _POSIX_PATH_MAX;
#else
const long System::cPATH_MAX = PATH_MAX;
#endif

bool System::SyscallWasInterrupted()
{
#if defined(WIN32)
	return ( WSAGetLastError () == WSAEINTR );
#else
	return ( errno == EINTR ) || (errno == EAGAIN);
#endif
}

// avoid that fd is shared by forked processes
void System::SetCloseOnExec(int fd)
{
#if !defined WIN32
	if (fd >= 0 ) {
		int options = fcntl(fd, F_GETFD);
		if (options >= 0) {
			fcntl(fd, F_SETFD, options | FD_CLOEXEC);
		}
	}
#endif
}

int System::DoSingleSelect(int fd, long timeout, bool bRead, bool bWrite)
{
	StartTrace1(System.DoSingleSelect, "testing fd:" << (long)fd << "(" << (bRead ? "r" : "-") << (bWrite ? "w" : "-") << ") with timeout:" << timeout << "ms");
//NB: solaris select is limited to 1024 file descriptors, poll doesn't carry this burden
// compile time settings might increase that up to 65536:
// see `man -s 3C select`

	if (fd < 0 ) {
		return -1;
	}

// SOP: alternative implementation using poll, needed ?
//	pollfd fds[1];
//	fds[0].fd= fd;
//	fds[0].events= (bRead?POLLIN:0)|(bWrite?POLLOUT:0);
//	return ::poll( fds, 1, timeout);

	fd_set readfds;
	fd_set writefds;
	fd_set exceptfds;
	timeval t;

	// initialize the descriptor sets
	FD_ZERO(&readfds);
	FD_ZERO(&writefds);
	FD_ZERO(&exceptfds);

	// set the descriptor sets
	if (bRead) {
		FD_SET(fd, &readfds);
	} else if (bWrite) {
		FD_SET(fd, &writefds);
	}
	t.tv_sec = timeout / 1000;
	t.tv_usec = 1000 * (timeout % 1000);

	bool bArg(bRead || bWrite);
	// FD_SETSIZE is ignored in WIN32 select call, on Ux systems it specifies the number of valid fds in the set
	return ::select (	FD_SETSIZE,
						bRead ? &readfds   : NULL,
						bWrite ? &writefds  : NULL,
						bArg  ? &exceptfds : NULL,
						timeout >= 0 ? &t : 0);
//		#ifdef __370__
//			int sockFd[1];
//			sockFd[0] = (int)fd;
//			if( event == POLLIN )
//			retCode = select(sockFd,1,0,0,timeout);
//
//			else  if( event == POLLOUT )
//			retCode = select(sockFd,0,1,0,timeout);
}

bool System::MicroSleep(long sleepTime)
{
#if defined(WIN32)
	Sleep(sleepTime / 1000L);
	// might work: WaitForMultipleObjects(0,0,true,sleepTime);
	// select won't work on Windows, according to WinSock2 documentation
	return true;
#else
	// alternative is to use Posix's nanosleep(struct timespec*,struct timespec*)
	struct timeval tv;
	const long MILLION = 1000000L;
	tv.tv_sec = sleepTime / MILLION;
	tv.tv_usec = (sleepTime % MILLION);
	return 0 == select(0, 0, 0, 0, &tv);
#endif
}

int System::GetSystemError()
{
#if defined(WIN32)
	// either it was a regular system error or a windows socket error
	if (GetLastError()) {
		return GetLastError();
	}
	if (WSAGetLastError()) {
		return WSAGetLastError();
	}
	return errno;
#else
	return errno;
#endif
}

// used to smoothify the given path;
// remove relative movements
// on WIN32, convert to correct drive notation and use only one kind of slashes
void System::ResolvePath(String &path)
{
	String newPath = "";
	if (!path.Length()) {
		return;
	}
	// remember where the real path segmnent starts, needed especially for WIN32
	long rootStart = 0;
	String drive;
#if defined(WIN32)
	// get drive letter if any specified
	char drv;
	if (GetDriveLetter(path, drv)) {
		drive.Append(drv);
		drive.Append(':');
		StripDriveLetter(path);
	}
#endif

	if (IsAbsolutePath(path)) {
		rootStart = 1;
		newPath << '/';
	}

#if defined(WIN32)
	// only use one kind of slashes, this simplifies the following code
	long sidx;
	while ((sidx = path.StrChr('\\')) != -1) {
		path.PutAt(sidx, '/');
	}
#endif

	// process the given path, resolve all relative path movements
	// and use it to create the new path
	String token, nxtoken;
	StringTokenizer myTok(path, '/');
	bool boTok = myTok(token);
	while (boTok) {
		boTok = myTok(nxtoken);
		if (token == ".") {
			// skip current path tokens
		} else if (token == "..") {
			long idx = newPath.StrRChr('/');
			if (idx == 0) {
				// do not go beyond root, trim to first slash
				// root=/, move=../c should yield /c
				newPath.Trim(1);
			} else if (idx > 0) {
				// must be a valid path segment which we can remove
				// just remove the last path segment because the back movement made it obsolete
				newPath.Trim(idx);
			} else {
				// probably no slash found, seems to be a relative path
				// reset the path because the back movement made it obsolete
				if (newPath.Length()) {
					newPath = "";
				}
			}
		} else if (!token.Length()) {
			// skip empty token
			if (!boTok && newPath.Length() && newPath[newPath.Length()-1L] != '/') {
				newPath << '/';
			}
		} else {
			// regular token
			if (newPath.Length() && newPath[newPath.Length()-1L] != '/') {
				newPath.Append('/');
			}
			newPath.Append(token);
		}
		token = nxtoken;
		nxtoken = "";
	}
	if (drive.Length()) {
		// prepend drive letter if any
		drive << newPath;
		newPath = drive;
	}
	path = newPath;
}

void System::StripDriveLetter(String &path)
{
#if defined(WIN32)
	// <drive-letter:>, standard NT syscalls return this notation
	// <//drive-letter>, some U*x-shells on WIN32 use this notation
	if ((path.Length() > 1) && isalpha(path[0L]) && path[1L] == ':') {
		path.TrimFront(2);
	} else if ((path.Length() > 2) && path[0L] == '/' && path[1L] == '/' && isalpha(path[2L])) {
		path.TrimFront(3);
	}
#endif
}

bool System::GetDriveLetter(const char *name, char &drive)
{
#if defined(WIN32)
	// <drive-letter:>, standard NT syscalls return this notation
	// <//drive-letter>, some U*x-shells on WIN32 use this notation
	String str(name);
	if ((str.Length() > 1) && isalpha(str[0L]) && str[1L] == ':') {
		drive = str[0L];
		return true;
	} else if ((str.Length() > 2) && str[0L] == '/' && str[1L] == '/' && isalpha(str[2L])) {
		drive = str[2L];
		return true;
	}
#endif
	return false;
}

bool System::IsAbsolutePath(const char *name)
{
	if (name) {
		// check for absolute path names
#if defined(WIN32)
		// following absolute paths could be possible on WIN32
		// <drive-letter:\>, standard NT syscalls return this notation
		// <//drive-letter/>, some U*x-shells on WIN32 use this notation
		// <\> and </> is absolute too
		if ( ((strlen(name) > 2) && isalpha(name[0]) && name[1] == ':' && (name[2] == '/' || name[2] == '\\')) ||
			 ((strlen(name) > 3) && name[0] == '/' && name[1] == '/' && isalpha(name[2]) && name[3] == '/') ||
			 (*name) == '\\' ||
			 (*name) == '/') {
			// under WIN32 it might start with or
			// without <drive-letter:>
			return true;
		}
#else
		if ( (*name) == System::Sep() ) {
			// it is an absolute path name
			return true;
		}
#endif
	}
	return false;
}

void System::GetCWD(String &cwd)
{
	String buf(4096L);
	cwd = ".";
	if (!getcwd((char *)(const char *)buf, buf.Capacity())) {
		SysLog::Alert("puh, cannot obtain current working directory" );
		return;
	} else {
		cwd = String((void *)(const char *)buf, strlen((const char *)buf));
		System::ResolvePath(cwd);
	}
}

bool System::ChangeDir(const String &dir)
{
	if (dir.Length() > 0 && dir != ".") {
		if (!System::IsDirectory(dir) || chdir(dir) != 0) {
			return false;
		}
	}
	return true;
}

int System::Chmod(const char *filename, int pmode)
{
#if defined(WIN32)
	return _chmod(filename, pmode);
#else
	return ::chmod(filename, pmode);
#endif
}

struct tm *System::LocalTime( const time_t *timer, struct tm *res ) {
#if defined(WIN32)
	// FIXME not 100% thread safe, localtime uses a global variable to store its content
	// subsequent calls will overwrite the value
	(*res) = (*::localtime(timer));
	return res;
#else
	return ::localtime_r(timer, res);
#endif
}

struct tm *System::GmTime( const time_t *timer, struct tm *res ) {
#if defined(WIN32)
	// FIXME not 100% thread safe, localtime uses a global variable to store its content
	// subsequent calls will overwrite the value
	(*res) = (*::localtime(timer));
	return res;
#else
	return ::gmtime_r(timer, res);
#endif
}

void System::AscTime( const struct tm *pTime, String &strTime )
{
#if defined WIN32
	strTime = asctime( pTime );
#else
	char timeBuf[255];
	asctime_r(pTime, timeBuf);
	strTime = timeBuf;
#endif
}

String System::EnvGet(const char *variable)
{
	if (variable) {
		return String(getenv(variable));
	} else {
		return String();
	}
}

void System::GetProcessEnvironment(Anything &anyEnv)
{
	StartTrace(System.GetProcessEnvironment);
#if defined(WIN32)
	extern char **_environ;
	for (long i = 0; _environ && _environ[i] ; ++i) {
		String entry(_environ[i]);
#else
	extern char **environ;
	for (long i = 0; environ && environ[i] ; ++i) {
		String entry(environ[i]);
#endif
		Trace("entry = <" << entry << ">");
		long equalsign = entry.StrChr('=');
		if (equalsign >= 0) {
			String key = entry.SubString(0L, equalsign);
			Trace("key = <" << key << ">");
			String value = entry.SubString(equalsign + 1);
			Trace("value = <" << value << ">");
			if (key.Length() > 0) {
				anyEnv[key] = value;
			}
		}
	}
}

//---- System --------------------------------------------------------------------------
String System::fgRootDir(0L, Storage::Global());
String System::fgPathList(0L, Storage::Global());

void System::InitPath(const char *root, const char *path)
{
	static bool once = false;
	String tmpRoot((root) ? String(root) : System::EnvGet("WD_ROOT"));
	String tmpPath((path) ? String(path) : System::EnvGet("WD_PATH"));

	if (!once || tmpRoot.Length() > 0 || tmpPath.Length() > 0) {
		once = true;
		if (tmpRoot.Length() > 0) {
			System::SetRootDir(tmpRoot, true);
		} else {
			System::SetRootDir(".", true);
		}
		if (tmpPath.Length() > 0) {
			System::SetPathList(tmpPath, true);
		} else {
			System::SetPathList(".:config:src:", true);
		}
	}
}

void System::SetRootDir(const char *root, bool print)
{
	if (root) {
		String rPath(root);
		// when a dot is supplied as root, ResolvePath would remove it and leave the path blank
		if (rPath != ".") {
			System::ResolvePath(rPath);
		}
		if (print) {
			SysLog::WriteToStderr( String("Root Dir: <") << rPath << ">" << "\n" );
		}

		String logMsg("Root Dir: <");
		logMsg << rPath << ">";
		SysLog::Info(logMsg);

		fgRootDir.SetAllocator(Storage::Global());
		fgRootDir = rPath;
	}
}

void System::SetPathList(const char *pathlist, bool print)
{
	if (pathlist) {
		if (print) {
			SysLog::WriteToStderr( String("Pathlist: <") << pathlist << ">" << "\n" );
		}

		String logMsg("Pathlist: <");
		logMsg << pathlist << ">";
		SysLog::Info(logMsg);

		fgPathList.SetAllocator(Storage::Global());
		fgPathList = pathlist;
	}
}

iostream *System::OpenIStream(const char *name, const char *extension, openmode mode, bool trace)
{
	String dummy;
	return IntOpenStream(dummy, name, extension, false, mode | ios::in, trace);
}

iostream *System::OpenOStream(const char *name, const char *extension, openmode mode, bool trace)
{
	String dummy;
	return IntOpenStream(dummy, name, extension, false, mode | ios::out, trace);
}

iostream *System::OpenStream(const char *name, const char *extension, openmode mode, bool trace)
{
	String dummy;
	return IntOpenStream(dummy, name, extension, true, mode, trace);
}

String System::GetFilePath(const char *name, const char *extension)
{
	String path;
	iostream *Ios = IntOpenStream(path, name, extension, true, ios::in);
	if (Ios) {
		delete Ios;
	}

	return path;
}

// name is already resolved or is absolute
// this method returns only a iostream *
// if the stream exists otherwise null
iostream *System::DoOpenStream(String &resultPath, const char *name, openmode mode, bool trace)
{
	StartTrace(System.DoOpenStream);
	// adjust mode to output, append implies it
	if ( mode & (ios::app | ios::ate) ) {
		mode |= ios::out;
	}
	// clear out flag if ate mode given
	// -> needed because some fstream implementations create the files even they should not!
	if ( mode & ios::ate ) {
		mode &= ~ios::out;
	}

	if ( IsRegularFile(name) || (mode & ios::out) ) {
		// ios::ate is special, it should only work on existing files according to standard
		// so must set the out flag here and not earlier
		if ( mode & ios::ate ) {
			mode |= ios::out;
		}
		static bool bUseMmapStreams = ( System::EnvGet("WD_USE_MMAP_STREAMS").AsLong(1L) == 1L );
		if ( bUseMmapStreams ) {
			MmapStream *fp = new MmapStream(name, mode);
			if ( fp->good() && fp->rdbuf()->is_open() ) {
				resultPath = name;
				return fp;
			}
			Trace("failed to open MmapStream, file [" << name << "]");
			delete fp;
		} else {
			fstream *fp = new fstream(name, (openmode)mode);
			if ( fp->good() && fp->rdbuf()->is_open() ) {
				resultPath = name;
				return fp;
			}
			Trace("failed to open fstream. file [" << name << "]");
			delete fp;
		}
	} else {
		if (trace) {
			SYSWARNING(String("file [") << name << "] does not exist or is not accessible");
		}
	}
	return 0;
}

iostream *System::IntOpenStream(String &resultPath, const char *name, const char *extension, bool searchPath, openmode mode, bool trace)
{
	StartTrace(System.IntOpenStream);
	// fast exit
	if ( name == 0 ) {
		return 0 ;
	}

	// init the rootdir and pathlist environment
	// if necessary
	if ( fgPathList.Length() == 0 ) {
		InitPath();
	}

	// we should use stat to look for the file prior to use

	String filename(name);
	if (extension && strlen(extension) > 0) {
		filename.Append('.').Append(extension);
	}

	iostream *Ios = 0;
	if (System::IsAbsolutePath(filename) || !searchPath) {
		if (System::IsAbsolutePath(filename) && trace) {
			SYSDEBUG(String("file [") << filename << "] is going to be opened absolute");
		} else if (!searchPath && trace) {
			SYSDEBUG(String("file [") << filename << "] is going to be opened relative");
		}
		Ios = DoOpenStream(resultPath, filename, mode, trace);
	} else {
		if (trace) {
			SYSDEBUG(String("file [") << filename << "] is going to be searched in [" << fgRootDir << "] with pathlist: [" << fgPathList << "]");
		}
		Ios = IntOpenStreamBySearch(resultPath, filename, fgPathList, mode, trace);
	}

	if ( trace ) {
		String logMsg;
		if ( Ios ) {
			logMsg << resultPath << " found";
		} else {
			logMsg << "can't open " << filename;
		}
		SYSDEBUG(logMsg);
	}
	if ( Ios == NULL ) {
		SYSWARNING(String("couldn't open file [") << filename << "] searched in [" << fgRootDir << "] with pathlist: [" << fgPathList << "]");
	}
	System::ResolvePath(resultPath);
	return Ios;
}

bool System::CheckPath(const char *path, struct stat *stbuf)
{
	StartTrace(System.CheckPath);
	bool result = false;
	while ( !(result = (stat(path, stbuf) == 0)) && SyscallWasInterrupted() ) {
		String msg("OOPS, stat failed with ");
		msg << SysLog::LastSysError() << " on " << path;
		Trace(msg);
		SYSWARNING(msg);
	}
	return result;
}

bool System::FindFile(String &fullPathName, const char *file, const char *path)
{
	StartTrace(System.FindFile);
	bool bRet = false;
#if defined(WIN32)
	char *pNamePos = NULL;
	String envPATH;
	if (path == NULL) {
		// no search path given, default is to use PATH variable
		envPATH = EnvGet("PATH");
		Trace("using sysPath [" << envPATH << "]");
		path = (const char *)envPATH;
	}
	// returns needed buffersize exclusive terminating null
	long lBufSize = SearchPath(
						path,		// search path, if NULL the system searches the file
						file,		// file name including extension
						NULL,		// extension, we do not use this notation
						0,			// buffersize for the resulting path, let the function tell us how long it is
						NULL,		// char* to buffer
						&pNamePos	// pointer to position of filename within buffer
					);
	if (lBufSize > 0) {
		Trace("buffersize needed:" << lBufSize);
		char *buffer = new char[lBufSize++];
		// and again with the allocated buffer
		lBufSize = SearchPath(
					   path,		// search path, if NULL the system searches the file
					   file,		// file name including extension
					   NULL,		// extension, we do not use this notation
					   lBufSize,	// buffersize for the resulting path, let the function tell us how long it is
					   buffer,		// char* to buffer
					   &pNamePos	// pointer to position of filename within buffer
				   );
		Trace("name from SearchPath [" << buffer << "]");
		fullPathName = buffer;
		delete[] buffer;
		ResolvePath(fullPathName);
		Trace("Normalized pathname [" << fullPathName << "]");
		bRet = true;
	} else {
		// some error occured, set a warning
		SYSWARNING(String("tried to find [") << file << "], error is:" << SysLog::LastSysError());
	}
#else
	// HUM: found this somewhere but don't know how to make functional on Solaris/Linux
//	char* pFullPath = NULL;
//	String envPATH;
//	if (path == NULL)
//	{ // no search path given, default is to use PATH variable
//		envPATH = EnvGet("PATH");
//		Trace("using sysPath [" << envPATH << "]");
//		path = (const char*)envPATH;
//	}
//	String strPathBuffer(256L);
//	pFullPath = pathfind_r(
//		path,		// search path
//        file,		// filename to lookup
//        "rx",		// mode flags, we need read and execute
//		(char *)(const char*)strPathBuffer,	// pointer to character buffer
//		256L		// buffersize
//	);
//	if (pFullPath != NULL)
//	{ // found the file
//		Trace(String("Normalized pathname [") << strPathBuffer << "]");
//		fullPathName = strPathBuffer;
//		bRet = true;
//	}
//	else
//	{ // some error occured, set a warning
//		SYSWARNING(String("tried to find [") << file << "], error is:" << SysLog::LastSysError());
//	}
#endif
	return bRet;
}

bool System::IsRegularFile(const char *path)
{
	struct stat stbuf;
	if ( CheckPath(path, &stbuf) ) {
#if defined(WIN32)
		// our version of MSDEV compiler (V 5) at least not fully
		// posix compatible.
		return ((stbuf.st_mode & _S_IFMT) == _S_IFREG);
#else
		return S_ISREG(stbuf.st_mode);
#endif
	}
	return false;
}

bool System::IsDirectory(const char *path)
{
	struct stat stbuf;
	if ( CheckPath(path, &stbuf) ) {
#if defined(WIN32)
		// our version of MSDEV compiler (V 5) at least not fully
		// posix compatible.
		return ((stbuf.st_mode & _S_IFMT) == _S_IFDIR );
#else
		return S_ISDIR(stbuf.st_mode);
#endif
	}
	return false;
}

bool System::GetFileSize(const char *path, ul_long &ulFileSize)
{
	StartTrace1(System.GetFileSize, "path to file [" << NotNull(path) << "]");
	struct stat stbuf;
	if ( CheckPath(path, &stbuf) ) {
		ulFileSize = stbuf.st_size;
		Trace("file size: " << (long)ulFileSize << " bytes");
		return true;
	}
	return false;
}

iostream *System::IntOpenStreamBySearch(String &resultPath, const char *name, const char *path, openmode mode, bool trace)
{
	StartTrace(System.IntOpenStreamBySearch);
	// this methods open a stream searching over a search path
	iostream *fp = 0;
	StringTokenizer st(path, ':');
	String dirpath, filepath;
	// search over pathlist
	while (st.NextToken(dirpath)) {
		filepath.Append(fgRootDir).Append(Sep()).Append(dirpath).Append(Sep()).Append(name);
		System::ResolvePath(filepath);

		if ( (fp = DoOpenStream(resultPath, filepath, mode, trace)) != NULL ) {
			return fp;
		}
		// reset filepath and try next path
		filepath = 0;
	}
	return fp;
}

const char *System::GetRootDir()
{
	if ( fgRootDir.Length() == 0 ) {
		InitPath();
	}

	return (const char *) fgRootDir;
}

const char *System::GetPathList()
{
	if ( fgPathList.Length() == 0 ) {
		InitPath();
	}

	return (const char *) fgPathList;
}

Anything System::DirFileList(const char *dir, const char *extension)
{
	StartTrace(System.DirFileList);
	Anything list;
//  PS: not needed because we have a default parameter....
//	if (!extension)
//		extension= "html";
//PS: I require more than that....
	String strExtension;
	if (extension && strlen(extension) >= 1) { // allow one to specify "" for getting all files
		strExtension.Append('.').Append(extension);
	}
	long truncext = strExtension.Length();
#if defined(WIN32)
	// messy directory interface
	WIN32_FIND_DATA FileData;
	HANDLE hSearch;
	String searchPath(dir);
	BOOL fFinished = false;

	searchPath << System::Sep() << "*" << strExtension;
	// Start searching for (.html) files in the directory.
	hSearch = FindFirstFile((const char *)searchPath, &FileData);
	if (hSearch == INVALID_HANDLE_VALUE) {
		return list;
	}
	// Copy each .TXT file to the new directory
	// and change it to read only, if not already.
	while (!fFinished) {
		const char *fName = FileData.cFileName;
		list.Append(Anything(fName, strlen(fName) - truncext)); // PS: warum die extension abschneiden?

		if (!FindNextFile(hSearch, &FileData))
			if (GetLastError() == ERROR_NO_MORE_FILES) {
				fFinished = true;
			} else {
				SYSWARNING("couldn't find file");
			}
	}
	// Close the search handle.
	FindClose(hSearch);
#else
	// searches directory dir and
	DIR *fp;

	if ( (fp = opendir(dir)) ) {
		// do not use Storage module to allocate memory here, since readdir_r wreaks havoc with our storage management
		struct dirent *direntp = (dirent *)calloc(1, sizeof(dirent) + _POSIX_PATH_MAX);
		struct dirent *direntpSave = direntp;
		int ret;
#ifndef __370__
		while ( ((ret = readdir_r( fp, direntp, &direntp )) == 0) && (direntp) )
#else
	while ( (direntp = readdir(fp)) )
#endif
		{
			String name = direntp->d_name;
			Trace("current entry [" << name << "]");
			long start = name.Length() - truncext;
			if ( (start > 0) && strcmp(name, ".") != 0 && strcmp(name, "..") != 0 && (name.CompareN(strExtension, truncext, start) == 0) ) {
				// append filename without extension
				Trace("appending entry [" << name.SubString(0, start) << "]");
				list.Append(Anything(name, start));
			}
		}
		if ( direntp ) {
			Trace("last entry [" << NotNull(direntp->d_name) << "]");
		}
		::free(direntpSave);
		closedir(fp);
	} else {
		String logMsg("can't open ");
		logMsg.Append(dir);
		SYSERROR(logMsg);
	}
#endif
	return list;
}

bool System::LoadConfigFile(Anything &config, const char *name, const char *ext, String &realfilename)
{
	StartTrace(System.LoadConfigFile);
	istream *is = IntOpenStream(realfilename, name, ext, true, (ios::in), false);
	bool result = false;
	if (!is || !(result = config.Import(*is, realfilename))) {
		String logMsg("cannot import config file ");
		logMsg.Append(realfilename).Append(" from ").Append(name).Append('.').Append(ext);
		SYSERROR(logMsg);
	}
	delete is;
	return result;
}

bool System::LoadConfigFile(Anything &config, const char *name, const char *ext)
{
	String dummy;
	return LoadConfigFile(config, name, ext, dummy);
}

#if defined(WIN32)
// the following code was taken from ACE
bool System::Uname(Anything &anyInfo)
{
	StartTrace(System.Uname);
	bool bRet = false;
	anyInfo["sysname"] = "Win32";

	OSVERSIONINFO vinfo;
	vinfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
	::GetVersionEx (&vinfo);

	SYSTEM_INFO sinfo;
	::GetSystemInfo(&sinfo);

	String processor, subtype;
	if ( vinfo.dwPlatformId == VER_PLATFORM_WIN32_NT ) {
		// Get information from the two structures
		String os("Windows "), build("Build ");
		if (vinfo.dwPlatformId == VER_PLATFORM_WIN32_NT) {
			os << "NT";
		} else {
			os << "CE";
		}
		anyInfo["release"] = os << (long)vinfo.dwMajorVersion << "." << (long)vinfo.dwMinorVersion;
		anyInfo["version"] = build << (long)vinfo.dwBuildNumber << " " << vinfo.szCSDVersion;

		WORD arch = sinfo.wProcessorArchitecture;
		switch (arch) {
			case PROCESSOR_ARCHITECTURE_INTEL:
				processor = "Intel";
				if (sinfo.wProcessorLevel == 3) {
					subtype = "80386";
				} else if (sinfo.wProcessorLevel == 4) {
					subtype = "80486";
				} else if (sinfo.wProcessorLevel == 5) {
					subtype = "Pentium";
				} else if (sinfo.wProcessorLevel == 6) {
					subtype = "Pentium Pro";
				} else if (sinfo.wProcessorLevel == 7) {
					subtype = "Pentium II";
				}
				break;
			case PROCESSOR_ARCHITECTURE_MIPS:
				processor = "MIPS";
				subtype = "R4000";
				break;
			case PROCESSOR_ARCHITECTURE_ALPHA:
				processor = "Alpha";
				subtype = sinfo.wProcessorLevel;
				break;
			case PROCESSOR_ARCHITECTURE_PPC:
				processor = "PPC";
				if (sinfo.wProcessorLevel == 1) {
					subtype = "601";
				} else if (sinfo.wProcessorLevel == 3) {
					subtype = "603";
				} else if (sinfo.wProcessorLevel == 4) {
					subtype = "604";
				} else if (sinfo.wProcessorLevel == 6) {
					subtype = "603+";
				} else if (sinfo.wProcessorLevel == 9) {
					subtype = "804+";
				} else if (sinfo.wProcessorLevel == 20) {
					subtype = "620";
				}
				break;
#if defined(PROCESSOR_ARCHITECTURE_IA64)
			case PROCESSOR_ARCHITECTURE_IA64:
				processor = "Itanium";
				subtype = sinfo.wProcessorLevel;
				break;
#endif
			case PROCESSOR_ARCHITECTURE_UNKNOWN:
			default:
				break;
		}
	} else if (vinfo.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS) {
		String release, reladd;
		if (vinfo.dwMajorVersion == 4 && vinfo.dwMinorVersion == 0) {
			release = "Windows 95";
			if (vinfo.szCSDVersion[1] == 'C') {
				reladd = " OSR2";
			}
		} else if (vinfo.dwMajorVersion == 4 && vinfo.dwMinorVersion == 10) {
			release = "Windows 98";
			if (vinfo.szCSDVersion[1] == 'A') {
				reladd = " SE";
			}
		} else if (vinfo.dwMajorVersion == 4 && vinfo.dwMinorVersion == 90) {
			release = "Windows Me";
		}
		if ( release.Length() ) {
			anyInfo["release"] = release << reladd;
		}
		anyInfo["version"] = (long)LOWORD(vinfo.dwBuildNumber);

		if (sinfo.dwProcessorType == PROCESSOR_INTEL_386) {
			processor = "Intel 80386";
		} else if (sinfo.dwProcessorType == PROCESSOR_INTEL_486) {
			processor = "Intel 80486";
		} else if (sinfo.dwProcessorType == PROCESSOR_INTEL_PENTIUM) {
			processor = "Intel Pentium";
		}
	}
	if ( processor.Length() ) {
		anyInfo["machine"] = processor << (subtype.Length() ? " " : "") << subtype;
	}

	String nodename;
	bRet = ( System::HostName(nodename) && bRet );
	if ( bRet ) {
		anyInfo["nodename"] = nodename;
	}
	return bRet;
}
#else
bool System::Uname(Anything &anyInfo)
{
	StartTrace(System.Uname);
	struct utsname name;
	int iRet = ::uname(&name);
	if ( iRet < 0 ) {
		String strErr("uname returned:");
		strErr << (long)iRet;
		SYSERROR(strErr);
	} else {
		anyInfo["sysname"] = name.sysname;
		anyInfo["nodename"] = name.nodename;
		anyInfo["release"] = name.release;
		anyInfo["version"] = name.version;
		anyInfo["machine"] = name.machine;
	}
	return ( iRet >= 0);
}
#endif /* WIN32 */

bool System::HostName(String &name)
{
	StartTrace(System.HostName);
	bool bRet = false;
#if defined(WIN32)
	const DWORD dSz = 256;
	DWORD lBufSz = dSz;
	char strBuf[dSz];
	bRet = ( ::GetComputerNameA(strBuf, &lBufSz) != 0);
	if ( bRet ) {
		name = strBuf;
	}
#else
	Anything host_info;
	if ( ( bRet = System::Uname(host_info) ) ) {
		name = host_info["nodename"].AsString("unknown");
	}
#endif
	return bRet;
}

pid_t System::getpid()
{
#if defined(WIN32)
	return GetCurrentProcessId();
#else
	return ::getpid();
#endif
}

uid_t System::getuid()
{
#if defined(WIN32)
	return (uid_t) - 1L;
#else
	return ::getuid();
#endif
}

int System::IO::access(const char *path, int amode)
{
	StartTrace(System.IO.access);
	return ::access(path, amode);
}

int System::IO::mkdir(const char *path, int pmode)
{
	StartTrace1(System.IO.mkdir, "path [" << NotNull(path) << "]");
#if defined(WIN32)
	// no access mode possible for win32
	return ::mkdir(path);
#else
	return ::mkdir(path, pmode);
#endif
}

int System::IO::rmdir(const char *path)
{
	StartTrace1(System.IO.rmdir, "path [" << NotNull(path) << "]");
	return ::rmdir(path);
}

int System::IO::unlink(const char *path)
{
	StartTrace1(System.IO.unlink, "path [" << NotNull(path) << "]");
	return ::unlink(path);
}

int System::IO::rename(const char *oldfilename, const char *newfilename)
{
	StartTrace1(System.IO.rename, "oldname [" << NotNull(oldfilename) << "] newname [" << NotNull(newfilename) << "]");
	return ::rename(oldfilename, newfilename);
}

bool System::BlocksLeftOnFS(const char *pFsPath, ul_long &ulBlocks, unsigned long &lBlkSize)
{
	StartTrace1(System.BlocksLeftOnFS, "fs path [" << NotNull(pFsPath) << "]");
#if defined(__sun)
	struct statvfs buf;
	if (0 == statvfs(pFsPath, &buf)) {
		lBlkSize = (unsigned long)buf.f_frsize;
		ulBlocks = (ul_long)buf.f_bavail;
#elif defined(__linux__)
	struct statfs buf;
	if (0 == statfs(pFsPath, &buf)) {
		lBlkSize = (unsigned long)buf.f_bsize;
		ulBlocks = (ul_long)buf.f_bavail;
#elif defined(WIN32)
	_ULARGE_INTEGER ulBytesAvailable;
	if ( GetDiskFreeSpaceEx(pFsPath, &ulBytesAvailable, NULL, NULL) != 0 ) {
		lBlkSize = 1;
		ulBlocks = ulBytesAvailable.QuadPart;
#endif
		Trace("blocksize: " << (long)lBlkSize << " bytes free blocks: " << (long)ulBlocks);
		return true;
	} else {
		SYSWARNING("Failed to get blocks left on FS [" << SysLog::LastSysError() << "]");
	}
	return false;
}

bool System::MakeDirectory(const char * path, int pmode, bool bRecurse) {
	StartTrace1(System.MakeDirectory, "dir to create [" << NotNull(path) << "], recurse " << (bRecurse ? "true" : "false"));
	String dir(path);
	// check for empty input parameter
	if ( !dir.Length() ) {
		SYSWARNING("empty path given, bailing out!");
		return false;
	}
	// check if directory already exists
	if ( IsDirectory(dir) ) {
		SYSERROR("directory already exists [" << dir << "]");
		return false;
	}
	// check whether path-length is ok
	if ( dir.Length() > System::cPATH_MAX ) {
		SYSERROR("Path size exceeded [" << dir << "]");
		return false;
	}
	// strip trailing slash if any
	long slPos = dir.StrRChr( System::Sep() );
	if ( slPos >= 0L && ( slPos == (dir.Length() - 1L) ) ) {
		dir.Trim(dir.Length() - 1L);
	}
	return IntMakeDirectory(dir, pmode, bRecurse);
}

bool System::IntMakeDirectory(String dir, int pmode, bool bRecurse) {
	StartTrace1(System.IntMakeDirectory, "dir to create [" << dir << "], recurse " << (bRecurse ? "true" : "false"));
	String strParentDir;
	long slPos = dir.StrRChr( System::Sep() );
	if ( slPos > 0L ) {
		// remove last segment and slash
		strParentDir = dir.SubString(0L, slPos);
	}
	Trace("parent path [" << strParentDir << "]");

	bool bGoAhead = true;
	if ( strParentDir.Length() && !IsDirectory(strParentDir) ) {
		if ( bRecurse ) {
			// recurse to make parent directories
			bGoAhead = IntMakeDirectory(strParentDir, pmode, bRecurse);
		} else {
			SYSERROR("parent directory does not exist [" << strParentDir << "]");
			bGoAhead = false;
		}
	}
	if ( bGoAhead ) {
		// make new directory
		bGoAhead = ( System::IO::mkdir(dir, pmode) == 0L );
		if ( !bGoAhead ) {
			// if errno is set to EEXIST, someone else might already have created the dir, so do not complain
			if ( System::GetSystemError() == EEXIST ) {
				bGoAhead = true;
				SYSINFO("mkdir of [" << dir << "] was unsuccessful [" << SysLog::LastSysError() << "] because the directory was created by someone else in the meantime?!");
			} else {
				SYSERROR("mkdir of [" << dir << "] failed with [" << SysLog::LastSysError() << "]");
			}
		}
	}
	return bGoAhead;
}

bool System::RemoveDirectory(const char * path, bool bRecurse) {
	StartTrace1(System.RemoveDirectory, "dir to remove [" << NotNull(path) << "]");
	String dir(path);
	// check for empty input parameter
	if ( !dir.Length() ) {
		SYSWARNING("empty path given, bailing out!");
		return false;
	}
	// check if directory already exists
	if ( !IsDirectory(dir) ) {
		SYSERROR("directory does not exist [" << dir << "]");
		return false;
	}
	// do not allow recursive deletion of absolute paths
	bool bAbsDir = IsAbsolutePath(dir);
	if ( bRecurse && bAbsDir ) {
		SYSERROR("recursive deletion of absolute path not allowed [" << dir << "]");
		return false;
	}
	return IntRemoveDirectory(dir, bRecurse, bAbsDir);
}

bool System::IntRemoveDirectory(String dir, bool bRecurse, bool bAbsDir) {
	StartTrace1(System.IntRemoveDirectory, "dir to remove [" << dir << "], recurse " << (bRecurse ? "true" : "false"));

	bool bGoAhead = ( System::IO::rmdir(dir) == 0L );
	if ( bGoAhead ) {
		if ( bRecurse && !bAbsDir ) {
			long slPos = dir.StrRChr( System::Sep() );
			if ( slPos > 0L ) {
				// remove parent dir
				bGoAhead = IntRemoveDirectory(dir.SubString(0L, slPos), bRecurse, bAbsDir);
			}
		}
	}
	return bGoAhead;
}

long System::Fork() {
#if defined(WIN32)
	return -1;
#elif defined(__sun)
	return fork1();
#else
	return fork();
#endif
}

bool System::GetLockFileState(const char * lockFileName) {
	StartTrace(System.CreateLockFile);
	// O_EXCL must be set to guarantee atomic lock file creation.
	int fd = open(lockFileName, O_WRONLY | O_CREAT | O_EXCL);
	if (fd < 0 && errno == EEXIST) {
		// the file already exist; another process is
		// holding the lock
		return true;
	}
	if (fd < 0) {
		SYSERROR("Getting lock state failed. Reason: [" << SysLog::LastSysError() << "]");
		// What to do in case of an error? Let's be conservative and consider the lockfile as locked.
		return true;
	}
	// if we got here, we own the lock or had an error
	return false;
}

//int System::GetNumberOfHardLinks(const char *path)
//{
//	StartTrace(System.GetNumberOfHardLinks);
//	struct stat		mystat;
//
//	// acquire inode information
//	if (stat(path, &mystat) == -1)
//	{
//		SYSERROR("Could not acquire inode information for " << path << "; stat reports [" << SysLog::LastSysError() << "]");
//		return -1;
//	}
//	// return number of hard links to <path>
//	return mystat.st_nlink;
//}
//
//bool System::CreateSymbolicLink(const char *filename, const char* symlinkname)
//{
//	StartTrace(System.CreateSymbolicLink);
//	if (symlink(filename, symlinkname) == -1)
//	{
//		SYSERROR("Could not create symbolic link " << symlinkname << " to file " << filename << "; symlink reports [" << SysLog::LastSysError() << "]");
//		return false;
//	}
//	// success
//	return true;
//}
