/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _WPMStatHandler_H
#define _WPMStatHandler_H

//---- baseclass include -------------------------------------------------
#include "config_mtfoundation.h"
#include "StatUtils.h"

//---- forward declaration -----------------------------------------------
#include "DiffTimer.h"
#include "Threads.h"

//---- WPMStatHandler ----------------------------------------------------------
//!gather statistical information about a worker pool
class EXPORTDECL_MTFOUNDATION WPMStatHandler: public StatEvtHandler
{
public:
	WPMStatHandler(long poolSize);
	~WPMStatHandler();

	enum EWPMStatEvt { eEnter, eLeave };

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
	ul_long fTotalTime;
	//!timer to measure elapsed time during processing of requests
	DiffTimer fTimer;
	//!guard for setting values
	SimpleMutex fMutex;

	friend class WPMStatHandlerTest;
};

#endif
