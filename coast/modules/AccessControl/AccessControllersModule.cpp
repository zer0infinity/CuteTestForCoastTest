/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#include "AccessControllersModule.h"
#include "SimpleAccessControllers.h"
#include "Dbg.h"
#include "Registry.h"
#include "Policy.h"

//---- AccessControllersModule -----------------------------------------------------------
RegisterModule(AccessControllersModule);

AccessControllersModule::AccessControllersModule(const char *name) : WDModule(name) {}

AccessControllersModule::~AccessControllersModule() {}

bool AccessControllersModule::Init(const ROAnything config)
{
	StartTrace(AccessControllersModule.Init);

	if (config.IsDefined("AccessControllers")) {
		ROAnything controllers(config["AccessControllers"]);

		AliasInstaller ai1("UserDataAccessController");
		bool ret = RegisterableObject::Install(controllers["User"], "UserDataAccessController", &ai1);

		AliasInstaller ai2("TokenDataAccessController");
		ret = RegisterableObject::Install(controllers["Token"], "TokenDataAccessController", &ai2) && ret;

		AliasInstaller ai3("EntityDataAccessController");
		return RegisterableObject::Install(controllers["Entity"], "EntityDataAccessController", &ai3) && ret;

	}
	return false;
}

bool AccessControllersModule::ResetFinis(const ROAnything )
{
	StartTrace(AccessControllersModule.ResetFinis);

	AliasTerminator at1("UserDataAccessController");
	bool ret = RegisterableObject::ResetTerminate("UserDataAccessController", &at1);

	AliasTerminator at2("TokenDataAccessController");
	ret = RegisterableObject::ResetTerminate("TokenDataAccessController", &at2) && ret;

	AliasTerminator at3("EntityDataAccessController");
	return RegisterableObject::ResetTerminate("EntityDataAccessController", &at3) && ret;
}

bool AccessControllersModule::Finis()
{
	StartTrace(AccessControllersModule.Finis);

	bool ret = StdFinis("UserDataAccessController", "UserDataAccessController");
	ret = StdFinis("TokenDataAccessController", "TokenDataAccessController") && ret;
	return StdFinis("EntityDataAccessController", "EntityDataAccessController") && ret;
}
