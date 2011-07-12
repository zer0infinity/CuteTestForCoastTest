/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#include "AccessManager.h"
#include "Registry.h"
#include "Tracer.h"
#include "Policy.h"

//---- AccessManagerModule ---------------------------------------------------------------
RegisterModule(AccessManagerModule);

// initialize static fields
AccessManager	*AccessManagerModule::fgMainAccessManager = 0;

bool AccessManagerModule::Init(const ROAnything config)
{
	StartTrace(AccessManagerModule.Init);
	bool ok = true;
	// impl analog PagesModule f�r Finis und Init

	if (config.IsDefined("AccessManagers")) {
		HierarchyInstaller hi("AccessManager");
		ok = RegisterableObject::Install(config["AccessManagers"], "AccessManager", &hi);
		Trace("access managers installed");
	}
	Trace("ok = " << ok);

	String dflt = config["DefaultAccessManager"].AsString("NoDefaultAccessManagerSpecified");
	fgMainAccessManager = AccessManager::FindAccessManager(dflt);
	Trace("default access manager '" << dflt << "' = " << (fgMainAccessManager ? "ok" : "not found"));

	return ok && fgMainAccessManager;
}

bool AccessManagerModule::Finis()
{
	StartTrace(AccessManagerModule.Finis);
	AliasTerminator at("AccessManager");
	return RegisterableObject::ResetTerminate("AccessManager", &at);
}

AccessManager *AccessManagerModule::GetAccessManager(const char *name)
{
	if (name) {
		return AccessManager::FindAccessManager(name);
	}
	return fgMainAccessManager;
}

//---- AccessManager -------------------------------------------------------
// (no implementation - is abstract)

//---- registry interface
RegCacheImpl(AccessManager);	// FindAccessManager() implementation

