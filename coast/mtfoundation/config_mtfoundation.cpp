/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

//--- interface include ---------------------------------------------------------
#include "config_mtfoundation.h"

#if defined(WIN32)
#ifdef _DLL
//--- standard modules used ----------------------------------------------------
#include "SysLog.h"
#include "Threads.h"
#include "StringStream.h"
DWORD fgThreadPtrKey;
Anything fgThreads;
SimpleMutex fgThreadsMutex("fgThreadsMutex");

void EXPORTDECL_MTFOUNDATION TerminateKilledThreads()
{
	static bool once = false;
	if (!once) {
		if (fgThreads.GetSize()) {
			SysLog::Warning("mtfoundation: following threads were still active !!");
			String strbuf;
			StringStream stream(strbuf);
			fgThreads.PrintOn(stream) << "\n";
			stream.flush();
			SysLog::WriteToStderr(strbuf);
			AnythingIterator aIter(fgThreads);
			Anything aAny;
			while (aIter.Next(aAny)) {
				Thread *pThr = (Thread *)aAny["Addr"].AsIFAObject();
				SysLog::Warning(String("  Thread[") << aAny["Name"].AsString() << "] Handle[" << aAny["id"].AsLong() << "] Addr [" << (long)aAny["Addr"].AsIFAObject() << "]");
				if (pThr && pThr->IsAlive()) {
					pThr->IntSetState(Thread::eTerminated);
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
	SimpleMutexEntry sm(fgThreadsMutex);
	sm.Use();
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
		case DLL_PROCESS_ATTACH: {
			SysLog::Info("mtfoundation: DLL_PROCESS_ATTACH called");
			SysLog::Info("mtfoundation: allocating TLS indexes");
			// Allocate a TLS index.
			if (THRKEYCREATE(MT_Storage::fgAllocatorKey, 0)) {
				SysLog::Error("TlsAlloc of fgAllocatorKey failed");
				return false;
			}
			if (THRKEYCREATE(Thread::fgCleanerKey, 0)) {
				SysLog::Error("TlsAlloc of Thread::fgCleanerKey failed");
			}
			if (THRKEYCREATE(fgThreadPtrKey, 0)) {
				SysLog::Error("TlsAlloc of fgThreadPtrKey failed");
			}
			if (THRKEYCREATE(Mutex::fgCountTableKey, 0)) {
				SysLog::Error("TlsAlloc of Mutex::fgCountTableKey failed");
			}
			break;
		}

		// The attached process creates a new thread.
		case DLL_THREAD_ATTACH:
			SysLog::Info(String("mtfoundation: DLL_THREAD_ATTACH for [") << Thread::MyId() << "]");
			break;

			// The thread of the attached process terminates.
		case DLL_THREAD_DETACH: {
			Thread *pThr = (Thread *)TlsGetValue(fgThreadPtrKey);
			SysLog::Info(String("DLL_THREAD_DETACH for [") << Thread::MyId() << "] &Thread:"  << (long)pThr );
			if ( pThr ) {
				// there seems to be a valid ThreadPtr, eg. it is a mtfoundation Thread
				// here we set the thread-state to the correct value
				SimpleMutexEntry me(pThr->fStateMutex);
				me.Use();
				pThr->fThreadId = 0;
				pThr->IntSetState(Thread::eTerminated);
				RemoveThreadDetach(pThr);
			} else {
				SYSINFO("Thread* was NULL for [" << Thread::MyId() << "] ErrorMessage: [" << SysLog::LastSysError() << "]");
			}
			break;
		}

		// The DLL unloading due to process termination or call to FreeLibrary.
		case DLL_PROCESS_DETACH: {
			TerminateKilledThreads();
			// Release the allocated memory for this thread.
			THRKEYDELETE(MT_Storage::fgAllocatorKey);
			THRKEYDELETE(fgThreadPtrKey);
			THRKEYDELETE(Thread::fgCleanerKey);
			THRKEYDELETE(Mutex::fgCountTableKey);
			SysLog::Info("mtfoundation: DLL_PROCESS_DETACH called");
			break;
		}

		default:
			break;
	}

	return true;
	UNREFERENCED_PARAMETER(hinstDLL);
	UNREFERENCED_PARAMETER(lpvReserved);
}

#endif	// _DLL
#endif	// WIN32
