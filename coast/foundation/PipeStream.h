/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _PipeStream_H
#define _PipeStream_H

#include "config_foundation.h"
#include "StringStream.h"

class Pipe;

const int cPipeStreamBufferSize = 8024;
//---- PipeStreamBuf -------------------------------------------------------------------
//! streambuf implementation for sockets
class EXPORTDECL_FOUNDATION PipeStreamBuf : public streambuf
{
public:
	/*! constructor takes socket object and timeout
		\param pipe the pipe this streambuf is writing to and/or reading from
		\param timeout the timeout for a read or write operation uses Pipe->IsReady... Method
		\param mode is the pipe reading, writing or both, default: in/out */
	PipeStreamBuf(Pipe *pipe, long timeout = 500, long sockbufsz = cPipeStreamBufferSize, int mode = ios::out | ios::in);
	PipeStreamBuf(const PipeStreamBuf &ssbuf);

	//! destructor flushes the buffer and empties put and get areas
	virtual ~PipeStreamBuf();

	//! flushes the buffer, it writes to the socket
	virtual int sync();

	long SetTimeout(long t) {
		long lOldTimeout = fTimeout;
		fTimeout = t;
		return lOldTimeout;
	}
	long GetTimeout() {
		return fTimeout;
	}

	//! canonical output operator for PipeStreamBufs
	friend EXPORTDECL_FOUNDATION ostream &operator<<(ostream &os, PipeStreamBuf *ssbuf);
#if defined(ONLY_STD_IOSTREAM)
protected: // seekxxx are protected in the std..
	typedef std::streambuf::pos_type	pos_type;
	typedef std::streambuf::off_type	off_type;
	typedef std::ios::seekdir	seekdir;
	typedef std::ios::openmode	openmode;
#else
	typedef streampos pos_type;
	typedef	streamoff off_type;
	typedef int 	openmode;
	typedef ios::seek_dir seekdir;
#endif

	//! standard iostream behavior, adjust put or get position absolutely
	virtual pos_type seekpos(pos_type pos, openmode mode = ios::in | ios::out);
	//! standard iostream behavior, adjust put or get position relatively
	virtual pos_type seekoff(off_type off, seekdir dir, openmode mode = ios::in | ios::out);

protected:
	PipeStreamBuf() { }
	/*! no buffer setting needed, because we carry our own buffer, a String object */
	streambuf *setbuf(char *, int) {
		return this;
	}

	//! consumes chars of the put area
	virtual int overflow(int c = EOF);

	//! produces characters for the get area
	virtual int underflow();

	/*! writes pending bytes to the pipe guarded by fTimeout
		\param buf the buffer to write
		\param len the maximum length of the buffer
		\return number of written characters */
	virtual long DoWrite(const char *buf, long len);

	/*! reads pending bytes from the socket guarded by fTimeout
		\param buf the buffer to read bytes in
		\param len the maximum length of the buffer
		\return number of characters read */
	virtual long DoRead(char *buf, long len) const;

	//! defines the holding area for the streambuf
	void xinit();

	//! auxiliary functions for interfacing to setp and setg, getting rid of
	//! old streambuf style setb() and base()
	char *startr() {
		if (fReadBufStorage.Capacity() > 0) {
			return (char *)(const char *)fReadBufStorage;
		} else {
			return 0;
		}
	}
	char *endr() {
		return startr() + fReadBufStorage.Capacity();
	}
	char *startw() {
		if (fWriteBufStorage.Capacity() > 0) {
			return (char *)(const char *)fWriteBufStorage;
		} else {
			return 0;
		}
	}
	char *endw() {
		return startw() + fWriteBufStorage.Capacity();
	}

	//! the storage of the holding area buffer
	String fReadBufStorage;
	String fWriteBufStorage;

	//! the pipe this streambuf uses
	Pipe *fPipe;

	//! the timeout for read and write operations
	long fTimeout;

	//! statistic variable
	long fReadCount;

	//! statistic variable
	long fWriteCount;
};

//---- iosItopiaPipe -------------------------------------------------------------------
//! adapts ios to a Pipe Stream buffer
class EXPORTDECL_FOUNDATION iosItopiaPipe : virtual public ios
{
public:
	iosItopiaPipe(Pipe *s, long timeout = 500, long sockbufsz = cPipeStreamBufferSize, int mode = ios::in | ios::out )
		: fPipeBuf(s, timeout, sockbufsz, mode) {
		init(&fPipeBuf);
	}

	virtual ~iosItopiaPipe() { }

	PipeStreamBuf *rdbuf()  {
		return &fPipeBuf;
	}

protected:
	iosItopiaPipe();
	//! the buffer with its underlying string
	PipeStreamBuf fPipeBuf;
	// void autoflush()  { flags(flags() | ios::unitbuf); } don't ever use this sh... with sockets
};

//---- IPipeStream -------------------------------------------------------------------
//! istream for sockets
class  EXPORTDECL_FOUNDATION IPipeStream : public iosItopiaPipe, public istream
{
public:
	/*! constructor creates iosItopiaPipe
		\param s the socket for the istream
		\param timeout the timeout for read operations */
	IPipeStream(Pipe *p, long timeout = 500, long bufsz = cPipeStreamBufferSize)
		: iosItopiaPipe(p, timeout, bufsz, ios::in)
#if defined(ONLY_STD_IOSTREAM)
		, istream(&fPipeBuf)
#endif
	{}

	~IPipeStream() { }

private:
	IPipeStream();
	IPipeStream(const IPipeStream &);
	IPipeStream &operator=(const IPipeStream &);
};

//---- OPipeStream -------------------------------------------------------------------
//! ostream for sockets
class  EXPORTDECL_FOUNDATION OPipeStream : public iosItopiaPipe, public ostream
{
public:
	/*! constructor creates iosItopiaPipe
		\param s the socket for the ostream
		\param timeout the timeout for write operations */
	OPipeStream(Pipe *s, long timeout = 500, long bufsz = cPipeStreamBufferSize)
		: iosItopiaPipe(s, timeout, bufsz, ios::out)
#if defined(ONLY_STD_IOSTREAM)
		, ostream(&fPipeBuf)
#endif
	{}

	//! destructor does nothing
	~OPipeStream() { }

private:
	OPipeStream();
	OPipeStream(const OPipeStream &);
	OPipeStream &operator=(const OPipeStream &);
};

//---- PipeStream -------------------------------------------------------------------
//! iostream for sockets
class  EXPORTDECL_FOUNDATION PipeStream : public iosItopiaPipe, public iostream
{
public:
	/*! constructor creates iosItopiaPipe
		\param s the socket for the iostream
		\param timeout the timeout for read/write operations
		\param buffer size of the underlying PipeStreamBuf */
	PipeStream(Pipe *s, long timeout = 500, long sockbufsz = cPipeStreamBufferSize);

	//! destructor does nothing
	~PipeStream() { }

private:
	PipeStream();
	PipeStream(const PipeStream &);
	PipeStream &operator=(const PipeStream &);
};

//---- PipeTimeoutModifier -------------------------------------------------------------------
//! temporarily changes the timeout used by a PipeStream
class  EXPORTDECL_FOUNDATION PipeTimeoutModifier
{
public:
	/*! constructor modifies the timeout parameter of the given PipeStream object
		\param ios pointer to the PipeStream to modify
		\param timeout the timeout for read/write operations */
	PipeTimeoutModifier(PipeStream *ios, long timeout);
	~PipeTimeoutModifier();

	void Use() {};

protected:
	long fOriginalTimeout;
	PipeStream *fStream;
};

#endif
