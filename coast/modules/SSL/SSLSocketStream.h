/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _SSLSocketStream_H
#define _SSLSocketStream_H

//--- module used in the interface
#include "SocketStream.h"

typedef struct ssl_st SSL;

class SSLSocket;

class SSLSocketStreamBuf : public SocketStreamBuf
{
public:
	SSLSocketStreamBuf(SSL *ctx, SSLSocket *ssl, long timeout = 300 * 1000);
	~SSLSocketStreamBuf();

protected:					  // get area
	virtual long DoWrite(const char *buf, long len);
	virtual long DoRead(char *buf, long len) const;
	virtual void SetStreamState(long bytesProcessed) const;

private:
	// don't use these
	SSLSocketStreamBuf();
	SSLSocketStreamBuf(const SSLSocketStreamBuf &ssbuf);

	SSL *fContext;
}; // SocketStreamBuf

//
// adapts ios to a SSL Socket Stream buffer
//

class iosITOSSLSocket : virtual public std::ios, public coast::AllocatorNewDelete
{
public:
	iosITOSSLSocket(SSL *ctx, SSLSocket *ssl, long timeout = 300 * 1000);
	virtual ~iosITOSSLSocket() { }

	SSLSocketStreamBuf *rdbuf()  {
		return &fSSLSockBuf;
	}

protected:
	SSLSocketStreamBuf fSSLSockBuf;   // the buffer with its underlying string
	Allocator *fAllocator;

	// void autoflush()  { flags(flags() | ios::unitbuf); } don't ever use this sh... with sockets
private:
	iosITOSSLSocket();
	// init from ios is needed, because
	// ios() won't do the job;
	// (see comment in iostream.h)
}; // iosIFASocket

class ISSLSocketStream : public iosITOSSLSocket, public std::istream
{
public:
	ISSLSocketStream(SSL *ctx, SSLSocket *ssl, long timeout = 300 * 1000)
		: iosITOSSLSocket(ctx, ssl, timeout)
		, std::istream(&fSSLSockBuf)
	{ } // read from s

	~ISSLSocketStream() { }
}; // iSSLSocketStream

class OSSLSocketStream : public iosITOSSLSocket, public std::ostream
{
public:

	OSSLSocketStream(SSL *ctx, SSLSocket *ssl, long timeout = 300 * 1000)
		: iosITOSSLSocket(ctx, ssl, timeout)
		, std::ostream(&fSSLSockBuf)
	{  }

	~OSSLSocketStream() { }

private:
	OSSLSocketStream(); // block usage of default ctor

}; // OSSLSocketStream

class SSLSocketStream : public iosITOSSLSocket, public std::iostream
{
public:
	SSLSocketStream(SSL *ctx, SSLSocket *ssl, long timeout = 300 * 1000)
		: iosITOSSLSocket(ctx, ssl, timeout)
		, std::iostream(&fSSLSockBuf)
	{ }

	~SSLSocketStream() { }
private:
	SSLSocketStream(); // block the usage of the default ctor

}; // SSLSocketStream

#endif

