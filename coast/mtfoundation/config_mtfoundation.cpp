/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#include "config_mtfoundation.h"
#include "InitFinisManagerMTFoundation.h"
#include "SystemLog.h"
#include "Threads.h"

namespace {
	void Init()
	{
		InitFinisManager::IFMTrace(">> mtfoundation::Init\n");
		// initialize InitFinisManagerMTFoundation relative components
		if ( InitFinisManagerMTFoundation::Instance() != NULL ) {
			InitFinisManagerMTFoundation::Instance()->Init();
		}
		InitFinisManager::IFMTrace("<< mtfoundation::Init\n");
	}

	void Finis()
	{
		InitFinisManager::IFMTrace(">> mtfoundation::Finis\n");
		// finalize InitFinisManagerMTFoundation relative components
		if ( InitFinisManagerMTFoundation::Instance() != NULL ) {
			InitFinisManagerMTFoundation::Instance()->Finis();
			delete InitFinisManagerMTFoundation::Instance();
		}
		InitFinisManager::IFMTrace("<< mtfoundation::Finis\n");
	}
}

#if defined(WIN32)
#ifdef _DLL
#include "StringStream.h"
#include "AnyIterators.h"

DWORD fgThreadPtrKey;
Anything fgThreads;
SimpleMutex fgThreadsMutex("fgThreadsMutex", Coast::Storage::Global());

void TerminateKilledThreads()
{
	static bool once = false;
	if (!once) {
		if (fgThreads.GetSize()) {
			SystemLog::Warning("mtfoundation: following threads were still active !!");
			String strbuf;
			StringStream stream(strbuf);
			fgThreads.PrintOn(stream) << "\n";
			stream.flush();
			SystemLog::WriteToStderr(strbuf);
			AnyExtensions::Iterator<ROAnything> aIter(fgThreads);
			ROAnything aAny;
			while (aIter.Next(aAny)) {
				Thread *pThr = (Thread *)aAny["Addr"].AsIFAObject();
				SystemLog::Warning(String("  Thread[") << aAny["Name"].AsString() << "] Handle[" << aAny["id"].AsLong() << "] Addr [" << (long)aAny["Addr"].AsIFAObject() << "]");
				if (pThr && pThr->IsAlive()) {
					pThr->IntSetState(Thread::eTerminationRequested);
				}
			}
		}
		once = true;
	}
}

void RemoveThreadDetach(Thread *pThr)
{
	String key;
	key = Anything((long)pThr).AsString();
	LockUnlockEntry sm(fgThreadsMutex);
	if (pThr && fgThreads.IsDefined(key)) {
		fgThreads.Remove(key);
	}
}

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
			if (THRKEYCREATE(fgThreadPtrKey, 0)) {
				SystemLog::Error("TlsAlloc of fgThreadPtrKey failed");
			}
			break;

			// The attached process creates a new thread.
		case DLL_THREAD_ATTACH:
			SystemLog::Info(String("mtfoundation: DLL_THREAD_ATTACH for [") << Thread::MyId() << "]");
			break;

			// The thread of the attached process terminates.
		case DLL_THREAD_DETACH: {
			Thread *pThr = (Thread *)TlsGetValue(fgThreadPtrKey);
			SystemLog::Info(String("DLL_THREAD_DETACH for [") << Thread::MyId() << "] &Thread:"  << (long)pThr );
			if ( pThr ) {
				// there seems to be a valid ThreadPtr, eg. it is a mtfoundation Thread
				RemoveThreadDetach(pThr);
				pThr->SetState(Thread::eTerminated);
			} else {
				SYSINFO("Thread* was NULL for [" << Thread::MyId() << "] ErrorMessage: [" << SystemLog::LastSysError() << "]");
			}
			break;
		}

		// The DLL unloading due to process termination or call to FreeLibrary.
		case DLL_PROCESS_DETACH:
			TerminateKilledThreads();
			THRKEYDELETE(fgThreadPtrKey);
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
extern "C" void __attribute__ ((constructor)) mtfoundation_init()
{
	Init();
}
extern "C" void __attribute__ ((destructor)) mtfoundation_fini()
{
	Finis();
}
#endif	// WIN32
