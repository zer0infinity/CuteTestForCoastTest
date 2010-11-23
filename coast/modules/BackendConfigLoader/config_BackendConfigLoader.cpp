/*
 * Copyright (c) 2008, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

//--- interface include ---------------------------------------------------------

//--- standard modules used ----------------------------------------------------
#include "InitFinisManager.h"

namespace {
	void Init()
	{
		InitFinisManager::IFMTrace(">> BackendConfigLoader::Init\n");
		InitFinisManager::IFMTrace("<< BackendConfigLoader::Init\n");
	}

	void Finis()
	{
		InitFinisManager::IFMTrace(">> BackendConfigLoader::Finis\n");
		InitFinisManager::IFMTrace("<< BackendConfigLoader::Finis\n");
	}
}

extern "C" void __attribute__ ((constructor)) BackendConfigLoader_init()
{
	Init();
}
extern "C" void __attribute__ ((destructor)) BackendConfigLoader_fini()
{
	Finis();
}
