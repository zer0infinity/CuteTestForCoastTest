/*
 * Copyright (c) 2009, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#include "VHostServiceDispatcher.h"
#include "ServiceHandler.h"
#include "Renderer.h"
#include "Dbg.h"

RegisterServiceDispatcher(VHostServiceDispatcher);
//--- VHostServiceDispatcher ---------------------------------------------------
VHostServiceDispatcher::VHostServiceDispatcher(const char *dispatcherName) :
	RendererDispatcher(dispatcherName)
{
}

ServiceHandler *VHostServiceDispatcher::FindServiceHandler(Context &ctx)
{
	StartTrace(VHostServiceDispatcher.FindServiceHandler);
	String requestVhostWithPort(ctx.Lookup("header.HOST", ""));
	String requestVhost(requestVhostWithPort.SubString(0, requestVhostWithPort.StrChr(':')));
	Trace("request HOST [" << requestVhost << "]");
	String requestURI(ctx.Lookup("REQUEST_URI", ""));
	Trace("request URI [" << requestURI << "]");
	ServiceHandler *sh = ServiceHandler::FindServiceHandler(requestVhost);
	if ( sh ) {
		Anything query(ctx.GetQuery());
		query["VHost"] = requestVhost;
		query["EntryPage"] = requestURI;
		query["Service"] = requestVhost;
		ROAnything roaURIPrefixList;
		if ( sh->Lookup("URIPrefix2ServiceMap", roaURIPrefixList) ) {
			long matchedPrefix = FindURIPrefixInList(requestURI, roaURIPrefixList);
			if (matchedPrefix >= 0) {
				String service;
				ROAnything roaPrefixCfg = roaURIPrefixList[matchedPrefix];
				Renderer::RenderOnString(service, ctx, roaPrefixCfg);
				ServiceHandler *pfxService = 0;
				if ( service.Length() && (pfxService=ServiceHandler::FindServiceHandler(service)) != 0 ) {
					sh = pfxService;
					query["Service"] = service;
					query["URIPrefix"] = roaURIPrefixList.SlotName(matchedPrefix);
				}
			}
		}
		SubTraceAny(query, query, "Query for service");
		return sh;
	}
	return RendererDispatcher::FindServiceHandler(ctx);
}
