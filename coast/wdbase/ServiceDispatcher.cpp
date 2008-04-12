/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

//--- interface include --------------------------------------------------------
#include "ServiceDispatcher.h"

//--- standard modules used ----------------------------------------------------
#include "Registry.h"
#include "Timers.h"
#include "ServiceHandler.h"
#include "Renderer.h"
#include "Dbg.h"

//---- ServiceDispatchersModule -----------------------------------------------------------
RegisterModule(ServiceDispatchersModule);

ServiceDispatchersModule::ServiceDispatchersModule(const char *name) : WDModule(name)
{
}

ServiceDispatchersModule::~ServiceDispatchersModule()
{
}

bool ServiceDispatchersModule::Init(const ROAnything config)
{
	if (config.IsDefined("ServiceDispatchers")) {
		HierarchyInstaller ai("ServiceDispatcher");
		return RegisterableObject::Install(config["ServiceDispatchers"], "ServiceDispatcher", &ai);
	}
	return false;
}

bool ServiceDispatchersModule::ResetFinis(const ROAnything config)
{
	AliasTerminator at("ServiceDispatcher");
	return RegisterableObject::ResetTerminate("ServiceDispatcher", &at);
}

bool ServiceDispatchersModule::Finis()
{
	return StdFinis("ServiceDispatcher", "ServiceDispatchers");
}

//---- ServiceDispatcher -----------------------------------------------------------
RegisterServiceDispatcher(ServiceDispatcher);

ServiceDispatcher::ServiceDispatcher(const char *ServiceDispatcherName)
	: HierarchConfNamed(ServiceDispatcherName)
{
	StartTrace(ServiceDispatcher.Ctor);
}

ServiceDispatcher::~ServiceDispatcher()
{
	StartTrace(ServiceDispatcher.Dtor);

}

void ServiceDispatcher::Dispatch2Service(ostream &reply, Context &ctx)
{
	StartTrace(ServiceDispatcher.Dispatch2Service);
	ctx.Push("ServiceDispatcher", this);
	ServiceHandler *sh = FindServiceHandler(ctx);

	// if no service handler is found DefaultHandler is used
	if (!sh) {
		String def = ctx.Lookup("DefaultHandler", "WebAppService");
		Trace("not found, looking for DefaultHandler:" << def);
		sh = ServiceHandler::FindServiceHandler(def);
	}
	sh->HandleService(reply, ctx);
	String strKey;
	ctx.Pop(strKey);
}

ServiceHandler *ServiceDispatcher::FindServiceHandler(Context &ctx)
{
	StartTrace(ServiceDispatcher.FindServiceHandler);
	String name = FindServiceName(ctx);
	Trace("Service:" << name);
	return ServiceHandler::FindServiceHandler(name);
}

String ServiceDispatcher::FindServiceName(Context &ctx)
{
	StartTrace(ServiceDispatcher.FindServiceName);

	return ctx.Lookup("DefaultService", "WebAppService");
}

RegisterServiceDispatcher(RendererDispatcher);
//--- RendererDispatcher ---------------------------------------------------
RendererDispatcher::RendererDispatcher(const char *rendererDispatcherName)
	: ServiceDispatcher(rendererDispatcherName)
{

}

RendererDispatcher::~RendererDispatcher()
{

}

long RendererDispatcher::FindURIPrefixInList(const String &requestURI, const ROAnything &uriPrefixList)
{
	StartTrace(RendererDispatcher.FindURIPrefixInList);

	long apSz = uriPrefixList.GetSize();
	for (long i = 0; i < apSz; ++i) {
		const char *uriPrefix = uriPrefixList.SlotName(i);
		if ( uriPrefix && requestURI.StartsWith(uriPrefix) ) {
			return i;
		}
	}
	return -1;
}

long *RendererDispatcher::FindVlanInList(const String &requestVhost, const String &requestURI, const ROAnything &vhostList, long *retval)
{
	StartTrace(RendererDispatcher.FindVlanInList);

	long apSz = vhostList.GetSize();
	for (long i = 0; i < apSz; i++) {
		const char *vhost = vhostList.SlotName(i);
		if ( vhost && requestVhost.StartsWith(vhost) ) {
			long matchedPrefix = FindURIPrefixInList(requestURI, vhostList[i]);
			if (matchedPrefix >= 0) {
				retval[0] = i;
				retval[1] = matchedPrefix;
				break;
			}
		}
	}
	return retval;
}

String RendererDispatcher::FindServiceName(Context &ctx)
{
	StartTrace(RendererDispatcher.FindServiceName);

	ROAnything uriPrefixList(ctx.Lookup("URIPrefix2ServiceMap"));
	ROAnything vhostList(ctx.Lookup("VHost2ServiceMap"));
	String requestURI(ctx.Lookup("REQUEST_URI", ""));
	Trace("request URI [" << requestURI << "]");
	String requestVhost(ctx.Lookup("header.HOST", ""));

	Anything query(ctx.GetQuery());
	SubTraceAny(uriPrefixList, uriPrefixList, "Service Prefixes: ");

	long matchedPrefix = FindURIPrefixInList(requestURI, uriPrefixList);
	long buf[] = { -1, -1};
	long *matchedVhost = FindVlanInList(requestVhost, requestURI, vhostList, buf);

	if (matchedPrefix >= 0) {
		String service;
		Renderer::RenderOnString(service, ctx, uriPrefixList[matchedPrefix]);
		query["Service"] = service;
		query["URIPrefix"] = uriPrefixList.SlotName(matchedPrefix);

		SubTraceAny(query, query, "Query: ");
		Trace("Service:<" << service << ">");
		return service;
	} else if (matchedVhost[0] >= 0) {
		String service;
		Renderer::RenderOnString(service, ctx, vhostList[matchedVhost[0]][matchedVhost[1]]);
		query["Service"] = service;
		query["VHost"] = vhostList.SlotName(matchedVhost[0]);
		query["URIPrefix"] = vhostList[matchedVhost[0]].SlotName(matchedVhost[1]);

		SubTraceAny(query, query, "Query: ");
		Trace("Service:<" << service << ">");
		return service;
	} else if (uriPrefixList.GetSize() > 0 && vhostList.GetSize() > 0) {
		query["Error"] = "ServiceNotFound";
	}

	String defaultHandler(ServiceDispatcher::FindServiceName(ctx));
	Trace("Service:<" << defaultHandler << ">");
	return defaultHandler;
}

//---- registry interface
RegCacheImpl(ServiceDispatcher);	// FindServiceDispatcher()
