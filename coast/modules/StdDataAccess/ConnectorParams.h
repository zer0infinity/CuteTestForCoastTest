/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _ConnectorParams_H
#define _ConnectorParams_H

//--- superclass interface ---
#include "config_stddataaccess.h"

class DataAccessImpl;
class String;
class Context;

class EXPORTDECL_STDDATAACCESS ConnectorParams
{
public:
	ConnectorParams(DataAccessImpl *master, Context &ctx);
	virtual ~ConnectorParams();

	virtual bool UseSSL();
	virtual String IPAddress();
	virtual long Port();
	virtual String PortAsString();
	virtual long Timeout();
	virtual bool UseThreadLocal();

protected:
	DataAccessImpl *fMaster;
	Context &fContext;

private:
	ConnectorParams();
	ConnectorParams(const ConnectorParams &);
	ConnectorParams &operator=(const ConnectorParams &);
};

#endif		//not defined _ConnectorParams_H
