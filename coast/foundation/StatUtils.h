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

	//!gathering statistics for event evt
	virtual void HandleStatEvt(long evt) = 0;
	//!collect the gathered statistics
	virtual void Statistic(Anything &statElements) = 0;

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

	//!collect the statistics
	virtual void Statistic(Anything &statElements) = 0;

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

	//!collect the statistics
	virtual void Register(const String &name, StatGatherer *) = 0;

private:
	StatObserver(const StatObserver &);
	StatObserver &operator=(const StatObserver &);
};

#endif		//ifndef _STATUTILS_H
