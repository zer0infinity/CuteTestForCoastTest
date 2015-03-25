/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _SYSTEMFILE_H_
#define _SYSTEMFILE_H_

#include "Anything.h"

//  definitions for io namespace
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

namespace coast {
	namespace system {

		typedef std::ios::openmode openmode;

		//! opens an iostream, search path is not used
		/*! tries to open an iostream in the given mode.
			\param path the filepath, it can be relative (to the current working directory) or absolute
			\param mode ios mode flags
			\return the pointer to the open iostream or NULL, the client is responsible for destruction */
		std::iostream *OpenStream(const String &path, openmode mode = (std::ios::in));

		//! opens an istream for reading, search path is not used
		/*! \param path the filepath, it can be relative (to the current working directory) or absolute
			\param mode ios mode flags
			\return the pointer to the open iostream or NULL, the client is responsible for destruction */
		std::iostream *OpenIStream(const String &path, openmode mode = (std::ios::in));

		//! opens an ostream for writing, search path is not used
		/*! \param path the filepath, it can be relative (to the current working directory) or absolute
			\param mode ios mode flags
			\return the pointer to the open iostream or NULL, the client is responsible for destruction */
		std::iostream *OpenOStream(const String &path, openmode mode = std::ios::out | std::ios::trunc);

		//! opens an iostream, uses search path
		/*! tries to open an iostream in the given mode. Uses fgRootDir and fgPathList variables to search
		    for the file unless it is an absolute filename. If a relative filename is not found it is interpreted
		    relative to the current working directory.
		    \param path the filepath, it can be relative (searched in fgRootDir/fgPathList) or absolute
			\param mode ios mode flags
			\return the pointer to the open iostream or NULL, the client is responsible for destruction */
		std::iostream *OpenStreamWithSearch(const String &path, openmode mode = (std::ios::in));

		//! opens an iostream, uses search path
		/*! tries to open an iostream in the given mode. Uses fgRootDir and fgPathList variables to search
		    for the file unless it is an absolute filename. If a relative filename is not found it is interpreted
		    relative to the current working directory.
		    \deprecated use OpenStream(const String&, openmode) or OpenStreamWithSearch(const String&, openmode)
			\param name the filename, it can be relative (searched in fgRootDir/fgPathList) or absolute
			\param extension the extension of the file
			\param mode ios mode flags
			\param log turn SystemLog::Debug messages on or off
			\return the pointer to the open iostream or NULL, the client is responsible for destruction	*/
		std::iostream *OpenStream(const char *name, const char *extension, openmode mode = (std::ios::in), bool log = false);

		//! opens an istream for reading, search path is not used
		/*! \deprecated use OpenIStream(const String&, openmode)
			\param name the filename, it can be relative (to the current working directory) or absolute
			\param extension the extension of the file
			\param mode ios mode flags
			\param log turn SystemLog::Debug messages on or off
			\return the pointer to the open iostream or NULL, the client is responsible for destruction */
		std::iostream *OpenIStream(const char *name, const char *extension, openmode mode = (std::ios::in), bool log = false);

		//! opens an ostream for writing, search path is not used
		/*! \deprecated use OpenOStream(const String&, openmode)
		    \param name the filename, it can be relative (to the current working directory) or absolute
			\param extension the extension of the file
			\param mode ios mode flags
			\param log turn SystemLog::Debug messages on or off
			\return the pointer to the open iostream or NULL, the client is responsible for destruction */
		std::iostream *OpenOStream(const char *name, const char *extension, openmode mode = std::ios::out | std::ios::trunc, bool log = false);

		//! returns the path and filename.extension of a file found using the search path
		String GetFilePath(const char *name, const char *extension);

		//! returns the path of a file found using the search path
		String GetFilePath(const String &relpath);

		//! returns the path of a file found using the search path or the relpath parameter
		String GetFilePathOrInput(const String &relpath);

		//! returns the system/user temp path
		String GetTempPath();

		//! returns directory entries as anything, entries are filtered by extension filter
		Anything DirFileList(const char *dir, const char *extension = "html");

		//! loads a config file as anything, returns resolved path
		bool LoadConfigFile(Anything &config, const char *name, const char *ext, String &realfilename);

		//! load a config file as anything and do not care where from
		bool LoadConfigFile(Anything &config, const char *name, const char *ext = "any");

		//! this sets the GLOBAL root directory where the search starts
		/*! \param rootdir the root directory
			\param print if true prints the new settings to cerr */
		void SetRootDir(const char *rootdir, bool print = false);

		//! this sets the GLOBAL pathlist, a ':' delimited list of relative paths ( relative to rootdir )
		/*! \param pathlist the pathlist e.g. config:src:test
			\param print if true prints the new settings to cerr */
		void SetPathList(const char *pathlist, bool print = false);

		//! returns the current rootdir for this process
		const char *GetRootDir();

		//! returns the current pathlist for this process
		const char *GetPathList();

		//! returns the system's path seperator e.g. '/' on unix
		char Sep();

		//! determines if path is a regular file
		/*! \param path fully qualified path to file which should be checked
			\return true if file is a regular file */
		bool IsRegularFile(const char *path);

		//! determines if path points to a directory
		/*! \param path fully qualified path to check
			\return true if path is a directory */
		bool IsDirectory(const char *path);

		//!determines if path is a symbolic link
		/*! \param path fully qualified path to check
			\return true if path is a symbolic link */
		bool IsSymbolicLink(const char *path);

		//!determines the size of a file if it exists, works only for files less than 2^31 bytes for now!
		/*! \param path fully qualified path to file
			\param ulFileSize parameter receiving the file size in bytes
			\return false if the underlying system call failed */
		bool GetFileSize(const char *path, ul_long &ulFileSize);

		//!determines the number of free blocks on the filesystem pointed to by pFsPath
		/*! \param pFsPath any path within the filesystem to get the free blocks
			\param ulFreeBlocks parameter receiving the number of free blocks
			\param lBlkSize parameter receiving the block size in bytes
			\return false if the underlying system call failed */
		bool BlocksLeftOnFS(const char *pFsPath, ul_long &ulFreeBlocks, unsigned long &lBlkSize);

		//! resolve full pathname to the given filename
		/*! \param fullPathName - contains the full pathname including filename to the given file if successful
			\param file - filename including extension of the file to be found
			\param path - searchpath to be used for finding the file
			\return success of finding the file */
		bool FindFile(String &fullPathName, const char *file, const char *path = 0);

		//! cleans the given path by resolving relative movements
		/*! \note WIN32 special: also corrects drive notation
			\param path resolves given path, strips ./ sections, resolves relative movements and unifies slashes
			\return path returned through path parameter */
		void ResolvePath(String &path);

		//! returns the drive letter for WIN32
		/*! \param name path to get drive letter from
			\param drive returned drive letter if any
			\return true if the path contained a drive letter */
		bool GetDriveLetter(const char *name, char &drive);

		//! remove drive letter from path
		/*! \param path path from which the drive letter including the colon should be removed */
		void StripDriveLetter(String &path);

		//! checks path whether it is absolute or not
		/*! \param path fully qualified path to check if absolute
			\return true if given path is absolute */
		bool IsAbsolutePath(const char *path);

		//! obtain current working directory
		/*! \param cwd variable which takes the current working directory */
		void GetCWD(String &cwd);

		//! set current working directory
		/*! \param dir value with the new current working directory
			\return true if success */
		bool ChangeDir(const String &dir);

		//! change file or directory permission
		/*! \param filename fully qualified path to file whose mode should be adjusted
			\param pmode BIT-combination of new mode (see <sys/stat.h> for bitvalues)
			\return 0 upon success, -1 otherwise */
		int Chmod(const char *filename, int pmode);

		//! initialization routine that sets fgRootDir and fgPathList, does not take parameters ;-)
		/*! Since OpenStream uses fgRootDir and fgPathList we have to make sure that they contain reasonable defaults whenever they are used.
			fgRootDir is initialized to the environment variable COAST_ROOT if set, to '.' otherwise
			fgPathList is initialized to the environment variable COAST_PATH if set, to ".:config:src" if root is set and to ".:../config:../src:" if root is not set
			\param root path to be used when wanting to override default of COAST_ROOT
			\param path path to be used when wanting to override default of COAST_PATH */
		void InitPath(const char *root = 0, const char *path = 0);

		/*! Status code of directory creation operations. */
		enum DirStatusCode {
			eSuccess = 1,									//! directory creation was successful
			eFailed = eSuccess << 1,						//! something went wrong, check other bits to find reason
			ePathEmpty = (eFailed | (eSuccess << 2) ),		//! given path parameter was empty
			eExists = (eFailed | (eSuccess << 3) ),			//! given directory already exists
			ePathTooLong = (eFailed | (eSuccess << 4) ),	//! path exceeds system limit
			eNotExists = (eFailed | (eSuccess << 5) ),		//! directory does not exist
			eNotDirectory = (eFailed | (eSuccess << 6) ),	//! a part of the path given is not a directory
			eNoMoreHardlinks = (eFailed | (eSuccess << 7) ),//! directory creation failed because no more hardlinks available (symlinks are still possible?!)
			eRecurseDeleteNotAllowed = (eFailed | (eSuccess << 8) ),//! not allowed to delete directory recursively
			eNoSuchFileOrDir = (eFailed | (eSuccess << 9) ),//! no such file or directory
			eNoPermission = (eFailed | (eSuccess << 10) ),	//! no permission to operate
			eQuotaExceeded = (eFailed | (eSuccess << 11) ),	//! disk quota of user exceeded
			eNoSpaceLeft = (eFailed | (eSuccess << 12) ),	//! no more room - free inodes - to create directory, fails also, when fragmentation of FS is bad! -> use fsck to examine
			eCreateSymlinkFailed = (eFailed | (eSuccess << 13) ),	//! was not able to create symbolic link
			eCreateDirFailed = (eFailed | (eSuccess << 14) ),	//! was not able to create directory for a reason not specified yet
			ePathIncomplete = (eFailed | (eSuccess << 15) ),//! given path was not complete or an operation on it lead to failures
			eIOOperationFailed = (eFailed | (eSuccess << 16) ),//! io operation on file was not successful
			ePathIllegal = (eFailed | (eSuccess << 17) ),	//! path argument points to an illegal address
		};

		//! create new directory with given permissions, works for relative or absolute path names and also recursive if specified.
		/*!	\note Unless bRecurse is true, the parent directory must already exist and only one directory level will be created
			\param path relative or absolute path to create new directory
			\param pmode permission of new directory, octal number
			\param bRecurse set to true if nonexisting parent directories should be created
			\param bExtendByLinks if a directory can not be created because its parent dir is exhausted of hard links (subdirectories), a true means to create an 'extension' parent directory of name \<dir\>_ex[0-9]+ and link the newly created directory into the original location
			\return system::eSuccess if new directory was created */
		DirStatusCode MakeDirectory(String &path, int pmode = 0755, bool bRecurse = false, bool bExtendByLinks = false);

		//! remove given directory - relative directories can be deleted recursively
		/*! \param path relative or absolute path of directory to be deleted
			\param bRecurse true if relative directory should be deleted recusrively
			\return system::eSuccess if directory was removed */
		DirStatusCode RemoveDirectory(String &path, bool bRecurse = false);

#if !defined(WIN32)
		//! create new directory with given permissions, works for relative or absolute path names and also
		/*! \param filename absolute directory or filename to link to
			\param symlinkname absolute name of link
			\return system::eSuccess if new symbolic link was created */
		DirStatusCode CreateSymbolicLink(const char *filename, const char *symlinkname);
#endif

		//! return number of possible hardlinks (directories) within a directory
		/*! \param path relative or absolute path of directory
			\return number of possible hardlinks */
		int GetNumberOfHardLinks(const char *path);

		//! This namespace is a general wrapper for IO specific functions.
		/*! It is not intended to extend the system specific functionality to handle unsafe conditions or whatever but to present an API which hides
			system specific differences or irregularities. */
		namespace io {
			//! check if a given file or directory is accessible using the given mode
			/*! \param path file or directory path
				\param amode permission setting, one or combination of [F_OK|X_OK|W_OK|R_OK]
				\return 0 in case of success, -1 if file does not exist or is not accessible in the given mode, check system::GetSystemError */
			int access(const char *path, int amode);

			//! make a directory, parent directory must exist already
			/*! \param path path to be created, must not contain trailing slash
				\param pmode permission mask of new directory
				\return 0 in case of success, -1 if something went wrong, check system::GetSystemError */
			int mkdir(const char *path, int pmode);

			//! remove a directory
			/*! \param path specify path to delete
				\return 0 in case of success, -1 if something went wrong, check system::GetSystemError */
			int rmdir(const char *path);

			//! remove a file or link
			/*! \param path specify filename and path of file to delete
				\return 0 in case of success, -1 if something went wrong, check system::GetSystemError */
			int unlink(const char *path);

			//! rename a file, path or link
			/*! \param oldfilename specify name of old file, path or link
				\param newfilename specify name of new file, path or link
				\return 0 in case of success, -1 if something went wrong, check system::GetSystemError */
			int rename(const char *oldfilename, const char *newfilename);
		}

	}
}

#endif /* _SYSTEMFILE_H_ */
