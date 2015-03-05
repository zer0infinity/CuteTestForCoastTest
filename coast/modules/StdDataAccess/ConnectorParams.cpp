/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#include "ConnectorParams.h"
#include "Mapper.h"

//---- ConnectorParams ----------------------------------------------
ConnectorParams::ConnectorParams(Context &ctx, ParameterMapper *pMapper) : fParams(Anything::ArrayMarker()), fParamEntry(ctx, "ConnectorParams", fParams, "Backend") {
	fParams["Name"] = pMapper->Get("Name", ctx).AsString();
	fParams["Server"] = pMapper->Get("Server", ctx).AsString("127.0.0.1");
	fParams["Port"] = pMapper->Get("Port", ctx).AsLong(80L);
	fParams["UseSSL"] = (pMapper->Get("UseSSL", ctx).AsLong(0L) != 0L);
	fParams["Timeout"] = (pMapper->Get("Timeout", ctx).AsLong(0L) * 1000L);
	fParams["UseThreadLocalMemory"] = (pMapper->Get("UseThreadLocalMemory", ctx).AsLong(0L) != 0L);
}

String ConnectorParams::Name() {
	return fParams["Name"].AsString();
}

bool ConnectorParams::UseSSL() {
	return fParams["UseSSL"].AsBool();
}

String ConnectorParams::IPAddress() {
	return fParams["Server"].AsString();
}

long ConnectorParams::Port() {
	return fParams["Port"].AsLong(80L);
}

String ConnectorParams::PortAsString() {
	return fParams["Port"].AsString();
}

long ConnectorParams::Timeout() {
	return fParams["Timeout"].AsLong(80L);
}

bool ConnectorParams::UseThreadLocal() {
	return fParams["UseThreadLocalMemory"].AsBool();
}
