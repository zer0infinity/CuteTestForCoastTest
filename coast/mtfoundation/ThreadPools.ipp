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
	long lCurrRequests( fpStatEvtHandler->GetCurrentParallelRequests() );
	while ( ( fPoolSize <= lCurrRequests ) || fBlockRequestHandling ) {
		fCond.TimedWait( fMutex, 0, 1000 );		// release mutex and wait for condition
		lCurrRequests = fpStatEvtHandler->GetCurrentParallelRequests();
	}

	Assert( fPoolSize > lCurrRequests );
	Trace("I slipped past the Critical Region and there are " << lCurrRequests << " requests in work");

	// use the lFindWorkerHint param as hint to get the next free WorkerThread
	if ( lFindWorkerHint < 0 ) {
		lFindWorkerHint = ( lCurrRequests + 1L ) % fPoolSize;
	} else {
		lFindWorkerHint = lFindWorkerHint % fPoolSize;
	}
	bool bEnterSuccess( false );
	// find a worker object that can run this request
	WorkerThread *hr = FindNextRunnable( lFindWorkerHint );
	if ( hr ) {
		bEnterSuccess = hr->SetWorking(workload);
	}
	return bEnterSuccess;
}

#endif
