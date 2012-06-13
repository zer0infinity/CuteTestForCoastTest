/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#include "GetThisHostNameRenderer.h"
#include "SystemBase.h"
#include "Resolver.h"

RegisterRenderer(GetThisHostNameRenderer);

void GetThisHostNameRenderer::RenderAll(std::ostream &reply, Context &ctx, const ROAnything &config) {
	StartTrace(GetThisHostNameRenderer.RenderAll);
	String thisHostName;
	if (Coast::System::HostName(thisHostName)) {
		String thisHostIp(Resolver::DNS2IPAddress(thisHostName));
		String what(config["Representation"].AsString("Full"));
		if ( what.IsEqual("IPAddress") ) {
			reply << thisHostIp << std::flush;
			return;
		}
		String thisHostDns(Resolver::IPAddress2DNS(thisHostIp));
		thisHostDns.ToLower();
		Trace("hostname [" << thisHostName << "] ip [" << thisHostIp << "] dns [" << thisHostDns << "]");
		StringTokenizer tokens(thisHostDns, '.');
		String hostName, domain;
		if (tokens.NextToken(hostName)) {
			domain = tokens.GetRemainder(false);
		}
		Trace("host [" << hostName << "] domain [" << domain << "]");
		if (what.IsEqual("Full")) {
			reply << thisHostDns;
		} else if (what.IsEqual("HostOnly")) {
			reply << hostName;
		} else if (what.IsEqual("DomainOnly")) {
			reply << domain;
		}
		reply << std::flush;
	}
}
