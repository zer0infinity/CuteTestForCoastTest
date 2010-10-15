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
#include "Context.h"

class ParameterMapper;

class EXPORTDECL_STDDATAACCESS ConnectorParams
{
public:
	ConnectorParams(Context &ctx, ParameterMapper *pMapper);

	String Name();
	bool UseSSL();
	String IPAddress();
	long Port();
	String PortAsString();
	long Timeout();
	bool UseThreadLocal();

private:
	Anything fParams;
	Context::PushPopEntry<Anything> fParamEntry;
	ConnectorParams();
	ConnectorParams(const ConnectorParams &);
	ConnectorParams &operator=(const ConnectorParams &);
};

#endif
