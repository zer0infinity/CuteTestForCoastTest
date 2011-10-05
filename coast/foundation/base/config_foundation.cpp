/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#include "Tracer.h"
#include "SystemLog.h"
#include "InitFinisManager.h"

namespace {
	void Init()
	{
		InitFinisManager::IFMTrace(">> foundation::Init\n");
		InitFinisManager::IFMTrace("<< foundation::Init\n");
	}

	void Finis()
	{
		InitFinisManager::IFMTrace(">> foundation::Finis\n");
		InitFinisManager::IFMTrace("<< foundation::Finis\n");
	}
}

#if defined(__GNUG__)
extern "C" void __attribute__((constructor)) foundation_init()
{
	Init();
}
extern "C" void __attribute__((destructor)) foundation_fini()
{
	Finis();
}
#endif
