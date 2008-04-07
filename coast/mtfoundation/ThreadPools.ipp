/*
 * Copyright (c) 2007, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _ThreadPools_IPP
#define _ThreadPools_IPP

//--- project modules used -----------------------------------------------------

//--- standard modules used ----------------------------------------------------

//--- c-modules used -----------------------------------------------------------

template< class WorkerParamType >
bool WorkerPoolManager::Enter( WorkerParamType workload, long lFindWorkerHint )
{
	// guard the entry to request handling
	// we're doing flow control on the main thread
	// causing it to wait for a request thread to
	// be available
	LockUnlockEntry me(fMutex);
	StartTrace1(WorkerPoolManager.Enter, "hint: " << lFindWorkerHint);
	bool bEnterSuccess( false );
	// find a worker object that can run this request
	WorkerThread *hr( FindNextRunnable( lFindWorkerHint ) );
	if ( hr != NULL ) {
		bEnterSuccess = hr->SetWorking(workload);
	}
	return bEnterSuccess;
}

#endif
