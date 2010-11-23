/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _EBCDICAcceptor_H
#define _EBCDICAcceptor_H

//---- baseclass include -------------------------------------------------
#include "Socket.h"

//---- EBCDICAcceptor ----------------------------------------------------------
//!accepts a connection from an EBCDIC peer and creates an appropriate socket
class EBCDICAcceptor : public Acceptor
{
	// This class handles an accept loop
	// in its own thread
public:
	EBCDICAcceptor(const char *ipadress, long port, long backlog,
				   AcceptorCallBack *cb);
	~EBCDICAcceptor();

protected:
	virtual Socket *DoMakeSocket(int socket, Anything &clientInfo, bool doClose = true);

private:
	// don't use these (they are not implemented anyway ...)
	EBCDICAcceptor();
	EBCDICAcceptor(const EBCDICAcceptor &);
	EBCDICAcceptor &operator=(const EBCDICAcceptor &);
};

#endif
