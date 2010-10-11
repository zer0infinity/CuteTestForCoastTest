/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

//--- interface include --------------------------------------------------------
#include "EBCDICSocket.h"

//--- standard modules used ----------------------------------------------------
#include "EBCDICSocketStreamBuf.h"
#include "EBCDICSocketStream.h"
#include "Dbg.h"

//--- c-library modules used ---------------------------------------------------

//---- EBCDICSocket ----------------------------------------------------------
EBCDICSocket::EBCDICSocket(int socket, const Anything &clientInfo, bool doClose, Allocator *a)
	: Socket(socket, clientInfo, doClose, 300 * 1000, a)
{
}

std::iostream *EBCDICSocket::DoMakeStream()
{
	StartTrace(EBCDICSocket.DoMakeStream);
	return new EBCDICSocketStream(this);
}
