/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

//--- interface include --------------------------------------------------------
#include "AccessControllersModule.h"

//--- project modules used -----------------------------------------------------

// kgu: NOTE: The following is not required, but we want to grant that
// at least one concrete AccessController implementation is registered,
// if the AccessControllersModule is used.
#include "SimpleAccessControllers.h"

//--- standard modules used ----------------------------------------------------
#include "Dbg.h"
#include "Registry.h"

//--- c-modules used -----------------------------------------------------------

//---- AccessControllersModule -----------------------------------------------------------
RegisterModule(AccessControllersModule);

AccessControllersModule::AccessControllersModule(const char *name) : WDModule(name) {}

AccessControllersModule::~AccessControllersModule() {}

bool AccessControllersModule::Init(const Anything &config)
{
	StartTrace(AccessControllersModule.Init);

	if (config.IsDefined("AccessControllers")) {
		Anything controllers(config["AccessControllers"]);

		AliasInstaller ai1("UserDataAccessController");
		bool ret = RegisterableObject::Install(controllers["User"], "UserDataAccessController", &ai1);

		AliasInstaller ai2("TokenDataAccessController");
		ret = RegisterableObject::Install(controllers["Token"], "TokenDataAccessController", &ai2) && ret;

		AliasInstaller ai3("EntityDataAccessController");
		return RegisterableObject::Install(controllers["Entity"], "EntityDataAccessController", &ai3) && ret;

	}
	return false;
}

bool AccessControllersModule::ResetFinis(const Anything &)
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
