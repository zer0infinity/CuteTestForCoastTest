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
		InitFinisManager::IFMTrace(">> ldapdaicache::Init\n");
		InitFinisManager::IFMTrace("<< ldapdaicache::Init\n");
	}

	void Finis()
	{
		InitFinisManager::IFMTrace(">> ldapdaicache::Finis\n");
		InitFinisManager::IFMTrace("<< ldapdaicache::Finis\n");
	}
}


extern "C" void __attribute__ ((constructor)) ldapdaicache_init()
{
	Init();
}
extern "C" void __attribute__ ((destructor)) ldapdaicache_fini()
{
	Finis();
}
