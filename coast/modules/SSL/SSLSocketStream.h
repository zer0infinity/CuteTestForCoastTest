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
#include "config_ssl.h"
#include "SocketStream.h"

typedef struct ssl_st SSL;

class SSLSocket;

class EXPORTDECL_SSL SSLSocketStreamBuf : public SocketStreamBuf
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

class EXPORTDECL_SSL iosITOSSLSocket : virtual public ios
{
public:
	iosITOSSLSocket(SSL *ctx, SSLSocket *ssl, long timeout = 300 * 1000);
	virtual ~iosITOSSLSocket() { }

	SSLSocketStreamBuf *rdbuf()  {
		return &fSSLSockBuf;
	}

	//!allocate object with optimized allocator strategy
	static void *operator new(size_t size, Allocator *a);
	//!delete object with optimized allocator strategy
	static void operator delete(void *d);
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

class  EXPORTDECL_SSL ISSLSocketStream : public iosITOSSLSocket, public istream
{
public:
	ISSLSocketStream(SSL *ctx, SSLSocket *ssl, long timeout = 300 * 1000)
		: iosITOSSLSocket(ctx, ssl, timeout)
#if defined(ONLY_STD_IOSTREAM)
		, istream(&fSSLSockBuf)
#endif
	{ } // read from s

	~ISSLSocketStream() { }
}; // iSSLSocketStream

class  EXPORTDECL_SSL OSSLSocketStream : public iosITOSSLSocket, public ostream
{
public:

	OSSLSocketStream(SSL *ctx, SSLSocket *ssl, long timeout = 300 * 1000)
		: iosITOSSLSocket(ctx, ssl, timeout)
#if defined(ONLY_STD_IOSTREAM)
		, ostream(&fSSLSockBuf)
#endif
	{  }

	~OSSLSocketStream() { }

private:
	OSSLSocketStream(); // block usage of default ctor

}; // OSSLSocketStream

class  EXPORTDECL_SSL SSLSocketStream : public iosITOSSLSocket, public iostream
{
public:
	SSLSocketStream(SSL *ctx, SSLSocket *ssl, long timeout = 300 * 1000)
		: iosITOSSLSocket(ctx, ssl, timeout)
#if defined(ONLY_STD_IOSTREAM)
		, iostream(&fSSLSockBuf)
#endif
	{ }

	~SSLSocketStream() { }
private:
	SSLSocketStream(); // block the usage of the default ctor

}; // SSLSocketStream

#endif

