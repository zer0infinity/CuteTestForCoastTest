/*
 * Copyright (c) 2009, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

//--- interface include --------------------------------------------------------

//--- standard modules used ----------------------------------------------------
#include "InitFinisManager.h"

namespace {
	static void Init()
	{
		InitFinisManager::IFMTrace(">> Oracle::Init\n");
		InitFinisManager::IFMTrace("<< Oracle::Init\n");
	}

	static void Finis()
	{
		InitFinisManager::IFMTrace(">> Oracle::Finis\n");
		InitFinisManager::IFMTrace("<< Oracle::Finis\n");
	}
}

extern "C" void __attribute__ ((constructor)) Oracle_init()
{
	Init();
}
extern "C" void __attribute__ ((destructor)) Oracle_fini()
{
	Finis();
}
