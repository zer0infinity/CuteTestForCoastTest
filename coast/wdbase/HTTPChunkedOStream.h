/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _HTTPCHUNKEDOSTREAM_H
#define _HTTPCHUNKEDOSTREAM_H

#include "StringStream.h"

//---- HTTPChunkedStreamBuf --------------------------------------------------------------

//! Streambuffer for HTTPChunkedOStream.
class HTTPChunkedStreamBuf : public streambuf
{
public:
	//! \param os wrapped output stream
	//! \param chunklength max length of a single chunk
	//! \param a memory allocator
	HTTPChunkedStreamBuf(std::ostream &os, long chunklength, Allocator *a = Coast::Storage::Current());

	virtual ~HTTPChunkedStreamBuf();

	//! Writes the current content of the buffer chunk encoded to the wrapped ostream.
	virtual int sync();

	//! Writes the rest of the buffer and the termination chunk to the wrapped ostream.
	//! After close() the ostream can not be used anymore.
	void close();

protected: // seekxxx are protected in the std..
	typedef std::streambuf::pos_type pos_type;
	typedef std::streambuf::off_type off_type;
	typedef std::ios::seekdir seekdir;
	typedef std::ios::openmode openmode;

	//! flushes the buffer and sets streampos to 0 it is not possible to seek on a socket
	virtual pos_type seekpos(pos_type, openmode mode = std::ios::in | std::ios::out);

	//! flushes the buffer and sets streampos to 0 it is not possible to seek on a socket
	virtual pos_type seekoff(off_type, seekdir, openmode mode = std::ios::in | std::ios::out);

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
	std::ostream *fOs;

private:
	//! size of the internal buffer, is equal to the chunk size
	long fBufSize;
};

//---- HTTPChunkedStreamBase --------------------------------------------------------------

//! Base class for HTTPChunkedStream classes (see RFC 2068 sec 3.6).
class HTTPChunkedStreamBase : virtual public std::ios
{
public:
	//! \param os wrapped output stream
	//! \param chunklength max length of a single chunk
	//! \param mode ios::out or ios::in
	HTTPChunkedStreamBase(std::ostream &os, long chunklength, int mode = std::ios::out)
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
class HTTPChunkedOStream : public HTTPChunkedStreamBase, public std::ostream
{
public:
	//! \param os wrapped output stream
	//! \param chunklength max length of a single chunk (deafault 1024)
	HTTPChunkedOStream(std::ostream &os, long chunklength = 1024)
		: HTTPChunkedStreamBase(os, chunklength), std::ostream(rdbuf()) {}

	//! The chunked encoding is finished and the stream is closed.
	//! The wrapped ostream is not closed.
	virtual ~HTTPChunkedOStream() {
		close();
	}
};

#endif
