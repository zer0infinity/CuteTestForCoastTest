/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

//--- interface include --------------------------------------------------------

//--- standard modules used ----------------------------------------------------
#include "InitFinisManager.h"

namespace {
	void Init()
	{
		InitFinisManager::IFMTrace(">> TestLib::Init\n");
		InitFinisManager::IFMTrace("<< TestLib::Init\n");
	}

	void Finis()
	{
		InitFinisManager::IFMTrace(">> TestLib::Finis\n");
		InitFinisManager::IFMTrace("<< TestLib::Finis\n");
	}
}

extern "C" void __attribute__ ((constructor)) TestLib_init()
{
	Init();
}
extern "C" void __attribute__ ((destructor)) TestLib_fini()
{
	Finis();
}
