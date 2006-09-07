/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

//--- interface include --------------------------------------------------------
#include "ServiceHandler.h"

//--- standard modules used ----------------------------------------------------
#include "Registry.h"
#include "Dbg.h"

//---- ServiceHandler -----------------------------------------------------------
ServiceHandler::ServiceHandler(const char *ServiceHandlerName)
	: HierarchConfNamed(ServiceHandlerName)
{
	StartTrace(ServiceHandler.Ctor);
	Trace("name:" << ServiceHandlerName);
}

ServiceHandler::~ServiceHandler()
{
	StartTrace(ServiceHandler.Dtor);
}
void ServiceHandler::HandleService(ostream &os, Context &ctx)
{
	StartTrace(ServiceHandler.HandleService);
	Trace("servicename:" << fName);
	ctx.Push("ServiceHandler", this);
	DoHandleService(os, ctx);
	String strKey;
	ctx.Pop(strKey);
}

//---- registry interface
RegCacheImpl(ServiceHandler);	// FindServiceHandler()

//---- ServiceHandlersModule -----------------------------------------------------------
RegisterModule(ServiceHandlersModule);

ServiceHandlersModule::ServiceHandlersModule(const char *name)
	: WDModule(name)
{
}

ServiceHandlersModule::~ServiceHandlersModule()
{
}

bool ServiceHandlersModule::Init(const ROAnything config)
{
	if (config.IsDefined("ServiceHandlers")) {
		HierarchyInstaller ai("ServiceHandler");
		return RegisterableObject::Install(config["ServiceHandlers"], "ServiceHandler", &ai);
	}
	return false;
}

bool ServiceHandlersModule::ResetFinis(const ROAnything config)
{
	AliasTerminator at("ServiceHandler");
	return RegisterableObject::ResetTerminate("ServiceHandler", &at);
}

bool ServiceHandlersModule::Finis()
{
	return StdFinis("ServiceHandler", "ServiceHandlers");
}
