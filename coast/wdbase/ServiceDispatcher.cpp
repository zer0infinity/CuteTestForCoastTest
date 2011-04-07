/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */
#include "ServiceDispatcher.h"
#include "ServiceHandler.h"
#include "Renderer.h"
#include "Policy.h"
RegisterModule(ServiceDispatchersModule);

bool ServiceDispatchersModule::Init(const ROAnything config) {
	if (config.IsDefined("ServiceDispatchers")) {
		HierarchyInstaller ai("ServiceDispatcher");
		return RegisterableObject::Install(config["ServiceDispatchers"], "ServiceDispatcher", &ai);
	}
	return false;
}

bool ServiceDispatchersModule::ResetFinis(const ROAnything config) {
	AliasTerminator at("ServiceDispatcher");
	return RegisterableObject::ResetTerminate("ServiceDispatcher", &at);
}

bool ServiceDispatchersModule::Finis() {
	return StdFinis("ServiceDispatcher", "ServiceDispatchers");
}
RegisterServiceDispatcher(ServiceDispatcher);

bool ServiceDispatcher::Dispatch2Service(std::ostream &reply, Context &ctx) {
	StartTrace(ServiceDispatcher.Dispatch2Service);
	String strKey("ServiceDispatcher");
	ctx.Push(strKey, this);
	ServiceHandler *sh = FindServiceHandler(ctx);
	// if no service handler is found, use DefaultHandler instead
	if (!sh) {
		String def = ServiceDispatcher::FindServiceName(ctx);
		Trace("using DefaultHandler [" << def << "]");
		sh = ServiceHandler::FindServiceHandler(def);
	}
	bool status = false;
	if (sh) {
		status = sh->HandleService(reply, ctx);
	}
	ctx.Pop(strKey); //!@FIXME: use PushPopEntry for LookupInterfaces too
	return status;
}

ServiceHandler *ServiceDispatcher::FindServiceHandler(Context &ctx) {
	StartTrace(ServiceDispatcher.FindServiceHandler);
	String name = FindServiceName(ctx);
	Trace("Service [" << name << "]");
	return ServiceHandler::FindServiceHandler(name);
}

String ServiceDispatcher::FindServiceName(Context &ctx) {
	StartTrace(ServiceDispatcher.FindServiceName);
	return ctx.Lookup("DefaultService", "WebAppService");
}
RegisterServiceDispatcher(RendererDispatcher);
long RendererDispatcher::FindURIPrefixInList(const String &requestURI, const ROAnything &uriPrefixList) {
	StartTrace(RendererDispatcher.FindURIPrefixInList);

	long apSz = uriPrefixList.GetSize();
	for (long i = 0; i < apSz; ++i) {
		const char *uriPrefix = uriPrefixList.SlotName(i);
		if (uriPrefix && requestURI.StartsWith(uriPrefix)) {
			return i;
		}
	}
	return -1;
}

String RendererDispatcher::FindServiceName(Context &ctx) {
	StartTrace(RendererDispatcher.FindServiceName);
	ROAnything uriPrefixList(ctx.Lookup("URIPrefix2ServiceMap"));
	String requestURI(ctx.Lookup("REQUEST_URI", ""));
	Anything query(ctx.GetQuery());
	SubTraceAny(uriPrefixList, uriPrefixList, "Service Prefixes: ");
	long matchedPrefix = FindURIPrefixInList(requestURI, uriPrefixList);
	if (matchedPrefix >= 0) {
		String service;
		Renderer::RenderOnString(service, ctx, uriPrefixList[matchedPrefix]);
		query["Service"] = service;
		query["URIPrefix"] = uriPrefixList.SlotName(matchedPrefix);
		SubTraceAny(query, query, "Query: ");
		Trace("Service [" << service << "]");
		return service;
	} else if (uriPrefixList.GetSize() > 0) {
		query["Error"] = String("Service[").Append(requestURI.SubString(0, requestURI.StrChr('/', 1))).Append("]NotFound");
	}
	String defaultHandler(ServiceDispatcher::FindServiceName(ctx));
	Trace("Service:<" << defaultHandler << ">");
	return defaultHandler;
}
