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
		InitFinisManager::IFMTrace(">> WorkerPoolManager::Init\n");
		InitFinisManager::IFMTrace("<< WorkerPoolManager::Init\n");
	}

	void Finis()
	{
		InitFinisManager::IFMTrace(">> WorkerPoolManager::Finis\n");
		InitFinisManager::IFMTrace("<< WorkerPoolManager::Finis\n");
	}
}

extern "C" void __attribute__ ((constructor)) WorkerPoolManager_init()
{
	Init();
}
extern "C" void __attribute__ ((destructor)) WorkerPoolManager_fini()
{
	Finis();
}
