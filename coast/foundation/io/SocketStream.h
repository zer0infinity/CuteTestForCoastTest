/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _SOCKETSTREAM_H
#define _SOCKETSTREAM_H

#include "Socket.h"
#include <cstdio>
#include <iostream>
#include <iomanip>

const int cSocketStreamBufferSize = 8024;

//! streambuf implementation for sockets
class SocketStreamBuf : public std::streambuf
{
public:
	//! constructor takes socket object and timeout
	/*! \param socket the socket this streambuf is writing to or reading from
		\param timeout the timeout for a read or write operation uses Socket->IsReady... Method
		\param sockbufsz initial internal read/write buffer size
		\param mode is the socket reading, writing or both, default: in/out */
	SocketStreamBuf(Socket *socket, long timeout = 300 * 1000, long sockbufsz = cSocketStreamBufferSize, int mode = std::ios::out | std::ios::in);
	SocketStreamBuf(const SocketStreamBuf &ssbuf);

	//!destructor flushes the buffer and empties put and get areas
	virtual ~SocketStreamBuf();

	//! flushes the buffer, it writes to the socket
	virtual int sync();

	void SetTimeout(long t) ;
	long GetTimeout() ;

	//!returns amount of bytes read from belonging iostream:
	virtual long GetReadCount() const {
		return fReadCount;
	}
	//!returns amount of bytes wrote from belonging iostream:
	virtual long GetWriteCount() const {
		return fWriteCount;
	}

	//! canonical output operator for SocketStreamBufs
	friend std::ostream &operator<<(std::ostream &os, SocketStreamBuf *ssbuf);

protected: // seekxxx are protected in the std..
	typedef std::streambuf::pos_type pos_type;
	typedef std::streambuf::off_type off_type;
	typedef std::ios::seekdir seekdir;
	typedef std::ios::openmode openmode;

	//! standard iostream behavior, adjust put or get position absolutely
	virtual pos_type seekpos(pos_type pos, openmode mode = std::ios::in | std::ios::out);
	//! standard iostream behavior, adjust put or get position relatively
	virtual pos_type seekoff(off_type off, seekdir dir, openmode mode = std::ios::in | std::ios::out);

	//! no buffer setting needed, because we carry our own buffer, a String object
	std::streambuf *setbuf(char *buf, int length) {
		return this;
	}

	//! consumes chars of the put area
	virtual int overflow(int c = EOF);

	//! produces characters for the get area
	virtual int underflow();

	//! writes pending bytes to the socket guarded by fTimeout
	//! \param buf the buffer to write
	//! \param len the maximum length of the buffer
	virtual long DoWrite(const char *buf, long len);

	//! reads pending bytes from the socket guarded by fTimeout
	//! \param buf the buffer to read bytes in
	//! \param len the maximum length of the buffer
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

	//! the socket this streambuf uses
	Socket *fSocket;

	//!stores here the amount of bytes read for LAST reply:
	void AddReadCount( long bytes );
	//!stores here the amount of bytes written from LAST request:
	void AddWriteCount( long bytes );

	//! statistic variable
	long fReadCount;
	long fWriteCount;
};

//! adapts ios to a Socket Stream buffer
class iosITOSocket : virtual public std::ios, public Coast::AllocatorNewDelete
{
public:
	iosITOSocket(Socket *s, long timeout = 300 * 1000, long sockbufsz = cSocketStreamBufferSize, int mode = std::ios::in | std::ios::out );
	// s is the source resp. the sink;

	SocketStreamBuf *rdbuf()  {//lint !e1511
		return &fSocketBuf;
	}

protected:
	iosITOSocket();// do not implement { }
	SocketStreamBuf fSocketBuf;   // the buffer with its underlying string

	//!allocator for dynamic allocations
	Allocator *fAllocator;
	// void autoflush()  { flags(flags() | ios::unitbuf); } don't ever use this sh... with sockets
}; // iosITOSocket

//! istream for sockets
class ISocketStream : public iosITOSocket, public std::istream
{
public:
	//! constructor creates iosITOSocket
	/*! \param s the socket for the istream
		\param timeout the timeout for read operations
		\param sockbufsz initial internal read/write buffer size */
	ISocketStream(Socket *s, long timeout = 300 * 1000, long sockbufsz = cSocketStreamBufferSize)
		: iosITOSocket(s, timeout, sockbufsz, std::ios::in)
		, std::istream(&fSocketBuf)
	{ } // read from s

	~ISocketStream() { }

private:
	ISocketStream();
	ISocketStream(const ISocketStream &);
	ISocketStream &operator=(const ISocketStream &);
}; // ISocketStream

//! ostream for sockets
class OSocketStream : public iosITOSocket, public std::ostream
{
public:
	//! constructor creates iosITOSocket
	/*! \param s the socket for the ostream
		\param timeout the timeout for write operations
		\param sockbufsz initial internal read/write buffer size */
	OSocketStream(Socket *s, long timeout = 300 * 1000, long sockbufsz = cSocketStreamBufferSize)
		: iosITOSocket(s, timeout, sockbufsz, std::ios::out)
		, std::ostream(&fSocketBuf)
	{  }

	//! destructor does nothing
	~OSocketStream() { }

private:
	OSocketStream();
	OSocketStream(const OSocketStream &);
	OSocketStream &operator=(const OSocketStream &);
}; // OSocketStream

//! iostream for sockets
class SocketStream : public iosITOSocket, public std::iostream
{
public:
	//! constructor creates iosITOSocket
	/*! \param s the socket for the ostream
		\param timeout the timeout for read/write operations
		\param sockbufsz initial internal read/write buffer size */
	SocketStream(Socket *s, long timeout = 300 * 1000, long sockbufsz = cSocketStreamBufferSize);

	//! destructor does nothing
	~SocketStream() {}

private:
	SocketStream();
	SocketStream(const SocketStream &);
	SocketStream &operator=(const SocketStream &);
}; // SocketStream

//! modifies the timeout used by a SocketStream for a scope
class TimeoutModifier
{
public:
	//!modifies timeout of the SocketStream with timeout; it stores away the original value
	TimeoutModifier(SocketStream *ios, long timeout);
	//!restores the original timeout of the socket stream
	~TimeoutModifier();

	void Use() {};
protected:
	//!store of original timeout
	long fOriginalTimeout;
	//!the stream which is modified
	SocketStream *fStream;
};

#endif
