/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

//--- interface include ---------------------------------------------------------

//--- project modules used -----------------------------------------------------
#include "InitFinisManagerWDBase.h"

//--- standard modules used ----------------------------------------------------
#include "SystemLog.h"

// special include to inline static objects needed for registry
// -> this one needs to be here - or say needs to be the first object linked/created by the linker/runtime
#include "RegistryInitFinis.h"

namespace {
	void Init()
	{
		InitFinisManager::IFMTrace(">> wdbase::Init\n");
		// initialize InitFinisManagerWDBase relative components
		if ( InitFinisManagerWDBase::Instance() != NULL ) {
			InitFinisManagerWDBase::Instance()->Init();
		}
		InitFinisManager::IFMTrace("<< wdbase::Init\n");
	}

	void Finis()
	{
#if defined(WIN32)
		TerminateKilledThreads();
#endif
		InitFinisManager::IFMTrace(">> wdbase::Finis\n");
		// finalize InitFinisManagerWDBase relative components
		if ( InitFinisManagerWDBase::Instance() != NULL ) {
			InitFinisManagerWDBase::Instance()->Finis();
			delete InitFinisManagerWDBase::Instance();
		}
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
