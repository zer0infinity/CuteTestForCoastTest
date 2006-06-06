/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

//--- interface include --------------------------------------------------------
#include "ServerPoolsManagerInterface.h"

//--- standard modules used ----------------------------------------------------
#include "Timers.h"
#include "Threads.h"
#include "Registry.h"
#include "Dbg.h"

//---- ServerPoolsManagerInterfacesModule -----------------------------------------------------------
RegisterModule(ServerPoolsManagerInterfacesModule);

ServerPoolsManagerInterfacesModule::ServerPoolsManagerInterfacesModule(const char *name) : WDModule(name)
{
}

ServerPoolsManagerInterfacesModule::~ServerPoolsManagerInterfacesModule()
{
}

bool ServerPoolsManagerInterfacesModule::Init(const ROAnything config)
{
	if (config.IsDefined("ServerPoolsManagerInterfaces")) {
		AliasInstaller ai("ServerPoolsManagerInterface");
		return RegisterableObject::Install(config["ServerPoolsManagerInterfaces"], "ServerPoolsManagerInterface", &ai);
	}
	return false;
}

bool ServerPoolsManagerInterfacesModule::ResetFinis(const ROAnything )
{
	AliasTerminator at("ServerPoolsManagerInterface");
	return RegisterableObject::ResetTerminate("ServerPoolsManagerInterface", &at);
}

bool ServerPoolsManagerInterfacesModule::Finis()
{
	return StdFinis("ServerPoolsManagerInterface", "ServerPoolsManagerInterfaces");
}

//---- ServerPoolsManagerInterface ------------------------------------------------------------------
ServerPoolsManagerInterface::ServerPoolsManagerInterface(const char *ServerThreadPoolsManagerName)
	: 	ConfNamedObject(ServerThreadPoolsManagerName),
		fReady(false),
		fMutex("ServerPoolsManagerInterface")
{
	StartTrace(ServerPoolsManagerInterface.Ctor);
}

ServerPoolsManagerInterface::~ServerPoolsManagerInterface()
{
	StartTrace(ServerPoolsManagerInterface.Dtor);
}

void ServerPoolsManagerInterface::SetReady(bool ready)
{
	MutexEntry me(fMutex);
	fReady = ready;
	fCond.BroadCast();
}

bool ServerPoolsManagerInterface::IsReady(bool ready, long timeout)
{
	MutexEntry me(fMutex);
	while ( fReady != ready ) {
		long ret = fCond.TimedWait(fMutex, timeout);
		if ( ret == TIMEOUTCODE ) {
			break;
		}
	}
	return (fReady == ready );
}

//---- registry interface
RegCacheImpl(ServerPoolsManagerInterface);	// FindServerPoolsManagerInterface()
