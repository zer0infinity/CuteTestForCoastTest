/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#include "EBCDICSocketStreamBuf.h"
#include "a2ee2a.h"

//---- EBCDICSocketStreamBuf ---------------------------------------------------
EBCDICSocketStreamBuf::EBCDICSocketStreamBuf(Socket *socket, long timeout, long sockbufsz)
	: SocketStreamBuf(socket, timeout, sockbufsz)
{
}

long EBCDICSocketStreamBuf::DoWrite(const char *buf, long len)
{
	char *ebcdicBuf = new char[len];

	ascii2ebcdic(ebcdicBuf, buf, len);
	// do I have to copy here?

	long bytesWritten = SocketStreamBuf::DoWrite(ebcdicBuf, len);
	delete ebcdicBuf;

	return bytesWritten;
}

long EBCDICSocketStreamBuf::DoRead(char *buf, long len) const
{
	long bytesRead = SocketStreamBuf::DoRead(buf, len);
	ebcdic2ascii(buf, buf, bytesRead);
	return bytesRead;

}
