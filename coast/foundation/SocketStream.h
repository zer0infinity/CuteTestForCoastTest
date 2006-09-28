/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _SOCKETSTREAM_H
#define _SOCKETSTREAM_H

//--- module used in the interface
#include "config_foundation.h"	// for definition of EXPORTDECL_FOUNDATION
#include "StringStream.h"
#include "Socket.h"

const int cSocketStreamBufferSize = 8024;

//---- SocketStreamBuf -------------------------------------------------------------------
//! streambuf implementation for sockets
class EXPORTDECL_FOUNDATION SocketStreamBuf : public streambuf
{
public:
	//!constructor takes socket object and timeout
	//! \param socket the socket this streambuf is writing to or reading from
	//! \param timeout the timeout for a read or write operation uses Socket->IsReady... Method
	//! \param mode is the socket reading, writing or both, default: in/out
	SocketStreamBuf(Socket *socket, long timeout = 300 * 1000, long sockbufsz = cSocketStreamBufferSize, int mode = ios::out | ios::in);
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
	friend EXPORTDECL_FOUNDATION ostream &operator<<(ostream &os, SocketStreamBuf *ssbuf);
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
	SocketStreamBuf() { }
	//! no buffer setting needed, because we carry our own buffer, a String object
	streambuf *setbuf(char *buf, int length) {
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

//---- iosITOSocket -------------------------------------------------------------------
//! adapts ios to a Socket Stream buffer
class EXPORTDECL_FOUNDATION iosITOSocket : virtual public ios
{
public:
	iosITOSocket(Socket *s, long timeout = 300 * 1000, long sockbufsz = cSocketStreamBufferSize, int mode = ios::in | ios::out );
	// s is the source resp. the sink;

	virtual ~iosITOSocket() { }
	//!allocate object with optimized allocator strategy
	static void *operator new(size_t size, Allocator *a);
	//!delete object with optimized allocator strategy
	static void operator delete(void *d);
#if defined(WIN32) && (_MSC_VER >= 1200) // VC6 or greater
	static void operator delete(void *d, Allocator *a);
#endif
	SocketStreamBuf *rdbuf()  {
		return &fSocketBuf;
	}

protected:
	iosITOSocket();// do not implement { }
	SocketStreamBuf fSocketBuf;   // the buffer with its underlying string

	//!allocator for dynamic allocations
	Allocator *fAllocator;
	// void autoflush()  { flags(flags() | ios::unitbuf); } don't ever use this sh... with sockets
}; // iosITOSocket

//---- ISocketStream -------------------------------------------------------------------
//! istream for sockets
class  EXPORTDECL_FOUNDATION ISocketStream : public iosITOSocket, public istream
{
public:
	//! constructor creates iosITOSocket
	//! \param s the socket for the istream
	//! \param timeout the timeout for read operations
	ISocketStream(Socket *s, long timeout = 300 * 1000, long sockbufsz = cSocketStreamBufferSize)
		: iosITOSocket(s, timeout, sockbufsz, ios::in)
#if defined(ONLY_STD_IOSTREAM)
		, istream(&fSocketBuf)
#endif
	{ } // read from s

	~ISocketStream() { }

private:
	ISocketStream();
	ISocketStream(const ISocketStream &);
	ISocketStream &operator=(const ISocketStream &);
}; // ISocketStream

//---- ISocketStream -------------------------------------------------------------------
//! ostream for sockets
class  EXPORTDECL_FOUNDATION OSocketStream : public iosITOSocket, public ostream
{
public:
	//! constructor creates iosITOSocket
	//! \param s the socket for the ostream
	//! \param timeout the timeout for write operations
	OSocketStream(Socket *s, long timeout = 300 * 1000, long sockbufsz = cSocketStreamBufferSize)
		: iosITOSocket(s, timeout, sockbufsz, ios::out)
#if defined(ONLY_STD_IOSTREAM)
		, ostream(&fSocketBuf)
#endif
	{  }

	//! destructor does nothing
	~OSocketStream() { }

private:
	OSocketStream();
	OSocketStream(const OSocketStream &);
	OSocketStream &operator=(const OSocketStream &);
}; // OSocketStream

//---- SocketStream -------------------------------------------------------------------
//! iostream for sockets
class  EXPORTDECL_FOUNDATION SocketStream : public iosITOSocket, public iostream
{
public:
	//! constructor creates iosITOSocket
	//! \param s the socket for the ostream
	//! \param timeout the timeout for read/write operations
	SocketStream(Socket *s, long timeout = 300 * 1000, long sockbufsz = cSocketStreamBufferSize);

	//! destructor does nothing
	~SocketStream() {}

private:
	SocketStream();
	SocketStream(const SocketStream &);
	SocketStream &operator=(const SocketStream &);
}; // SocketStream

//---- TimeoutModifier -------------------------------------------------------------------
//! modifies the timeout used by a SocketStream for a scope
class  EXPORTDECL_FOUNDATION TimeoutModifier
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
