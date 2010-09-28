/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

//--- interface include --------------------------------------------------------
#include "SystemFile.h"
#include "SystemBase.h"
#include "MmapStream.h"

//--- standard modules used ----------------------------------------------------
#include "SystemLog.h"
#include "Dbg.h"

//--- c-library modules used ---------------------------------------------------
#include <climits>
#include <errno.h>
#include <cstring>

#if defined(ONLY_STD_IOSTREAM)
#include <fstream>
using namespace std;
#else
#include <fstream.h>
#endif

#if defined(WIN32)
#include <io.h>
#include <direct.h>
#else
#include <dirent.h>  // directory access
#include <sys/statvfs.h>
#endif
#if defined(__SUNPRO_CC)
#include <sys/types.h>
#endif

namespace {
	//!contains the systems path seperator
	const char cSep = '/';

	//!defines maximal size of path
	#if defined(WIN32) && !defined(_POSIX_)
	#define PATH_MAX	512
	#endif
	#if defined(_POSIX_)
	const long cPATH_MAX = _POSIX_PATH_MAX;
	#else
	const long cPATH_MAX = PATH_MAX;
	#endif

	//!contains the root directory path that is used to locate files, it can be relative or absolute
	String fgRootDir(0L, Storage::Global());

	//!contains a search path list that is ':' delimited, it is used to search for files
	String fgPathList(0L, Storage::Global());

	//! checks existence of a path using stat
	/*! \param path file or directory path
		\param stbuf stat buffer to fill
		\param bIsSymbolicLink set to true if the given path is a link pointing either to a file or directory. The st_mode member of the stat param will be set to the real type the link points to.
		\return true in case the call to stat was successful */
	bool CheckPath(const char *path, struct stat *stbuf, bool &bIsSymbolicLink)
	{
		StartTrace(System.CheckPath);
		bool result = false;
		while ( !(result = (lstat(path, stbuf) == 0)) && Coast::System::SyscallWasInterrupted() ) {
			String msg("OOPS, lstat failed with ");
			msg << SystemLog::LastSysError() << " on " << path;
			Trace(msg);
			SYSWARNING(msg);
		}
		bIsSymbolicLink = ( result ? S_ISLNK(stbuf->st_mode) : false );
		if ( result ) {
			Trace("mode field value of lstat: " << (long)stbuf->st_mode);
		}
		while ( !(result = (stat(path, stbuf) == 0)) && Coast::System::SyscallWasInterrupted() ) {
			String msg("OOPS, stat failed with ");
			msg << SystemLog::LastSysError() << " on " << path;
			Trace(msg);
			SYSWARNING(msg);
		}
		if ( result ) {
			Trace("mode field value of stat: " << (long)stbuf->st_mode);
		}
		return result;
	}

	//! bottleneck routine that opens the stream if possible it returns null if not successful
	/*! \param resultPath the location of the iostream opened
		\param name the filename, it can be relative or absolute, it contains the extension
		\param mode the mode of the stream to be opened e.g. ios::in, mode flags can be combined by the | operation
		\param trace if true writes messages to SystemLog
		\return an open iostream or NULL if the open fails */
	iostream *DoOpenStream(String &resultPath, const char *name, openmode mode, bool trace = false)
	{
		StartTrace1(System.DoOpenStream, "file [" << NotNull(name) << "]");
		// adjust mode to output, append implies it
		if ( mode & ios::app ) {
			mode |= ios::out;
		}

		if ( Coast::System::IsRegularFile(name) || (mode & ios::out) ) {
			static bool bUseMmapStreams = ( Coast::System::EnvGet("COAST_USE_MMAP_STREAMS").AsLong(1L) == 1L );
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

	//! internal method that opens a stream if possible, it searches the pathlist for the correct location
	/*! \param resultPath the location of the iostream opened
		\param pathlist the pathlist to be searched,relative to the root directory
		\param name the filename, it can be relative or absolute, it contains the extension
		\param mode the mode of the stream to be opened e.g. ios::in, mode flags can be combined by the | operation
		\param trace if true writes messages to SystemLog
		\return an open iostream or NULL if the open fails */
	iostream *IntOpenStreamBySearch(String &resultPath, const char *name, const char *path, openmode mode, bool trace = false)
	{
		StartTrace(System.IntOpenStreamBySearch);
		// this methods open a stream searching over a search path
		iostream *fp = 0;
		StringTokenizer st(path, ':');
		String dirpath, filepath;
		// search over pathlist
		while (st.NextToken(dirpath)) {
			filepath.Append(fgRootDir).Append(Coast::System::Sep()).Append(dirpath).Append(Coast::System::Sep()).Append(name);
			Coast::System::ResolvePath(filepath);

			if ( (fp = DoOpenStream(resultPath, filepath, mode, trace)) != NULL ) {
				return fp;
			}
			// reset filepath and try next path
			filepath = 0;
		}
		return fp;
	}

	//! internal method that opens a stream if possible
	/*! \param resultPath the location of the iostream opened
		\param search flag if true searches pathlist until a stream can be opened
		\param name the filename, it can be relative or absolute
		\param extension the extension of the file
		\param mode the mode of the stream to be opened e.g. ios::in, mode flags can be combined by the | operation
		\param trace if true writes messages to SystemLog
		\return an open iostream or NULL if the open fails */
	iostream *IntOpenStream(String &resultPath, const char *name, const char *extension, bool searchPath, openmode mode, bool trace = false)
	{
		StartTrace(System.IntOpenStream);
		// fast exit
		if ( name == 0 ) {
			return 0 ;
		}

		// init the rootdir and pathlist environment
		// if necessary
		if ( fgPathList.Length() == 0 ) {
			Coast::System::InitPath();
		}

		// we should use stat to look for the file prior to use

		String filename(name);
		if (extension && strlen(extension) > 0) {
			filename.Append('.').Append(extension);
		}

		iostream *Ios = 0;
		if (Coast::System::IsAbsolutePath(filename) || !searchPath) {
			if (Coast::System::IsAbsolutePath(filename) && trace) {
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
			SYSINFO(String("couldn't open file [") << filename << "] searched in [" << fgRootDir << "] with pathlist: [" << fgPathList << "]");
		}
		Coast::System::ResolvePath(resultPath);
		return Ios;
	}

	//! internal method to extend a directory with given permissions by creating 'extension' directories and softlinks
	/*!	\param path relative or absolute path to create new directory
		\param pmode permission of new directory, octal number
		\return System::eSuccess if new directory was created */
	Coast::System::DirStatusCode IntExtendDir(String &strOriginalDir, int pmode)
	{
		StartTrace1(System.IntExtendDir, "dir to create [" << strOriginalDir << "]");
		Coast::System::DirStatusCode aDirStatus(Coast::System::ePathIncomplete);
		String strBasePath(strOriginalDir), strDirToExtend, strFinalDir, strExtensionDir;
		long slPos = strBasePath.StrRChr( Coast::System::Sep() );
		if ( slPos > 0L ) {
			// find final segment
			strFinalDir = strBasePath.SubString(slPos + 1);
			strBasePath.Trim(slPos);
			// find path to extend by other dir and link
			slPos = strBasePath.StrRChr( Coast::System::Sep() );
			if ( slPos > 0L ) {
				// find final segment
				strDirToExtend = strBasePath.SubString(slPos + 1);
				strBasePath.Trim(slPos);
			}
		}
		bool bContinue = ( strFinalDir.Length() > 0L && strDirToExtend.Length() > 0L );
		long lExt = 0L;
		while ( bContinue ) {
			strExtensionDir = strBasePath;
			if ( strExtensionDir.Length() > 0L ) {
				strExtensionDir.Append(Coast::System::Sep());
			}
			strExtensionDir.Append(strDirToExtend).Append("_ex").Append(lExt).Append(Coast::System::Sep()).Append(strFinalDir);
			Trace("trying extension directory [" << strExtensionDir << "]");
			switch ( aDirStatus = Coast::System::MakeDirectory(strExtensionDir, pmode, true, false) ) {
				case Coast::System::eNoMoreHardlinks: {
					// no more room for creating a 'real' directory
					// lets try the next extension number
					break;
				}
				case Coast::System::eExists:
					// need to catch this, directory might exist already or someone else created it recently
				case Coast::System::eSuccess: {
					// create link into original directory (strOriginalDir)
					aDirStatus = Coast::System::CreateSymbolicLink(strExtensionDir, strOriginalDir);
				}
				default: {
					bContinue = false;
				}
			}
			++lExt;
		}
		return aDirStatus;
	}

	//! internal method to create new directory with given permissions, works for relative or absolute path names and also recursive if specified.
	/*! \note Unless bRecurse is true, the parent directory must already exist and only one directory level will be created
		\param path relative or absolute path to create new directory
		\param pmode permission of new directory, octal number
		\param bRecurse set to true if nonexisting parent directories should be created
		\param bExtendByLinks if a directory can not be created because its parent dir is exhausted of hard links (subdirectories), a true means to create an 'extension' parent directory of name <dir>_ex[0-9]+ and link the newly created directory into the original location
		\return System::eSuccess if new directory was created */
	Coast::System::DirStatusCode IntMakeDirectory(String &path, int pmode, bool bRecurse, bool bExtendByLinks)
	{
		StartTrace1(System.IntMakeDirectory, "dir to create [" << path << "], recurse " << (bRecurse ? "true" : "false"));
		String strParentDir;
		long slPos = path.StrRChr( Coast::System::Sep() );
		if ( slPos > 0L ) {
			// remove last segment and slash
			strParentDir = path.SubString(0L, slPos);
		}
		Trace("parent path [" << strParentDir << "]");

		Coast::System::DirStatusCode aDirStatus = Coast::System::ePathIncomplete;
		if ( strParentDir.Length() ) {
			if ( Coast::System::IsDirectory(strParentDir) ) {
				aDirStatus = Coast::System::eSuccess;
			} else {
				if ( bRecurse ) {
					// recurse to make parent directories
					aDirStatus = IntMakeDirectory(strParentDir, pmode, bRecurse, bExtendByLinks);
				} else {
					SYSERROR("parent directory does not exist [" << strParentDir << "]");
					aDirStatus = Coast::System::eNotExists;
				}
			}
		} else {
			// assume root level or relative
			aDirStatus = Coast::System::eSuccess;
		}
		// eExists might occur if someone created the parent directory in parallel
		if ( aDirStatus == Coast::System::eSuccess || aDirStatus == Coast::System::eExists ) {
			// make new directory
			if ( Coast::System::IO::mkdir(path, pmode) != 0L ) {
				switch ( Coast::System::GetSystemError() ) {
						// if errno is set to EEXIST, someone else might already have created the dir, so do not complain
					case EEXIST: {
						SYSINFO("mkdir of [" << path << "] was unsuccessful [" << SystemLog::LastSysError() << "] because the directory was created by someone else in the meantime?!");
						aDirStatus = Coast::System::eExists;
						break;
					}
					case EDQUOT: {
						SYSERROR("mkdir of [" << path << "] was unsuccessful [" << SystemLog::LastSysError() << "]");
						aDirStatus = Coast::System::eQuotaExceeded;
						break;
					}
					case ENOSPC: {
						SYSERROR("mkdir of [" << path << "] was unsuccessful [" << SystemLog::LastSysError() << "] -> check for free inodes using $>df -F ufs -o i <FS>");
						aDirStatus = Coast::System::eNoSpaceLeft;
						break;
					}
					case EACCES: {
						SYSWARNING("mkdir of [" << path << "] was unsuccessful [" << SystemLog::LastSysError() << "]");
						aDirStatus = Coast::System::eNoPermission;
						break;
					}
					case ENOENT: {
						SYSWARNING("mkdir of [" << path << "] was unsuccessful [" << SystemLog::LastSysError() << "]");
						aDirStatus = Coast::System::eNotExists;
						break;
					}
					case ENOTDIR: {
						SYSWARNING("mkdir of [" << path << "] was unsuccessful [" << SystemLog::LastSysError() << "]");
						aDirStatus = Coast::System::eNotDirectory;
						break;
					}
					case EIO: {
						SYSERROR("mkdir of [" << path << "] was unsuccessful [" << SystemLog::LastSysError() << "]");
						aDirStatus = Coast::System::eIOOperationFailed;
						break;
					}
					case EFAULT: {
						SYSERROR("mkdir of [" << path << "] was unsuccessful [" << SystemLog::LastSysError() << "]");
						aDirStatus = Coast::System::ePathIllegal;
						break;
					}
					case EMLINK: {
						SYSWARNING("mkdir of [" << path << "] was unsuccessful [" << SystemLog::LastSysError() << "] because the parent directory is exhausted of hardlinks!");
						aDirStatus = Coast::System::eNoMoreHardlinks;
						if ( bExtendByLinks ) {
							// if no more directories can be created, we use 'extension' links instead
							aDirStatus = IntExtendDir(path, pmode);
						}
						break;
					}
					default: {
						SYSERROR("mkdir of [" << path << "] failed with [" << SystemLog::LastSysError() << "]");
						aDirStatus = Coast::System::eCreateDirFailed;
					}
				}
			}
		} else {
			// assign parent directory to parameter to be able to localize the problem
			path = strParentDir;
		}
		return aDirStatus;
	}

	//! internal method to remove given directory - relative directories can be deleted recursively
	/*! \param path relative or absolute path of directory to be deleted
		\param bRecurse true if relative directory should be deleted recusrively
		\param bAbsDir if true we can not recurse to delete directories
		\return System::eSuccess if directory was removed */
	Coast::System::DirStatusCode IntRemoveDirectory(String &path, bool bRecurse, bool bAbsDir)
	{
		StartTrace1(System.IntRemoveDirectory, "dir to remove [" << path << "], recurse " << (bRecurse ? "true" : "false"));

		Coast::System::DirStatusCode aDirStatus = Coast::System::eNoSuchFileOrDir;
		if ( ( Coast::System::IsDirectory(path) && Coast::System::IO::rmdir(path) == 0L ) || ( Coast::System::IsSymbolicLink(path) && Coast::System::IO::unlink(path) == 0L ) ) {
			aDirStatus = Coast::System::eSuccess;
			if ( bRecurse && !bAbsDir ) {
				long slPos = path.StrRChr( Coast::System::Sep() );
				if ( slPos > 0L ) {
					// remove parent dir
					String strParent(path.SubString(0L, slPos));
					aDirStatus = IntRemoveDirectory(strParent, bRecurse, bAbsDir);
					if ( aDirStatus != Coast::System::eSuccess ) {
						path = strParent;
					}
				}
			}
		} else {
			SYSERROR("rmdir of [" << path << "] was unsuccessful [" << SystemLog::LastSysError() << "]");
			switch ( Coast::System::GetSystemError() ) {
				case EEXIST:
				case ENOTEMPTY:
					aDirStatus = Coast::System::eExists;
					break;
				case ENOENT:
					aDirStatus = Coast::System::eNotExists;
					break;
				default:
					;
			}
		}
		return aDirStatus;
	}

}


namespace Coast {
	namespace System {

		char Sep() {
			return cSep;
		}

		// used to smoothify the given path;
		// remove relative movements
		// on WIN32, convert to correct drive notation and use only one kind of slashes
		void ResolvePath(String &path)
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

		void StripDriveLetter(String &path)
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

		bool GetDriveLetter(const char *name, char &drive)
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

		bool IsAbsolutePath(const char *name)
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
				if ( (*name) == Sep() ) {
					// it is an absolute path name
					return true;
				}
		#endif
			}
			return false;
		}

		void GetCWD(String &cwd)
		{
			String buf(4096L);
			cwd = ".";
			if (!getcwd((char *)(const char *)buf, buf.Capacity())) {
				SystemLog::Alert("puh, cannot obtain current working directory" );
				return;
			} else {
				cwd = String((void *)(const char *)buf, strlen((const char *)buf));
				ResolvePath(cwd);
			}
		}

		bool ChangeDir(const String &dir)
		{
			if (dir.Length() > 0 && dir != ".") {
				if (!IsDirectory(dir) || chdir(dir) != 0) {
					return false;
				}
			}
			return true;
		}

		int Chmod(const char *filename, int pmode)
		{
		#if defined(WIN32)
			return _chmod(filename, pmode);
		#else
			return ::chmod(filename, pmode);
		#endif
		}

		//---- System --------------------------------------------------------------------------
		void InitPath(const char *root, const char *path)
		{
			static bool once = false;
			String tmpRoot((root) ? String(root) : EnvGet("COAST_ROOT"));
			String tmpPath((path) ? String(path) : EnvGet("COAST_PATH"));

			if (!once || tmpRoot.Length() > 0 || tmpPath.Length() > 0) {
				once = true;
				if (tmpRoot.Length() > 0) {
					SetRootDir(tmpRoot, true);
				} else {
					SetRootDir(".", true);
				}
				if (tmpPath.Length() > 0) {
					SetPathList(tmpPath, true);
				} else {
					SetPathList(".:config:src:", true);
				}
			}
		}

		void SetRootDir(const char *root, bool print)
		{
			if (root) {
				String rPath(root);
				// when a dot is supplied as root, ResolvePath would remove it and leave the path blank
				if (rPath != ".") {
					ResolvePath(rPath);
				}
				if (print) {
					SystemLog::WriteToStderr( String("Root Dir: <") << rPath << ">" << "\n" );
				}

				String logMsg("Root Dir: <");
				logMsg << rPath << ">";
				SystemLog::Info(logMsg);

				fgRootDir.SetAllocator(Storage::Global());
				fgRootDir = rPath;
			}
		}

		void SetPathList(const char *pathlist, bool print)
		{
			if (pathlist) {
				if (print) {
					SystemLog::WriteToStderr( String("Pathlist: <") << pathlist << ">" << "\n" );
				}

				String logMsg("Pathlist: <");
				logMsg << pathlist << ">";
				SystemLog::Info(logMsg);

				fgPathList.SetAllocator(Storage::Global());
				fgPathList = pathlist;
			}
		}

		iostream *OpenIStream(const char *name, const char *extension, openmode mode, bool trace)
		{
			String dummy;
			return IntOpenStream(dummy, name, extension, false, mode | ios::in, trace);
		}

		iostream *OpenOStream(const char *name, const char *extension, openmode mode, bool trace)
		{
			String dummy;
			return IntOpenStream(dummy, name, extension, false, mode | ios::out, trace);
		}

		iostream *OpenStream(const char *name, const char *extension, openmode mode, bool trace)
		{
			String dummy;
			return IntOpenStream(dummy, name, extension, true, mode, trace);
		}

		String GetFilePath(const char *name, const char *extension)
		{
			String path;
			iostream *Ios = IntOpenStream(path, name, extension, true, ios::in);
			if (Ios) {
				delete Ios;
			}

			return path;
		}

		bool FindFile(String &fullPathName, const char *file, const char *path)
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
				SYSWARNING(String("tried to find [") << file << "], error is:" << SystemLog::LastSysError());
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
		//		SYSWARNING(String("tried to find [") << file << "], error is:" << SystemLog::LastSysError());
		//	}
		#endif
			return bRet;
		}

		bool IsRegularFile(const char *path)
		{
			struct stat stbuf;
			bool bIsSymbolicLink;
			if ( CheckPath(path, &stbuf, bIsSymbolicLink) ) {
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

		bool IsDirectory(const char *path)
		{
			struct stat stbuf;
			bool bIsSymbolicLink;
			if ( CheckPath(path, &stbuf, bIsSymbolicLink) ) {
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

		bool IsSymbolicLink(const char *path)
		{
			struct stat stbuf;
			bool bIsSymbolicLink = false;
			if ( CheckPath(path, &stbuf, bIsSymbolicLink) ) {
				return bIsSymbolicLink;
			}
			return false;
		}

		bool GetFileSize(const char *path, ul_long &ulFileSize)
		{
			StartTrace1(System.GetFileSize, "path to file [" << NotNull(path) << "]");
			struct stat stbuf;
			bool bIsSymbolicLink;
			if ( CheckPath(path, &stbuf, bIsSymbolicLink) ) {
				ulFileSize = stbuf.st_size;
				Trace("file size: " << (long)ulFileSize << " bytes");
				return true;
			}
			return false;
		}

		const char *GetRootDir()
		{
			if ( fgRootDir.Length() == 0 ) {
				InitPath();
			}

			return (const char *) fgRootDir;
		}

		const char *GetPathList()
		{
			if ( fgPathList.Length() == 0 ) {
				InitPath();
			}

			return (const char *) fgPathList;
		}

		Anything DirFileList(const char *dir, const char *extension)
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

			searchPath << Sep() << "*" << strExtension;
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

		bool BlocksLeftOnFS(const char *pFsPath, ul_long &ulBlocks, unsigned long &lBlkSize)
		{
			StartTrace1(System.BlocksLeftOnFS, "fs path [" << NotNull(pFsPath) << "]");

		#if defined(WIN32)
			_ULARGE_INTEGER ulBytesAvailable;
			if ( GetDiskFreeSpaceEx(pFsPath, &ulBytesAvailable, NULL, NULL) != 0 ) {
				lBlkSize = 1;
				ulBlocks = ulBytesAvailable.QuadPart;
				Trace("blocksize: " << (long)lBlkSize << " bytes free blocks: " << (long)ulBlocks);
				return true;
			} else
		#else
			struct statvfs buf;
			if (0 == statvfs(pFsPath, &buf)) {
				lBlkSize = (unsigned long)buf.f_frsize;
				ulBlocks = (ul_long)buf.f_bavail;
				Trace("blocksize: " << (long)lBlkSize << " bytes free blocks: " << (long)ulBlocks);
				return true;
			} else
		#endif
			{
				SYSWARNING("Failed to get blocks left on FS [" << SystemLog::LastSysError() << "]");
			}
			return false;
		}

		DirStatusCode MakeDirectory(String &path, int pmode, bool bRecurse, bool bExtendByLinks)
		{
			StartTrace1(System.MakeDirectory, "dir to create [" << path << "], recurse " << (bRecurse ? "true" : "false") << ", extendbylinks " << (bExtendByLinks ? "true" : "false"));
			// cleanup relative movements and slashes
			ResolvePath(path);

			// check for empty input parameter
			if ( !path.Length() ) {
				SYSWARNING("empty path given, bailing out!");
				return ePathEmpty;
			}
			// check if directory already exists
			if ( IsDirectory(path) ) {
				SYSWARNING("directory already exists [" << path << "]");
				return eExists;
			}
			// check whether path-length is ok
			if ( path.Length() > cPATH_MAX ) {
				SYSERROR("Path size exceeded [" << path << "]");
				return ePathTooLong;
			}
			return IntMakeDirectory(path, pmode, bRecurse, bExtendByLinks);
		}

		DirStatusCode RemoveDirectory(String &path, bool bRecurse)
		{
			StartTrace1(System.RemoveDirectory, "dir to remove [" << path << "]");

			// check for empty input parameter
			if ( !path.Length() ) {
				SYSWARNING("empty path given, bailing out!");
				return ePathEmpty;
			}
			// check if directory already exists
			if ( !IsDirectory(path) && !IsSymbolicLink(path) ) {
				SYSERROR("directory does not exist [" << path << "]");
				return eNotExists;
			}
			// do not allow recursive deletion of absolute paths
			bool bAbsDir = IsAbsolutePath(path);
			if ( bRecurse && bAbsDir ) {
				SYSERROR("recursive deletion of absolute path not allowed [" << path << "]");
				return eRecurseDeleteNotAllowed;
			}
			return IntRemoveDirectory(path, bRecurse, bAbsDir);
		}

		int GetNumberOfHardLinks(const char *path)
		{
			StartTrace1(System.GetNumberOfHardLinks, "directory [" << NotNull(path) << "]");
			struct stat	mystat;

			// acquire inode information
			if (stat(path, &mystat) == -1) {
				SYSERROR("Could not acquire inode information for " << path << "; stat reports [" << SystemLog::LastSysError() << "]");
				return -1;
			}
			// return number of hard links to <path>
			return mystat.st_nlink;
		}

		DirStatusCode CreateSymbolicLink(const char *filename, const char *symlinkname)
		{
			StartTrace1(System.CreateSymbolicLink, "directory [" << NotNull(filename) << "] link [" << NotNull(symlinkname) << "]");
			if ( symlink(filename, symlinkname) == -1 ) {
				SYSERROR("Could not create symbolic link " << symlinkname << " to file " << filename << "; symlink reports [" << SystemLog::LastSysError() << "]");
				return eCreateSymlinkFailed;
			}
			// success
			return eSuccess;
		}

		bool LoadConfigFile(Anything &config, const char *name, const char *ext, String &realfilename)
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

		bool LoadConfigFile(Anything &config, const char *name, const char *ext)
		{
			String dummy;
			return LoadConfigFile(config, name, ext, dummy);
		}

		namespace IO {
			int access(const char *path, int amode)
			{
				StartTrace(System.IO.access);
				return ::access(path, amode);
			}

			int mkdir(const char *path, int pmode)
			{
				StartTrace1(System.IO.mkdir, "path [" << NotNull(path) << "]");
			#if defined(WIN32)
				// no access mode possible for win32
				return ::mkdir(path);
			#else
				return ::mkdir(path, pmode);
			#endif
			}

			int rmdir(const char *path)
			{
				StartTrace1(System.IO.rmdir, "path [" << NotNull(path) << "]");
				return ::rmdir(path);
			}

			int unlink(const char *path)
			{
				StartTrace1(System.IO.unlink, "path [" << NotNull(path) << "]");
				return ::unlink(path);
			}

			int rename(const char *oldfilename, const char *newfilename)
			{
				StartTrace1(System.IO.rename, "oldname [" << NotNull(oldfilename) << "] newname [" << NotNull(newfilename) << "]");
				return ::rename(oldfilename, newfilename);
			}
		}

	}
}
