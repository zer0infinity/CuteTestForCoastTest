/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _ZipStream_H
#define _ZipStream_H

#include "config_compress.h"
#include "StringStream.h"
#include "zlib.h"

class ZipIStream;

//---- ZipStreamBuf ----------------------------------------------------------
//! Does not do anything in particular except hold a few static constants.
//! Once could merge ZipOStreamBuf and ZipIStreamBuf into this class, but
//! this would result in a very complex class.
class EXPORTDECL_COMPRESS ZipStreamBuf : public streambuf
{
public:
	static const unsigned char fgcGzSimpleHeader[10];	// definition og GzHeader see RFC 1952

protected:
	static const unsigned char fgcZDeflated;

#if defined(WIN32)
	// nt os code
	static const unsigned char fgcOsCode;
#else
	// unix os code
	static const unsigned char fgcOsCode;
#endif

	static const long fgcStoreSize;
};

//---- ZipIStreamBuf ----------------------------------------------------------
//! wrap other istream objects with decompression
class EXPORTDECL_COMPRESS ZipIStreamBuf : public ZipStreamBuf
{
public:
	//--- constructors
	ZipIStreamBuf(istream &zis, istream &, Allocator *a = Storage::Current());

	~ZipIStreamBuf();

	void close();

protected:
	//! consumes chars of the put area
	virtual int overflow(int c = EOF);

	//! produces characters for the get area
	virtual int underflow();

	//! defines the holding area for the streambuf
	void xinit();
	void pinit(long);
	void zipinit();

	void runInflate();
	void fillCompressed();

	//! not much to do when synchronizing, just insert string termination character
	virtual int sync();

	//! get long from underlying stream
	unsigned long getLong();

	//! the storage of the holding area buffer
	String fStore;
	//! the storage of the compressed buffer
	String fCompressed;

	istream &fZis;
	istream *fIs;

	Allocator *fAllocator;
	z_stream_s fZip;
	unsigned long fCrc;

private:
	bool isInitialized;
	long fZipErr;
};

//---- ZipOStreamBuf ----------------------------------------------------------
//! wrap other ostream objects with compression compression
class EXPORTDECL_COMPRESS ZipOStreamBuf : public ZipStreamBuf
{
public:
	//--- constructors
	ZipOStreamBuf(ostream &, Allocator *a = Storage::Current());

	~ZipOStreamBuf();

	//! not much to do when synchronizing, just insert string termination character
	virtual int sync();
	void close();
#if !defined(ONLY_STD_IOSTREAM) /* no need to overwrite seek hooks */
	//! flushes the buffer and sets streampos to 0 it is not possible to seek on a socket
	virtual streampos seekpos(streampos, int mode = ios::in | ios::out);

	//! flushes the buffer and sets streampos to 0 it is not possible to seek on a socket
	virtual streampos seekoff(streamoff, ios::seek_dir, int mode = ios::in | ios::out);
#endif
protected:
	//! consumes chars of the put area
	virtual int overflow(int c = EOF);

	//! produces characters for the get area
	virtual int underflow();
	//! defines the holding area for the streambuf
	void xinit();
	void pinit();
	void zipinit();
	void flushCompressed();
	void flushCompressedIfNecessary();
	void putLong(unsigned long);

	Allocator *fAllocator;
	//! the storage of the holding area buffer
	String fStore;
	//! the storage of the compressed buffer
	String fCompressed;
	ostream *fOs;

	unsigned long fCrc;
	z_stream_s fZip;

private:
	bool isInitialized;
};

//---- ZipOStream ----------------------------------------------------------
//! wrap other ostream objects with compression
class EXPORTDECL_COMPRESS ZipOStream : public ostream
{
public:
	//--- constructors
	ZipOStream(ostream &os):
#if defined(ONLY_STD_IOSTREAM)
		ostream(&fBuf),
#endif
		fBuf(os) {
		init(&fBuf);
	}
	~ZipOStream() {
		close();
	}
	ZipStreamBuf *rdbuf()  {
		return &fBuf;
	}
	void close() {
		fBuf.close();
	}
protected:
	ZipOStreamBuf fBuf;
};

//---- ZipOStreamBuf ----------------------------------------------------------
//! wrap other istream objects with decompression
class EXPORTDECL_COMPRESS ZipIStream : public istream
{
public:
	//--- constructors
	ZipIStream(istream &is):
#if defined(ONLY_STD_IOSTREAM)
		istream(&fBuf),
#endif
		fBuf(*this, is) {
		init(&fBuf);
	}
	~ZipIStream() {
		close();
	}
	ZipStreamBuf *rdbuf()  {
		return &fBuf;
	}
	void close() {
		fBuf.close();
	}
protected:
	ZipIStreamBuf fBuf;
};

#endif
