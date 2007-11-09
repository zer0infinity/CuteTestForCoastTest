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
void WorkerPoolManager::Enter( WorkerParamType workload, long lFindWorkerHint )
{
	// guard the entry to request handling
	// we're doing flow control on the main thread
	// causing it to wait for a request thread to
	// be available

	LockUnlockEntry me(fMutex);
	StartTrace(WorkerPoolManager.Enter);

	while ((fPoolSize <= fCurrentParallelRequests) || fBlockRequestHandling) {
		fCond.Wait( fMutex );		// release mutex and wait for condition
	}

	Assert(fPoolSize > fCurrentParallelRequests);
	Trace("I slipped past the Critical Region and there are " << fCurrentParallelRequests << " requests in work");

	// use the lFindWorkerHint param as hint to get the next free WorkerThread
	if ( lFindWorkerHint < 0 ) {
		lFindWorkerHint = ( fCurrentParallelRequests + 1L );
	}

	// find a worker object that can run this request
	WorkerThread *hr = FindNextRunnable(lFindWorkerHint);

	hr->SetWorking(workload);
}

#endif
