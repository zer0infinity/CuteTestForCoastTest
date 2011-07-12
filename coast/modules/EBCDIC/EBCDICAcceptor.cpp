/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#include "Anything.h"
#include "EBCDICSocket.h"
#include "Tracer.h"
#include "EBCDICAcceptor.h"

//---- EBCDICAcceptor ----------------------------------------------------------
EBCDICAcceptor::EBCDICAcceptor(const char *ipadress, long port,
							   long backlog, AcceptorCallBack *cb):
	Acceptor(ipadress, port, backlog, cb)
{
	StartTrace(EBCDICAcceptor.Ctor);
}

EBCDICAcceptor::~EBCDICAcceptor()
{
	StartTrace(EBCDICAcceptor.Dtor);
}

Socket *EBCDICAcceptor::DoMakeSocket(int socket, Anything &clientInfo, bool doClose)
{
	StartTrace(EBCDICAcceptor.DoMakeSocket);
	Allocator *a = GetSocketAllocator();
	return new (a) EBCDICSocket(socket, clientInfo, doClose, a);
}
