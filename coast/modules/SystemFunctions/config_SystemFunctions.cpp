/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

//--- interface include --------------------------------------------------------
#include "config_SystemFunctions.h"

//--- standard modules used ----------------------------------------------------
#include "InitFinisManager.h"

static void Init()
{
	InitFinisManager::IFMTrace(">> SystemFunctions::Init\n");
	InitFinisManager::IFMTrace("<< SystemFunctions::Init\n");
}

static void Finis()
{
	InitFinisManager::IFMTrace(">> SystemFunctions::Finis\n");
	InitFinisManager::IFMTrace("<< SystemFunctions::Finis\n");
}

#if defined(WIN32)
#ifdef _DLL
// DllMain() is the entry-point function for this DLL.
BOOL WINAPI	DllMain(HANDLE hinstDLL,	// DLL module handle
					DWORD fdwReason,					// reason called
					LPVOID lpvReserved)					// reserved
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

			// The DLL unloading due to process termination or call to FreeLibrary
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
extern "C" void __attribute__ ((constructor)) my_SystemFunctions_init()
{
	Init();
}
extern "C" void __attribute__ ((destructor)) my_SystemFunctions_fini()
{
	Finis();
}
#endif	// WIN32
