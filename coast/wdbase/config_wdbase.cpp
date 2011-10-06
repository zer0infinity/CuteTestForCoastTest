/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#include "InitFinisManager.h"
#include "SystemLog.h"

namespace {
	void Init()
	{
		InitFinisManager::IFMTrace(">> wdbase::Init\n");
		InitFinisManager::IFMTrace("<< wdbase::Init\n");
	}

	void Finis()
	{
#if defined(WIN32)
		TerminateKilledThreads();
#endif
		InitFinisManager::IFMTrace(">> wdbase::Finis\n");
		InitFinisManager::IFMTrace("<< wdbase::Finis\n");
	}
}

extern "C" void __attribute__ ((constructor)) wdbase_init()
{
	Init();
}
extern "C" void __attribute__ ((destructor)) wdbase_fini()
{
	Finis();
}
