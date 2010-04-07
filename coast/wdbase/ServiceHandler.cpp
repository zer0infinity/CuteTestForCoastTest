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
const char* ServiceHandler::gpcCategory = "ServiceHandler";
const char* ServiceHandler::gpcConfigPath = "ServiceHandlers";

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
	Trace("Service [" << fName << "]");
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
	ROAnything roaHandlerConfig;
	if ( config.LookupPath(roaHandlerConfig, ServiceHandler::gpcConfigPath) ) {
		HierarchyInstaller ai(ServiceHandler::gpcCategory);
		return RegisterableObject::Install(roaHandlerConfig, ServiceHandler::gpcCategory, &ai);
	}
	return false;
}

bool ServiceHandlersModule::ResetFinis(const ROAnything config)
{
	AliasTerminator at(ServiceHandler::gpcCategory);
	return RegisterableObject::ResetTerminate(ServiceHandler::gpcCategory, &at);
}

bool ServiceHandlersModule::Finis()
{
	return StdFinis(ServiceHandler::gpcCategory, ServiceHandler::gpcConfigPath);
}
