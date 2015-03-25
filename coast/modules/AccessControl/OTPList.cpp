/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#include "OTPList.h"
#include "Tracer.h"
#include "Registry.h"
#include "Policy.h"

//---- OTPListsModule -----------------------------------------------------------
RegisterModule(OTPListsModule);

bool OTPListsModule::Init(const ROAnything config)
{
	if (config.IsDefined("OTPLists")) {
		AliasInstaller ai("OTPList");
		return RegisterableObject::Install(config["OTPLists"], "OTPList", &ai);
	}
	return false;
}

bool OTPListsModule::Finis()
{
	return StdFinis("OTPList", "OTPLists");
}

//---- OTPList -----------------------------------------------------------

RegCacheImpl(OTPList);	// FindOTPList()

