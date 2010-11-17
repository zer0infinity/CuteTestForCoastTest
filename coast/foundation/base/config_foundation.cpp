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
#include "Dbg.h"
#include "SystemLog.h"
#include "InitFinisManagerFoundation.h"

namespace {
	#if defined(WIN32)
	void InitWSock()
	{
		static bool once = false;
		if (!once) {
			once = true;
			WORD wVersionRequested;
			WSADATA wsaData = { 0, 0, {0}, {0}, 0, 0, 0 };
			int err;

			wVersionRequested = MAKEWORD( 2, 0 );

			err = WSAStartup( wVersionRequested, &wsaData );
			if ( err != 0 ) {
				/* Tell the user that we couldn't find a useable */
				/* WinSock DLL.                                  */
				return;
			}

			/* Confirm that the WinSock DLL supports 2.0.*/
			/* Note that if the DLL supports versions greater    */
			/* than 2.0 in addition to 2.0, it will still return */
			/* 2.0 in wVersion since that is the version we      */
			/* requested.                                        */

			if ( LOBYTE( wsaData.wVersion ) != 2 ||
				 HIBYTE( wsaData.wVersion ) != 0 ) {
				/* Tell the user that we couldn't find a useable */
				/* WinSock DLL.                                  */
				WSACleanup( );
				return;
			}
		}
	}

	void CleanupWSock()
	{
		WSACleanup();
	}
	#endif

	void Init()
	{
		InitFinisManager::IFMTrace(">> foundation::Init\n");
		// initialize syslog channel
		SystemLog::Init("Coast");

	#if defined(WIN32)
		InitWSock();
	#endif

		// initialize InitFinisManagerFoundation relative components
		if ( InitFinisManagerFoundation::Instance() != NULL ) {
			InitFinisManagerFoundation::Instance()->Init();
		}
		InitFinisManager::IFMTrace("<< foundation::Init\n");
	}

	void Finis()
	{
		InitFinisManager::IFMTrace(">> foundation::Finis\n");

	#if defined(WIN32)
		CleanupWSock();
	#endif

		// finalize InitFinisManagerFoundation relative components
		if ( InitFinisManagerFoundation::Instance() != NULL ) {
			InitFinisManagerFoundation::Instance()->Finis();
			delete InitFinisManagerFoundation::Instance();
		}
		SystemLog::Terminate();
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
