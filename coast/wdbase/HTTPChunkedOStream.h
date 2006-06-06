/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _HTTPCHUNKEDOSTREAM_H
#define _HTTPCHUNKEDOSTREAM_H

#include "config_wdbase.h"
#include "StringStream.h"

//---- HTTPChunkedStreamBuf --------------------------------------------------------------

//! Streambuffer for HTTPChunkedOStream.
class EXPORTDECL_WDBASE HTTPChunkedStreamBuf : public streambuf
{
public:
	//! \param os wrapped output stream
	//! \param chunklength max length of a single chunk
	//! \param a memory allocator
	HTTPChunkedStreamBuf(ostream &os, long chunklength, Allocator *a = Storage::Current());

	~HTTPChunkedStreamBuf();

	//! Writes the current content of the buffer chunk encoded to the wrapped ostream.
	virtual int sync();

	//! Writes the rest of the buffer and the termination chunk to the wrapped ostream.
	//! After close() the ostream can not be used anymore.
	void close();

#if defined(ONLY_STD_IOSTREAM)
protected: // seekxxx are protected in the std..
	typedef std::streambuf::pos_type	pos_type;
	typedef std::streambuf::off_type	off_type;
	typedef std::ios::seekdir	seekdir;
	typedef std::ios::openmode	openmode;
#else
	typedef streampos pos_type;
	typedef	streamoff off_type;
	typedef ios::seek_dir seekdir;
	typedef ios::openmode openmode;
#endif

	//! flushes the buffer and sets streampos to 0 it is not possible to seek on a socket
	virtual pos_type seekpos(pos_type, openmode mode = ios::in | ios::out);

	//! flushes the buffer and sets streampos to 0 it is not possible to seek on a socket
	virtual pos_type seekoff(off_type, seekdir, openmode mode = ios::in | ios::out);

protected:
	//! consumes chars of the put area
	virtual int overflow(int c = EOF);

	//! produces characters for the get area
	virtual int underflow();

	//! defines the holding area for the streambuf
	void xinit();
	void pinit();

	Allocator *fAllocator;

	//! the storage of the holding area buffer
	String fStore;

	//! the wrapped ostream
	ostream *fOs;

private:
	//! size of the internal buffer, is equal to the chunk size
	long fBufSize;
};

//---- HTTPChunkedStreamBase --------------------------------------------------------------

//! Base class for HTTPChunkedStream classes (see RFC 2068 sec 3.6).
class EXPORTDECL_WDBASE HTTPChunkedStreamBase : virtual public ios
{
public:
	//! \param os wrapped output stream
	//! \param chunklength max length of a single chunk
	//! \param mode ios::out or ios::in
	HTTPChunkedStreamBase(ostream &os, long chunklength, int mode = ios::out)
		: fBuf(os, chunklength) {
		init(&fBuf);
	}
	// init from ios is needed, because
	// ios() won't do the job;
	// (see comment in iostream.h)

	//! Gets the internal buffer of the stream.
	HTTPChunkedStreamBuf *rdbuf()  {
		return &fBuf;
	}

	//! Writes the rest of the buffer and the termination chunk to the wrapped ostream.
	//! After close() the ostream can not be used anymore.
	void close() {
		fBuf.close();
	}

protected:
	HTTPChunkedStreamBuf fBuf;
};

//---- HTTPChunkedOStream --------------------------------------------------------------

//! Output stream filter to generate http chunked encoding (see RFC 2068 sec 3.6).
//! Can be used if the content length of the generated server output is not known in advanced,
//! but persistent connections are required.
//! The HTTP header must contain the field "Transfer-Encoding: chunked".
class EXPORTDECL_WDBASE HTTPChunkedOStream : public HTTPChunkedStreamBase, public ostream
{
public:
	//! \param os wrapped output stream
	//! \param chunklength max length of a single chunk (deafault 1024)
	HTTPChunkedOStream(ostream &os, long chunklength = 1024)
		: HTTPChunkedStreamBase(os, chunklength), ostream(rdbuf()) {}

	//! The chunked encoding is finished and the stream is closed.
	//! The wrapped ostream is not closed.
	~HTTPChunkedOStream() {
		close();
	}
};

#endif
