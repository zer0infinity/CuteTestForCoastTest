/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

//--- standard modules used ----------------------------------------------------
#include "Anything.h"
#include "Context.h"
#include "DataAccessImpl.h"
#include "Dbg.h"

//--- interface include --------------------------------------------------------
#include "ConnectorParams.h"

//---- ConnectorParams ----------------------------------------------
ConnectorParams::ConnectorParams(DataAccessImpl *master, Context &ctx) : fMaster(master), fContext(ctx)
{
	// store away the servers ip address or dns name for later use
	Anything tmpStore(fContext.GetTmpStore());
	tmpStore["Backend"]["Server"] = IPAddress();
	tmpStore["Backend"]["Port"] = Port();
	tmpStore["Backend"]["UseSSL"] = UseSSL();
}

ConnectorParams::~ConnectorParams()
{
}

bool ConnectorParams::UseSSL()
{
	return (fContext.Lookup("CurrentServer.UseSSL", fMaster->Lookup("UseSSL", 0L)) != 0);
}

String ConnectorParams::IPAddress()
{
	return fContext.Lookup("CurrentServer.Server", fMaster->Lookup("Server", "127.0.0.1"));
}

long ConnectorParams::Port()
{
	return fContext.Lookup("CurrentServer.Port", fMaster->Lookup("Port", 80L));
}

String ConnectorParams::PortAsString()
{
	String work;
	return work.Append(fContext.Lookup("CurrentServer.Port", fMaster->Lookup("Port", 80L)));
}

long ConnectorParams::Timeout()
{
	return fContext.Lookup("CurrentServer.Timeout", fMaster->Lookup("Timeout", 0L)) * 1000; // since timeout is in milliseconds
}

bool ConnectorParams::UseThreadLocal()
{
	return fContext.Lookup("CurrentServer.UseThreadLocalMemory", fMaster->Lookup("UseThreadLocalMemory", 0L)); // since timeout is in milliseconds
}
