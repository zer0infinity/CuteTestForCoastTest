/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _EBCDICSocketStream_H
#define _EBCDICSocketStream_H

#include "config_ebcdic.h"
#include "EBCDICSocketStreamBuf.h"

//!provides the correct SocketStreamBuf for EBCDIC Sockets
class EXPORTDECL_EBCDIC iosItopiaEBCDICSocket : virtual public ios
{
public:
	iosItopiaEBCDICSocket(Socket *s, long timeout = 300 * 1000, long sockbufsz = cSocketStreamBufferSize )
		: ssbuf(s, timeout, sockbufsz) {
		init(&ssbuf);
	}
	// init from ios is needed, because
	// ios() won't do the job;
	// (see comment in iostream.h)
	virtual ~iosItopiaEBCDICSocket() { }

	EBCDICSocketStreamBuf *rdbuf()  {
		return &ssbuf;
	}

protected:
	EBCDICSocketStreamBuf ssbuf;   // the buffer with its underlying string

private:
	iosItopiaEBCDICSocket();
}; // iosItopiaEBCDICSocket

//---- EBCDICSocketStream ------------------------------------------------
//!text socket stream which translates data to/from EBCDIC
//! outbound data is converted from ASCII to EBCDIC,
//! inbound data is converted from EBCDIC to ASCII.
class  EXPORTDECL_EBCDIC EBCDICSocketStream : public iosItopiaEBCDICSocket, public iostream
{
public:
	EBCDICSocketStream(Socket *s, long timeout = 300 * 1000, long sockbufsz = cSocketStreamBufferSize);
	~EBCDICSocketStream();

private:
	EBCDICSocketStream(); // block the usage of the default ctor
}; // EBCDICSocketStreamBuf

#endif
