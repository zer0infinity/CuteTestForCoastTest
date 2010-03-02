/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

//--- interface include ---------------------------------------------------------
#include "config_wdbase.h"

//--- project modules used -----------------------------------------------------
#include "InitFinisManagerWDBase.h"

//--- standard modules used ----------------------------------------------------
#include "SystemLog.h"

// special include to inline static objects needed for registry
// -> this one needs to be here - or say needs to be the first object linked/created by the linker/runtime
#include "RegistryInitFinis.h"

static void Init()
{
	InitFinisManager::IFMTrace(">> wdbase::Init\n");
	// initialize InitFinisManagerWDBase relative components
	if ( InitFinisManagerWDBase::Instance() != NULL ) {
		InitFinisManagerWDBase::Instance()->Init();
	}
	InitFinisManager::IFMTrace("<< wdbase::Init\n");
}

static void Finis()
{
	InitFinisManager::IFMTrace(">> wdbase::Finis\n");
	// finalize InitFinisManagerWDBase relative components
	if ( InitFinisManagerWDBase::Instance() != NULL ) {
		InitFinisManagerWDBase::Instance()->Finis();
		delete InitFinisManagerWDBase::Instance();
	}
	InitFinisManager::IFMTrace("<< wdbase::Finis\n");
}

#if defined(WIN32)
#ifdef _DLL
#include "Threads.h"
// DllMain() is the entry-point function for this DLL.
BOOL WINAPI DllMain(HANDLE hinstDLL,  // DLL module handle
					DWORD fdwReason,                    // reason called
					LPVOID lpvReserved)                 // reserved
{
	switch (fdwReason) {

			// The DLL is loading due to process
			// initialization or a call to LoadLibrary.
		case DLL_PROCESS_ATTACH:
			Init();
			break;

			// The attached process creates a new thread.
		case DLL_THREAD_ATTACH:
			SystemLog::Info(String("wdbase::DLL_THREAD_ATTACH for [") << Thread::MyId() << "]");
			break;

			// The thread of the attached process terminates.
		case DLL_THREAD_DETACH:
			SystemLog::Info(String("wdbase::DLL_THREAD_DETACH for [") << Thread::MyId() << "]");
			break;

			// The DLL unloading due to process termination or call to FreeLibrary.
		case DLL_PROCESS_DETACH:
			TerminateKilledThreads();
			Finis();
			break;

		default:
			break;
	}

	return true;
	UNREFERENCED_PARAMETER(hinstDLL);
	UNREFERENCED_PARAMETER(lpvReserved);
}
#endif	// _DLL
#else
extern "C" void __attribute__ ((constructor)) wdbase_init()
{
	Init();
}
extern "C" void __attribute__ ((destructor)) wdbase_fini()
{
	Finis();
}
#endif	// WIN32
