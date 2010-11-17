/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _MmapStream_H
#define _MmapStream_H

#if !defined(WIN32)
#include <sys/mman.h>

#include <cstdio>
#include <iostream>
#include <iomanip>

//!helper class to manage dependencies between ios flags and c-api mmap flags
class EXPORTDECL_FOUNDATION MmapMagicFlags
{
public:
	MmapMagicFlags(int omode, int syncflag);
	int GetIOSMode() {
		return fOpenMode;
	}
	int GetSyscallOpenMode() {
		return fSyscallOpenMode;
	}
	int GetMmapProtection() {
		return fProtection;
	}
	int GetSyncFlag() {
		return fSyncFlag;
	}

	bool TestIos(int mode) {
		return fOpenMode & mode;
	}

	bool IsIosOut() {
		return TestIos(std::ios::out);
	}
	bool IsIosIn() {
		return TestIos(std::ios::in);
	}
	bool IsIosApp() {
		return TestIos(std::ios::app);
	}
	bool IsIosAppendOrAtEnd() {
		return TestIos( std::ios::app | std::ios::ate);
	}
	void IosClearAtEnd() {
		if (TestIos(std::ios::ate)) {
			fOpenMode &= ~std::ios::ate; // adjust it only once for ate
		}
	}
	enum { eRead = PROT_READ, eWrite = PROT_WRITE } ;

	bool IsReadable() {
		return (fProtection & eRead);
	}
	bool IsWriteable() {
		return (fProtection & eWrite);
	}

private:
	int    fProtection; // PROT_READ or PROT_WRITE
	int    fSyncFlag; // eSync or eAsync for subsequent sync calls
	int    fOpenMode; // save ios open mode from params
	int    fSyscallOpenMode; // calculate param for ::open() syscall
};

//! streambuf class implementing mmap based file access on platforms supporting it
class EXPORTDECL_FOUNDATION MmapStreamBuf : public std::streambuf
{
	friend class MmapStreambase;
public:
	// wrap some constants from mmap.h
	enum { ePrivate = MAP_PRIVATE, eShared = MAP_SHARED } ;
	// we only support MAP_SHARED
	enum { eSync = MS_SYNC, eAsync = MS_ASYNC, eInvalidate = MS_INVALIDATE };
	//! default protection for new files rw-rw-r-- 0664
	static const int openprot;

	//! default constructor creating a closed streambuf
	MmapStreamBuf(int syncflag = eSync);

	//! destructor, munmaps the open file
	virtual ~MmapStreamBuf();
	//! schedule writing the file,
	virtual int sync();        // flushes the buffer
	//! query if streambuf is connected to a file, i.e. it is valid
	bool is_open() {
		return fMapFd >= 0 ;    // could also be fAddr != 0
	}
	//! return current length of mapped file
	long Length() {
		return fLength;
	}

protected: // seekxxx are protected in the std..
	typedef std::streambuf::pos_type pos_type;
	typedef std::streambuf::off_type off_type;
	typedef std::ios::seekdir seekdir;
	typedef std::ios::openmode openmode;

	//! standard iostream behavior, adjust put or get position absolutely
	virtual pos_type seekpos(pos_type pos, openmode mode = std::ios::in | std::ios::out);
	//! standard iostream behavior, adjust put or get position relatively
	virtual pos_type seekoff(off_type off, seekdir, openmode mode = std::ios::in | std::ios::out);

	//! auxiliary operation to keep track of really written bytes
	void AdjustFileLength() ;
	//!opens and map the file
	//! \return returns this if OK, 0 otherwise
	MmapStreamBuf *open(const char *path, int mode = std::ios::in, int prot = MmapStreamBuf::openprot);
	//! clean up and set this to nascent state
	void close();
	//! extends the mapped object to an additional next page
	virtual int overflow(int c = EOF);
	//!couldn't produce new chars because we always map the whole file
	virtual int underflow();
	// future optimisation might do this page-wise
	// then underflow and overflow might
	// really do some work.
	//! need to implement because, we support putback
	virtual int pbackfail(int c);

private:
	//! test the validity of the streambuf at the end of xinit()
	bool isvalid();
	//! set up the mmap and return success
	bool xinit(int fd);
	//!expand file size and mapped region to new length
	//! \param newlength the new size of the file, increment step is page size
	bool reserve ( long newlength );
	// mmap doesn't give you input buffering, because it is not needed
	//	int syncInput();        // flushes the input buffer -- not needed
	//! flushes the output buffer, schedules an msync
	int syncOutput();
	// calc buffer addresses in one place
	char *start() {
		return (char *)fAddr;
	}
	char *end() {
		return (char *)fAddr + fLength - fMapOffset;
	}

	int    fMapFd;    // filedescriptor used with mmap
	long   fLength;    // length of the mapped space,
	// for extension a multiple of pagesize
	off_type fMapOffset; // for large files written, we only map from fMapOffset to the end
	long	fFileLength; // the real number of bytes within the file <= fLength
	char *fAddr;  // zero or the base addr of the mapped file
	MmapMagicFlags fFl; // all the relevant flags
}; // MmapStreamBuf

//! adapts ios to a Mmap buffer --> THIS CLASS IS NOT TO BE INSTANTIATED
//! may be unsafe_ios
class EXPORTDECL_FOUNDATION MmapStreambase : virtual public std::ios
{
public:
	//! constructor of empty stream
	MmapStreambase() {
		init(&fMmapBuf);
	}
	//! normal constructor opening a file corresponds to fstream ctor
	MmapStreambase(const char *name,
				   int mode = std::ios::in,
				   int prot = MmapStreamBuf::openprot,
				   int syncflag = MmapStreamBuf::eSync);
	//! dtor for cleaning up
	virtual ~MmapStreambase() {}

	//!similar to contstructor for reattaching a file to the stream
	void open(const char *path, int mode, int prot = MmapStreamBuf::openprot);
	//!query open state on the stream
	bool is_open()  {
		return rdbuf()->is_open();
	}
	//!disconnect file from stream
	void close() {
		rdbuf()->close();
		setstate(std::ios::eofbit);
	}
	//!let streambuf leak
	MmapStreamBuf *rdbuf()  {
		return &fMmapBuf;
	}

protected:
	MmapStreamBuf fMmapBuf; // the buffer with its underlying mmap'ed file
}; // MmapStreambase

//!implementation of istream with mmap
class  EXPORTDECL_FOUNDATION IMmapStream : public MmapStreambase, public std::istream
{
public:
	//!constrcutor to use for read-only streams
	IMmapStream(const char *path) : MmapStreambase(path, std::ios::in)
		, std::istream(&fMmapBuf)
	{ }
	~IMmapStream() {}
}; // IMmapStream

//!implementation of ostream with mmap
class  EXPORTDECL_FOUNDATION OMmapStream : public MmapStreambase, public std::ostream
{
public:
	//! constructor for output streams opening a file
	OMmapStream(const char *path,
				int mode = std::ios::out, int prot = MmapStreamBuf::openprot,
				int syncflag = MmapStreamBuf::eSync)
		: MmapStreambase(path, std::ios::out | mode, prot, syncflag)
		, std::ostream(&fMmapBuf)
	{  }
	virtual ~OMmapStream() { }
}; // OMmapStream

//!implementation of iostream with mmap
class  EXPORTDECL_FOUNDATION MmapStream : public MmapStreambase, public std::iostream
{
public:
	//! constructor for intput/output streams opening a file
	MmapStream(const char *path, int mode = std::ios::out | std::ios::in, int prot = MmapStreamBuf::openprot,
			   int syncflag = MmapStreamBuf::eSync)
		: MmapStreambase(path, mode, prot, syncflag)
		, std::iostream(&fMmapBuf)
	{  }
	virtual ~MmapStream() { }
}; // MmapStream

#endif
#endif
