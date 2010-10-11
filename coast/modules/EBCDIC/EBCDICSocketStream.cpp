/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

//--- interface include --------------------------------------------------------
#include "EBCDICSocketStream.h"

//--- standard modules used ----------------------------------------------------
#include "Socket.h"

//--- c-library modules used ---------------------------------------------------

//---- EBCDICSocketStream ------------------------------------------------------
EBCDICSocketStream::EBCDICSocketStream(Socket *s, long timeout, long sockbufsz)
	: iosCoastEBCDICSocket(s, timeout, sockbufsz)
	, std::iostream(rdbuf())
{ }

EBCDICSocketStream::~EBCDICSocketStream()
{
}
