/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
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
		InitFinisManager::IFMTrace(">> dataaccess::Init\n");
		InitFinisManager::IFMTrace("<< dataaccess::Init\n");
	}

	void Finis()
	{
		InitFinisManager::IFMTrace(">> dataaccess::Finis\n");
		InitFinisManager::IFMTrace("<< dataaccess::Finis\n");
	}
}

extern "C" void __attribute__ ((constructor)) dataaccess_init()
{
	Init();
}
extern "C" void __attribute__ ((destructor)) dataaccess_fini()
{
	Finis();
}
