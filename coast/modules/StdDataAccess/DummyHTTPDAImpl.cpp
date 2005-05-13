/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

//--- standard modules used ----------------------------------------------------
#include "Anything.h"
#include "ConnectorParams.h" // for ConnectorParams
#include "Mapper.h"
#include "Context.h"
#include "Dbg.h"

//--- interface include --------------------------------------------------------
#include "DummyHTTPDAImpl.h"

//--- DummyHTTPDAImpl -----------------------------------------------------
RegisterDataAccessImpl(DummyHTTPDAImpl);

DummyHTTPDAImpl::DummyHTTPDAImpl(const char *name) : DummyDAImpl(name)
{
}

DummyHTTPDAImpl::~DummyHTTPDAImpl()
{
}

IFAObject *DummyHTTPDAImpl::Clone() const
{
	return new DummyHTTPDAImpl(fName);
}

bool DummyHTTPDAImpl::Exec( Context &context, ParameterMapper *in, ResultMapper *out)
{
	StartTrace(DummyHTTPDAImpl.Exec);

	ConnectorParams cps(this, context);
	return DoExec(context, in, out);
}
