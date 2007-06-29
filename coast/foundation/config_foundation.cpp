/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

//--- interface include ---------------------------------------------------------
#include "config_foundation.h"

//--- standard modules used ----------------------------------------------------
#if defined(WIN32)
#include "Socket.h"
#endif
#include "Dbg.h"
#include "SysLog.h"
#include "InitFinisManagerFoundation.h"

static void Init()
{
	InitFinisManager::IFMTrace(">> foundation::Init\n");
	// initialize syslog channel
	SysLog::Init("Coast");
	// initialize tracing - if available
	ResetTracer();
	// initialize InitFinisManagerFoundation relative components
	if ( InitFinisManagerFoundation::Instance() != NULL ) {
		InitFinisManagerFoundation::Instance()->Init();
	}
#if defined(WIN32)
	Socket::InitWSock();
#endif
	InitFinisManager::IFMTrace("<< foundation::Init\n");
}

static void Finis()
{
	InitFinisManager::IFMTrace(">> foundation::Finis\n");
#if defined(WIN32)
	Socket::CleanupWSock();
#endif
	TerminateTracer();
	// finalize InitFinisManagerFoundation relative components
	if ( InitFinisManagerFoundation::Instance() != NULL ) {
		InitFinisManagerFoundation::Instance()->Finis();
		delete InitFinisManagerFoundation::Instance();
	}
	SysLog::Terminate();
	InitFinisManager::IFMTrace("<< foundation::Finis\n");
}

#if defined(WIN32)
#ifdef _DLL
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
			break;

			// The thread of the attached process terminates.
		case DLL_THREAD_DETACH:
			break;

			// The DLL unloading due to process termination or call to FreeLibrary.
		case DLL_PROCESS_DETACH:
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
#if defined(__GNUG__) || ( defined(__SUNPRO_CC) && ( __SUNPRO_CC >= 0x590 ) )
extern "C" void __attribute__ ((constructor)) foundation_init()
{
	Init();
}
extern "C" void __attribute__ ((destructor)) foundation_fini()
{
	Finis();
}
#elif defined(__SUNPRO_CC) && ( __SUNPRO_CC < 0x590 )
#pragma init (Init)
#pragma fini (Finis)
#endif
#endif	// WIN32
