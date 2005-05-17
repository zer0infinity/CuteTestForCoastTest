/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

//--- interface include --------------------------------------------------------
#include "config_Queueing.h"

//--- used modules
#if defined(WIN32)
#ifdef _DLL
#include "SysLog.h"

// DllMain() is the entry-point function for this DLL.
BOOL WINAPI	DllMain(HANDLE hinstDLL,	// DLL module handle
					DWORD fdwReason,					// reason called
					LPVOID lpvReserved)					// reserved
{
	switch (fdwReason) {

			// The DLL is loading due to process
			// initialization or a call to LoadLibrary.
		case DLL_PROCESS_ATTACH:
			SysLog::Info("Queueing: DLL_PROCESS_ATTACH called");
			break;

			// The attached process creates a new thread.
		case DLL_THREAD_ATTACH:
			break;

			// The thread of the attached process terminates.
		case DLL_THREAD_DETACH:
			break;

			// The DLL unloading due to process termination or call to FreeLibrary
		case DLL_PROCESS_DETACH:
			SysLog::Info("Queueing: DLL_PROCESS_DETACH called");
			break;

		default:
			break;
	}

	return true;
	UNREFERENCED_PARAMETER(hinstDLL);
	UNREFERENCED_PARAMETER(lpvReserved);
}

#endif	// _DLL
#endif	// WIN32
