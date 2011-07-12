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
#include "Tracer.h"
#include "AnyIterators.h"

RegisterServiceDispatcher(VHostServiceDispatcher);
//--- VHostServiceDispatcher ---------------------------------------------------
namespace {
	Anything buildPathSegmentList(String requestURI) {
		StartTrace1(VHostServiceDispatcher.buildPathSegmentList, "input URI [" << requestURI << "]");
		Anything anySegments;
		long lSlashIdx = requestURI.StrChr('?'), lAppendIdx=-1L;
		if ( lSlashIdx != -1L ) {
			requestURI.Trim(lSlashIdx);
			lSlashIdx = -1L;
		}
		while ( requestURI.Length() ) {
			if ( lAppendIdx < 0L || !anySegments[lAppendIdx].IsEqual(static_cast<const char*>(requestURI)) ) {
				Trace("adding URI [" << requestURI << "]");
				lAppendIdx = anySegments.Append(requestURI);
			}
			lSlashIdx=requestURI.StrRChr('/', lSlashIdx);
			// trim to previous slash including trailing slash if any
			requestURI.Trim(lSlashIdx+1);
			Trace("trimmed URI [" << requestURI << "]");
		}
		return anySegments;
	}
}

ServiceHandler *VHostServiceDispatcher::findServiceBasedOnLongestURIPrefixMatch(Context &ctx, String& requestURI)
{
	StartTrace(VHostServiceDispatcher.findServiceBasedOnLongestURIPrefixMatch);
	ServiceHandler *sh = 0;
	Anything anySegments( buildPathSegmentList(requestURI) );
	TraceAny(anySegments, "path split into single segments");
	AnyExtensions::Iterator<ROAnything> aSegmentIter(anySegments);
	ROAnything roaSegment, roaURIPrefixEntry;
	while ( aSegmentIter.Next(roaSegment) ) {
		String strLookupSegment("URIPrefix2ServiceMap.");
		strLookupSegment.Append(roaSegment.AsString());
		Trace("current path to lookup [" << strLookupSegment << "]");
		if ( ctx.Lookup(strLookupSegment, roaURIPrefixEntry) || ctx.Lookup(strLookupSegment.Trim(strLookupSegment.Length()-1), roaURIPrefixEntry) ) {
			strLookupSegment.TrimFront(strLookupSegment.StrChr('.')+1);
			Trace("matched segment and new URIPrefix[" << strLookupSegment << "]");
			String service;
			Renderer::RenderOnString(service, ctx, roaURIPrefixEntry);
			Trace("servicename [" << service << "]");
			ServiceHandler *pfxService = 0;
			if ( service.Length() && (pfxService=ServiceHandler::FindServiceHandler(service)) != 0 ) {
				sh = pfxService;
				requestURI = strLookupSegment;
				break;
			}
		}
	}
	return sh;
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
		// allow finding URIPrefix2ServiceMap in our own config too
		ctx.Push("ServiceHandler", sh);
		ROAnything roaURIPrefixList;
		ServiceHandler *pfxService = 0;
		if ( (pfxService=findServiceBasedOnLongestURIPrefixMatch(ctx, requestURI)) != 0 ) {
			sh = pfxService;
			query["URIPrefix"] = requestURI;
		}
		query["Service"] = sh->GetName();
		String strKey;
		ctx.Pop(strKey);
		SubTraceAny(query, query, "Query for service");
		return sh;
	}
	return RendererDispatcher::FindServiceHandler(ctx);
}
