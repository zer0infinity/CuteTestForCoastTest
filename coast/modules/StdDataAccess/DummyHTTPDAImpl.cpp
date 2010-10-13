/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */
//--- interface include --------------------------------------------------------
#include "DummyHTTPDAImpl.h"
//--- standard modules used ----------------------------------------------------
#include "ConnectorParams.h" // for ConnectorParams
#include "Mapper.h"
#include "Context.h"
#include "Dbg.h"
//--- DummyHTTPDAImpl -----------------------------------------------------
RegisterDataAccessImpl(DummyHTTPDAImpl);

DummyHTTPDAImpl::DummyHTTPDAImpl(const char *name) :
	DummyDAImpl(name) {
}

DummyHTTPDAImpl::~DummyHTTPDAImpl() {
}

IFAObject *DummyHTTPDAImpl::Clone(Allocator *a) const {
	return new (a) DummyHTTPDAImpl(fName);
}

bool DummyHTTPDAImpl::Exec(Context &context, ParameterMapper *in, ResultMapper *out) {
	StartTrace(DummyHTTPDAImpl.Exec);
	ConnectorParams cps(context, in);
	return DoExec(context, in, out);
}
