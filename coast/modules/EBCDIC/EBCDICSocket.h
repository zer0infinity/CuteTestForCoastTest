/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _EBCDICSocket_H
#define _EBCDICSocket_H

//---- baseclass include -------------------------------------------------
#include "config_ebcdic.h"
#include "Socket.h"

//--- EBCDICSocket ---
//!Socket to an EBCDIC speeking peer
//! Everything you read/write throu the stream of this Socket will be
//! converted from/to EBCDIC, what you do to the fd will *NOT* be converted!
class EXPORTDECL_EBCDIC EBCDICSocket : public Socket
{
	// opens a client side socket connection
	// and closes it in destructor
public:
	EBCDICSocket(int socket, const Anything &clientInfo, bool doClose = true, Allocator *a = Storage::Global());
	// use socket descriptor

protected:

	virtual std::iostream *DoMakeStream();

private:
	EBCDICSocket(); // must not use these
	EBCDICSocket(const EBCDICSocket &);
	EBCDICSocket &operator=(const EBCDICSocket &);
};

#endif
