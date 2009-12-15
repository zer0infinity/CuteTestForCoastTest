/*
 * Copyright (c) 2009, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#include "VHostServiceDispatcher.h"
#include "Renderer.h"
#include "Dbg.h"
RegisterServiceDispatcher(VHostServiceDispatcher);
//--- VHostServiceDispatcher ---------------------------------------------------
VHostServiceDispatcher::VHostServiceDispatcher(const char *dispatcherName) :
	RendererDispatcher(dispatcherName)
{
}

void VHostServiceDispatcher::FindVHostInList(const String &requestVhost,
		const String &requestURI, const ROAnything &vhostList,
		long &matchedVhost, long &matchedVhostPrefix)
{
	StartTrace(VHostServiceDispatcher.FindVHostInList);

	long apSz = vhostList.GetSize();
	for (long i = 0; i < apSz; i++) {
		const char *vhost = vhostList.SlotName(i);
		if (vhost && requestVhost.StartsWith(vhost)) {
			long matchedPrefix = FindURIPrefixInList(requestURI, vhostList[i]);
			if (matchedPrefix >= 0) {
				matchedVhost = i;
				matchedVhostPrefix = matchedPrefix;
				break;
			}
		}
	}
}

String VHostServiceDispatcher::FindServiceName(Context &ctx)
{
	StartTrace(VHostServiceDispatcher.FindServiceName);

	ROAnything vhostList(ctx.Lookup("VHost2ServiceMap"));
	String requestURI(ctx.Lookup("REQUEST_URI", ""));
	Trace("request URI [" << requestURI << "]");
	String requestVhost(ctx.Lookup("header.HOST", ""));

	Anything query(ctx.GetQuery());

	long matchedVhost = -1;
	long matchedVhostPrefix = -1;
	FindVHostInList(requestVhost, requestURI, vhostList, matchedVhost,
					matchedVhostPrefix);

	if (matchedVhost >= 0) {
		String service;
		Renderer::RenderOnString(service, ctx,
								 vhostList[matchedVhost][matchedVhostPrefix]);
		query["Service"] = service;
		query["VHost"] = vhostList.SlotName(matchedVhost);
		query["URIPrefix"] = vhostList[matchedVhost].SlotName(
								 matchedVhostPrefix);
		SubTraceAny(query, query, "Query for service <" << service << ">");
		return service;
	}
	return RendererDispatcher::FindServiceName(ctx);
}
