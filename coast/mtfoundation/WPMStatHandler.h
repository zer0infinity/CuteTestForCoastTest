/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _WPMStatHandler_H
#define _WPMStatHandler_H

#include "StatUtils.h"
#include "DiffTimer.h"
#include "Threads.h"

//!gather statistical information about a worker pool
class WPMStatHandler: public StatEvtHandler
{
public:
	WPMStatHandler(long poolSize);

	enum EWPMStatEvt { eEnter, eLeave };

	/*! set new PoolSize, currently it is only used when printing statistics
		\param lNewPoolSize new size of Pool for printing statistics */
	void setPoolSize(long lNewPoolSize) {
		LockUnlockEntry me(fMutex);
		fPoolSize = lNewPoolSize;
	}
	/*! get current PoolSize
		\return current PoolSize */
	long getPoolSize() {
		LockUnlockEntry me(fMutex);
		return fPoolSize;
	}
	/*! atomically increment current PoolSize
		\return new PoolSize */
	long incrementPoolSize() {
		LockUnlockEntry me(fMutex);
		return ++fPoolSize;
	}

protected:
	//!gathering statistics for event evt
	void DoHandleStatEvt(long evt);
	//!collect the gathered statistics
	void DoStatistic(Anything &statElements);
	//!get the number of requests processed so far
	long DoGetTotalRequests();
	//!get the number of currently active requests
	long DoGetCurrentParallelRequests();

private:
	//! number of allowed request threads that can run in parallel
	long fPoolSize;
	//! number of currently running requests
	long fMaxParallelRequests;
	//! number of currently running requests
	long fCurrentParallelRequests;
	//! number of total requests serviced
	ul_long fTotalRequests;
	//! number of ms used to service the fTotalRequests
	double fTotalTime;
	//!timer to measure elapsed time during processing of requests
	DiffTimer fTimer;
	//!guard for setting values
	SimpleMutex fMutex;

	friend class WPMStatHandlerTest;
};

#endif
