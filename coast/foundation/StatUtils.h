/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _STATUTILS_H
#define _STATUTILS_H

#include "config_foundation.h"	// for definition of EXPORTDECL_FOUNDATION
class EXPORTDECL_FOUNDATION String;
class EXPORTDECL_FOUNDATION Anything;

//--- StatEvtHandler -----------------------------------------------
//!event handler for gathering statistic data
class EXPORTDECL_FOUNDATION StatEvtHandler
{
public:
	StatEvtHandler();
	virtual ~StatEvtHandler();

	//! gathering statistics for event evt
	void HandleStatEvt(long evt);
	//! collect the gathered statistics
	void Statistic(Anything &statElements);
	//! get number of requests made since start
	long GetTotalRequests();
	//! get the number of currently active requests
	long GetCurrentParallelRequests();

protected:
	//! methods to override in concrete implementation
	//! gathering statistics for event evt
	virtual void DoHandleStatEvt(long evt) = 0;
	//! collect the gathered statistics
	virtual void DoStatistic(Anything &statElements) = 0;
	//! get number of requests made since start
	virtual long DoGetTotalRequests() = 0;
	//! get the number of currently active requests
	virtual long DoGetCurrentParallelRequests() = 0;

private:
	StatEvtHandler(const StatEvtHandler &);
	StatEvtHandler &operator=(const StatEvtHandler &);
};

//--- StatGatherer -----------------------------------------------

//!gathering statistic data
class EXPORTDECL_FOUNDATION StatGatherer
{
public:
	StatGatherer();
	virtual ~StatGatherer();

	/*! get collected statistics
		\param statistics Anything to get statistics data */
	void Statistic(Anything &statElements);

	/*! prints collected pool statistics on stderr */
	void PrintStatisticsOnStderr( const String &strName );

protected:
	//! collect the statistics in using class
	virtual void DoGetStatistic(Anything &statElements) = 0;

private:
	StatGatherer(const StatGatherer &);
	StatGatherer &operator=(const StatGatherer &);
};

//--- StatObserver -----------------------------------------------

//!observing statistic data
class EXPORTDECL_FOUNDATION StatObserver
{
public:
	StatObserver();
	virtual ~StatObserver();

	//! register
	void Register(const String &name, StatGatherer *pGatherer);

protected:
	//! register
	virtual void DoRegisterGatherer(const String &name, StatGatherer *) = 0;

private:
	StatObserver(const StatObserver &);
	StatObserver &operator=(const StatObserver &);
};

#endif		//ifndef _STATUTILS_H
