/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _EBCDICSocketStreamBuf_H
#define _EBCDICSocketStreamBuf_H

//---- baseclass include -------------------------------------------------
#include "SocketStream.h"

//---- EBCDICSocketStreamBuf ---------------------------------------------
//!Translates data to and from EBCDIC
//! data received is converted from EBCDIC to ASCII,
//! data sent is converted from ASCII to EBCDIC
//! Caution: *ALL* data is converted. Do *NOT* send binary data accross
//! an EBCDICSocketStreamBuf (or characters that cannot be translated)
class EBCDICSocketStreamBuf : public SocketStreamBuf
{
public:
	EBCDICSocketStreamBuf(Socket *socket, long timeout = 300 * 1000, long sockbufsz = cSocketStreamBufferSize);

protected:
	//! translate buf from ASCII to EBCDIC before writing to the socket
	//! \param buf the buffer to read bytes in
	//! \param len the maximum length of the buffer
	//! \pre buf is not 0
	virtual long DoWrite(const char *buf, long len);

	//!translate buf from EBCDIC to ASCII after reading from the socket
	//! \param buf the buffer to write
	//! \param len the maximum length of the buffer
	//! \pre buf is not 0
	virtual long DoRead(char *buf, long len) const;

private:
	// don't use these
	EBCDICSocketStreamBuf();
	EBCDICSocketStreamBuf(const EBCDICSocketStreamBuf &ssbuf);
}; // EBCDICSocketStreamBuf

#endif
