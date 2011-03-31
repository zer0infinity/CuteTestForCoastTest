/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#include "InitFinisManager.h"

namespace {
	void Init()
	{
		InitFinisManager::IFMTrace(">> bases::Init\n");
		InitFinisManager::IFMTrace("<< bases::Init\n");
	}

	void Finis()
	{
		InitFinisManager::IFMTrace(">> bases::Finis\n");
		InitFinisManager::IFMTrace("<< bases::Finis\n");
	}
}

extern "C" void __attribute__ ((constructor)) bases_init()
{
	Init();
}
extern "C" void __attribute__ ((destructor)) bases_fini()
{
	Finis();
}
