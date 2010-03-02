/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

//--- interface include --------------------------------------------------------
#include "MmapStream.h"

#if !defined(WIN32)
//--- standard modules used ----------------------------------------------------
#include "System.h"
#include "SystemLog.h"

//--- c-library modules used ---------------------------------------------------
#include <fcntl.h>

#if defined(linux) && ! defined(MAP_FAILED)
// missing #define in mmap.h
#define MAP_FAILED ((void*)-1)
#endif

const int MmapStreamBuf::openprot = 0644;

static long roundtopagesize(long pos, bool up = true)
{
	// getpagesize() is invariant, we should not get threading problems here:
	static const long ps = getpagesize();
	long ret = 0;
	if (up) {
		ret = ((long(pos - 1) / ps) + 1) * ps;
	} else {
		ret = (long(pos) / ps) * ps;
	}
	return ret;
}

MmapMagicFlags::MmapMagicFlags(int omode, int syncflag )
	: fProtection(eRead)
	, fSyncFlag(syncflag)
	, fOpenMode(0)
	, fSyscallOpenMode(0)
{
	// adjust mode to output if append or at-end implies it
	if (omode & (ios::ate | ios::app)) {
		omode |= ios::out;
	}
	// set it for read only input
	if (omode & ios::in) {
		fSyscallOpenMode = O_RDONLY;
		fProtection = eRead;
	}
	if (omode & ios::out) {
		if (!(omode & (ios::app | ios::ate))) {
			// comply with fstream, truncate if not appending
			omode |= ios::trunc;
		}
		fSyscallOpenMode = O_RDWR | O_CREAT;
		fProtection |= eWrite;
		if (omode & ios::trunc) {
			fSyscallOpenMode |= O_TRUNC;
		}
	}
#if defined(__SUNPRO_CC) && __SUNPRO_CC<0x500
	// brain dead iostream library from sun requires this hack, alway have files
	// writable. bad luck if a file is accessible read only.... :-(
	fSyscallOpenMode |= O_RDWR;
	fProtection |= eWrite;
#endif
	fOpenMode = omode;
}

MmapStreamBuf::MmapStreamBuf(int syncflag)
	: fMapFd(-1)
	, fLength(0)
	, fMapOffset(0)
	, fFileLength(0)
	, fAddr(0)
	, fFl(0, syncflag)
{
}

MmapStreamBuf::~MmapStreamBuf()
{
	this->close();
}

inline void MmapStreamBuf::AdjustFileLength()
{
	if ( pbase() && fFl.IsIosOut() ) {
		long newlen = (pptr() - pbase()) + fMapOffset;
		if (newlen > fFileLength) {
			fFileLength = newlen;
		}
	}
}

void MmapStreamBuf::close()
{
	sync();
	if (fAddr) {
		munmap((char *) fAddr, fLength - fMapOffset);
	}
	if (fMapFd >= 0) {
		if (fFl.IsIosOut()) {
			// do the tricky stuff for files written
			if (fFileLength != fLength) { // the file has wrong length
				::ftruncate(fMapFd, fFileLength);
			}
		}
		::close(fMapFd); // really close it
	}
	fMapFd = -1;
	fFileLength = 0;
	fAddr = 0;
	fLength = 0;
	fMapOffset = 0;
	fFl = MmapMagicFlags(0, MmapStreamBuf::eSync);
	// adjust now invalid buff ptrs
	//setb(0,0,0);
	setp(0, 0);
	setg(0, 0, 0);
}

MmapStreamBuf *MmapStreamBuf::open(const char *path, int mode/*= ios::in*/, int prot/*=0666*/)
{
	// do the nasty work of setting up the stuff.
	if (path) {
		fFl = MmapMagicFlags(mode, fFl.GetSyncFlag());
		if (fFl.GetIOSMode() & ios::trunc) {
			fLength = 0;
			fMapOffset = 0;
		}
		int fd = ::open(path, fFl.GetSyscallOpenMode(), prot);
		if (fd >= 0) {
			if (0 <= (fLength = lseek(fd, 0, SEEK_END))) {
				fFileLength = fLength;
				// PS: here we could optimize in case of ios::app
				// and reserve up to the current block-size, however
				// this overhead only occurs for the first time write
				if (xinit(fd)) {
					// fix for _M_mode initialisation problem of gcc 3.2
					if ( (fFileLength == 0) && (mode & ios::out) ) {
						reserve(16L);
					}
					return this;
				}
			} else {
				fLength = 0;
				fMapOffset = 0;
			}
		}

	}
	// we failed to open the file....
	return 0;
}

bool MmapStreamBuf::isvalid()
{
	if (fAddr) {
		return true; // everything OK.
	} else {
		// several special cases
		if (fMapFd >= 0) { // the file exists
			// yes there is a file
			if (fFl.IsIosOut() && fFl.IsWriteable()) {
				return true;    // we can extend it
			}
			if (fFl.IsIosIn() && fFl.IsReadable() &&
				fFileLength == 0 && fLength == 0) {
				return true;    // the file was empty, so everything is OK until we read
			}
		}
	}
	return false;
}

bool MmapStreamBuf::xinit(int fd)
{
	// hey now we "just" need to mmap it.
	long putoffset = 0; // remember offsets if we readjust the mmap/file
	long getoffset = 0;
	if (fd >= 0) {
		if (fAddr != 0) { // we need to unmap first
			Assert(fAddr == pbase());// fAddr should always be base()
			if (pptr() > pbase() || fMapOffset > 0) {
				putoffset = pptr() - pbase() + fMapOffset; // remember where we have been
				if (putoffset > fFileLength) {
					fFileLength = putoffset;
				}
			}
			Assert(fAddr == eback());
			if (gptr() > eback()) {
				getoffset = gptr() - eback();
			}
			if (0 > munmap(fAddr, fLength - fMapOffset)) { // re-mapping after adjustment of size
				// oops munmap fails, potential vm leak notify
				SystemLog::Error("MmapStreamBuf::xinit: munmap failed, potential VM leak");
			}
		}
		if (fMapFd != fd) {
			if (fMapFd >= 0) {
				if (fFl.IsIosOut() && fFileLength != fLength) {
					// the file has wrong length
					::ftruncate(fMapFd, fFileLength);
				}
				::close(fMapFd); // really close it
			}
			fMapFd = fd;
		}
		fLength = lseek(fd, 0, SEEK_END); // determine (new) file length for mmap
		if (fLength < 0) {
			fLength = 0;    // adjust in case of errors
		}
	}
	fAddr = 0;
	fMapFd = fd;
	if (fLength > 0) {
		// an empty file, if it is for writing this is OK
		// we do not map it yet. reserve will take care
		// recalc fMapOffset it is one page below putoffset if we
		// are writing a file, for readable files always 0,
		if ((fFl.IsIosOut()) && !(fFl.IsIosIn()) && putoffset > 0) {
			fMapOffset = roundtopagesize(putoffset, false); // round down
		}
		fAddr = (char *)mmap(0, fLength - fMapOffset, fFl.GetMmapProtection(), eShared, fMapFd, fMapOffset);
		if (MAP_FAILED == fAddr) {
			fAddr = 0;
			fMapOffset = 0;
			fLength = 0;
			::close(fMapFd); // clean up
			fMapFd = -1;
			return false;
		}
	}

	if ((fFl.IsIosAppendOrAtEnd()) && putoffset < fFileLength  ) {
		putoffset = fFileLength; // adjust it to the end for app and ate
		fFl.IosClearAtEnd(); // do it only once
	}
	//setb((char*)fAddr,(char*)fAddr+fLength-fMapOffset,0);
	setg((fFl.IsReadable()) ? (char *)fAddr : (char *)fAddr + fFileLength - fMapOffset,
		 (char *)fAddr + getoffset ,
		 (char *)fAddr + fFileLength - fMapOffset);
	// now init get an put area
	setp((char *)fAddr, (char *)fAddr + fLength - fMapOffset);
	pbump(putoffset - fMapOffset); // to allow for backing up, because setb is no longer supported
	// calculate error: true means OK = either the file was non empty or we write
	// reserve() will take care of allocating the space for us when writing
	return isvalid();
}

// we only get overflow if the file is written past the current end
// adjust the file size accordingly and reset the file
int MmapStreamBuf::overflow( int c )
{
	Assert( pbase() ? pbase() <= pptr() : pbase() == pptr() );

	if (c != EOF) {
		// use base() in calculation, because we only use one buffer
		if (reserve(pptr() - pbase() + 1 + fMapOffset)) { // this will add a page or adjust to the page size
			Assert(pptr() < epptr()); // did we get space?
			return sputc(c);
		}
		// we failed somehow. return EOF below
	}
	setp((char *)fAddr, (char *)fAddr + fLength - fMapOffset);   // reset put area
	return EOF;
} // overflow

int MmapStreamBuf::underflow()
{
	AdjustFileLength(); // look if we wrote something, we can now read.
	if (gptr() && gptr() - eback() < fFileLength) {
		// re-adjust egptr, because we have something still available
		setg(eback(), gptr(), (char *)fAddr + fFileLength - fMapOffset);
	}
	if (gptr() < egptr()) {
		return ZAPEOF(*gptr());    // we still got something
	}
	return EOF; // we never handle underflow, because our buffer is the file content
} // underflow

int MmapStreamBuf::syncOutput()
{
	AdjustFileLength();
	// schedule msync
	if ( 0 == msync(fAddr, fFileLength - fMapOffset, fFl.GetSyncFlag()) ) {
		// to keep it fast, use option: MS_ASYNC
		// MS_SYNC is slow but works for log files, therefore the default
		// adjust filelength to current put-position
		return 0; // all OK
	} else {
		return EOF;
	}
}

int MmapStreamBuf::sync()
{
	int ret = 0;
	if (fFl.IsIosOut()) { // fProtection&eWrite)
		ret = syncOutput();
	}

	return ret;
}

bool MmapStreamBuf::reserve(long newlength)
{
	// we might check if newlength is really bigger than fLength
	// second, only if fProtection includes eWrite it is possible to enlarge the file
	if (newlength > fLength) {
		Assert((epptr() - pbase()) == fLength - fMapOffset);
		if (fFl.IsWriteable()) {
			long roundedlength = roundtopagesize(newlength);
			// the following seek could also be absolute:
			long lengthminus1 = lseek(fMapFd, roundedlength - 1, SEEK_SET);
			//long lengthminus1 = lseek(fMapFd,roundedlength-fLength-1,SEEK_END);
			// was fLength = lseek(fMapFd,roundedlength-fLength-1,SEEK_END);
			if (lengthminus1 == -1 || 1 != write(fMapFd, "", 1)) {
				// write something to create the pages in the file
				// OOPS we got an error
				SystemLog::Error("MmapStreamBuf::reserve file could not append, not open?");
				return false;
			}
			// adjust the buffer info
			return xinit(fMapFd);
		} else {
			return false;
		}
	}
	return true; // space is still available, false alarm
}

//
// if p is in range it sets the position for further operations
// otherwise it returns EOF;
//
MmapStreamBuf::pos_type MmapStreamBuf::seekpos(MmapStreamBuf::pos_type p, MmapStreamBuf::openmode mode)
{
	AdjustFileLength();
	if (long(p) > long(fLength)) {
		// we need to enlarge the file
		// we can only if we write
		if (! (mode & ios::out) || !reserve((long)p)) {
			// OOPS we got a problem
			return pos_type(EOF);
		}
	}
	if (mode & ios::in) {
		setg(fFl.IsReadable() ? (char *)fAddr : (char *)fAddr + fFileLength - fMapOffset, fAddr + p, (char *)fAddr + fFileLength - fMapOffset);
	}
	if (mode & ios::out) {
		if (fFl.IsIosApp()) {
			// do this on a best try basis
			if (p < fFileLength && fFileLength > 0) {
				p = fFileLength;    // always go to the end
			}
		}
		// need to consider fMapOffset, remap things if needed
		setp((char *)fAddr, (char *)fAddr + fLength - fMapOffset);
		pbump(p - fMapOffset); // to allow for backing up, because setb is no longer supported
		if (fMapOffset > 0 && p - fMapOffset < 0) {
			// we want to go to an unmapped page, should call
			xinit(fMapFd);
		}
	}
	return p;
} // seekpos

// implement putback behavior for streams

int MmapStreamBuf::pbackfail(int c)
{
	// move get pointer
	// only happens for read-only streams
	if (c != EOF) {
		// sanity check
		if ( fFl.IsIosOut()) {
			// we also can write
			if ( eback() && eback() < gptr() ) {
				// adjust get pointer
				gbump(-1);
				*(gptr()) = char(c); // store the characte
				return c;
				// this goes into endless recursion    return sputbackc(c);
			}
		} else {
			// we got a readonly streambuf
			if (gptr() && gptr() > eback() && c == *(gptr() - 1)) {
				// sanity check
				//setg(eback(),gptr()-1,egptr()); // just adjust gptr()
				gbump(-1);
				return ZAPEOF(c);
			}
		}
	}
	return EOF; // fail for putback
}

//
// Sets the relative position for further get operations;
// returns EOF if out of range or input isn't allowed;
//
MmapStreamBuf::pos_type MmapStreamBuf::seekoff(MmapStreamBuf::off_type of, MmapStreamBuf::seekdir dir, MmapStreamBuf::openmode mode)
{
	//sync(); // will adjust fFileLength if needed
	long pos = long(of);
	if (dir == ios::cur) {
		pos += long((mode & ios::in ? gptr() : pptr()) - (char *)fAddr + fMapOffset);
	} else if (dir == ios::end && fFileLength > 0) {
		pos += long(fFileLength);
	}
//	pos += (dir == ios::cur) ? long((mode&ios::in ? gptr() : pptr())-(char*)fAddr) :
//			(dir == ios::end && fFileLength >0) ? long(fFileLength) : 0L;
	if (pos < 0L ) {
		return pos_type(EOF);
	}
	return seekpos(pos, mode);
} // seekoff

MmapStreambase::MmapStreambase(const char *name, int omode, int prot, int syncflag)
	: fMmapBuf(syncflag)
{
	this->open(name, omode, prot);
}

void MmapStreambase::open(const char *name, int omode, int prot)
{
	if (this->is_open()) {
		this->close();
	}
	ios::init(&fMmapBuf);
	ios::clear(); // clear all flags, for std::iostream must be done after init!
	if (&fMmapBuf != fMmapBuf.open(name, omode, prot)) {
		// recognize the error
		ios::clear(ios::eofbit | ios::badbit); // set eof and bad stream bits
	}
}

#endif
