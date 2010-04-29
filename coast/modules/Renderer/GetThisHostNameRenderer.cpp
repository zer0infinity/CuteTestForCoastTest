/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

//--- interface include --------------------------------------------------------
#include "GetThisHostNameRenderer.h"

//--- project modules used -----------------------------------------------------

//--- standard modules used ----------------------------------------------------
#include "System.h"
#include "Dbg.h"
#include "Resolver.h"

//--- c-modules used -----------------------------------------------------------

//---- GetThisHostNameRenderer ---------------------------------------------------------------
RegisterRenderer(GetThisHostNameRenderer);

GetThisHostNameRenderer::GetThisHostNameRenderer(const char *name) : Renderer(name) { }

GetThisHostNameRenderer::~GetThisHostNameRenderer() { }

void GetThisHostNameRenderer::RenderAll(ostream &reply, Context &ctx, const ROAnything &config)
{
	StartTrace(GetThisHostNameRenderer.RenderAll);
	String thisHostName;
	if ( System::HostName(thisHostName) ) {
		String thisHostIp(Resolver::DNS2IPAddress(thisHostName));
		String thisHostDns(Resolver::IPAddress2DNS(thisHostIp));
		Trace("hostname [" << thisHostName << "] ip [" << thisHostIp << "] dns [" << thisHostDns << "]");
		StringTokenizer tokens(thisHostDns, '.');
		String hostName, domain;
		if (tokens.NextToken(hostName)) {
			domain = tokens.GetRemainder(false);
		}
		Trace("host [" << hostName << "] domain [" << domain << "]");
		String what(config["Representation"].AsString("Full"));
		if ( what == "Full" ) {
			reply << thisHostDns;
		} else if ( what == "HostOnly" ) {
			reply << hostName;
		} else if ( what == "DomainOnly" ) {
			reply << domain;
		}
	}
}
