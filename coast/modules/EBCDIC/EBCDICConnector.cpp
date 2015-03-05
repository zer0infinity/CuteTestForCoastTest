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
#include "EBCDICConnector.h"

//--- EBCDICConnector --------------------------------------------
EBCDICConnector::EBCDICConnector(const char *ipAdr, long port, long connectTimeout, const char
								 *srcIpAdr, long srcPort) :
	Connector(ipAdr, port, connectTimeout, srcIpAdr, srcPort)
{
	StartTrace(EBCDICConnector.Ctor);
}

EBCDICConnector::~EBCDICConnector()
{
	StartTrace(EBCDICConnector.Dtor);
}

Socket *EBCDICConnector::DoMakeSocket(int socket, Anything &clientInfo, bool doClose)
{
	StartTrace(EBCDICConnector.DoMakeSocket);
	Allocator *a = GetSocketAllocator();
	return new (a) EBCDICSocket(socket, clientInfo, doClose, a);
}
