/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#include "StringStreamSocket.h"

//---- StringStreamSocket ----------------------------------------------------------------
StringStreamSocket::StringStreamSocket(String &str): Socket(-1)
{
	fStream = new StringStream(str);
	fClientInfo["REMOTE_ADDR"] = "localhost";
	fClientInfo["HTTPS"] = false;
}

bool StringStreamSocket::IsReady(bool forreading)
{
	return true;
}

bool StringStreamSocket::IsReady(long fd, short event, long timeout, long &retCode)
{
	return true;
}
