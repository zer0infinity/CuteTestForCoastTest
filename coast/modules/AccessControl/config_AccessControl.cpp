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
		InitFinisManager::IFMTrace(">> AccessControl::Init\n");
		InitFinisManager::IFMTrace("<< AccessControl::Init\n");
	}

	void Finis()
	{
		InitFinisManager::IFMTrace(">> AccessControl::Finis\n");
		InitFinisManager::IFMTrace("<< AccessControl::Finis\n");
	}
}

extern "C" void __attribute__ ((constructor)) AccessControl_init()
{
	Init();
}
extern "C" void __attribute__ ((destructor)) AccessControl_fini()
{
	Finis();
}
