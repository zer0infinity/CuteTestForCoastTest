/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _EBCDICConnector_H
#define _EBCDICConnector_H

//---- baseclass include -------------------------------------------------
#include "config_ebcdic.h"
#include "Socket.h"

//---- EBCDICConnector ---------------------------------------------------
//!opens a socket connection to a server who understands and speaks EBCDIC text

//--- EBCDICConnector --------------------------------------------
class EXPORTDECL_EBCDIC EBCDICConnector : public Connector
{
	// this class takes an active end point specification
	// of a socket and connects to a server on the other
	// side, creating a socket connection that's read/writeable
public:
	EBCDICConnector(const char *ipAdr, long port, long connectTimeout = 0, const char *srcIpAdr = 0, long srcPort = 0);
	virtual ~EBCDICConnector();

protected:
	virtual Socket *DoMakeSocket(int socket, Anything &clientInfo, bool doClose = true);

private:
	// don't use these
	EBCDICConnector();
	EBCDICConnector(const EBCDICConnector &);
	EBCDICConnector &operator=(const EBCDICConnector &);

};

#endif
