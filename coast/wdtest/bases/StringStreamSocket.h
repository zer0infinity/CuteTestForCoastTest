/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _StringStreamSocket_H
#define _StringStreamSocket_H

#include "Socket.h"
#include "StringStream.h"

//---- StringStreamSocket ----------------------------------------------------------
//!utility lightweight mock up socket for writing test cases easily
class StringStreamSocket : public Socket
{
public:
	//--- constructors
	//!param: str - supplies the data for reading and collects from writing
	StringStreamSocket(String &str);

protected:
	bool IsReady(bool forreading);
	//deprecated:
	bool IsReady(long fd, short event, long timeout, long &retCode);
};

#endif
