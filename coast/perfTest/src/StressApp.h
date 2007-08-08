/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _StressApp_h_
#define _StressApp_h_

#include "config_perftest.h"
#include "Application.h"

// -- forward declaration ------------
class Anything;

//---- StressApp -----------------------------------------------------------
//! Runs a Stresser and shows the results
class EXPORTDECL_PERFTEST StressApp : public Application
{
public:
	StressApp();
	StressApp(const char *AppName);
	~StressApp();

protected:
	//! does the work
	//! Runs the configured Stresser and collects the results.
	//! \return return code to pass up to calling process
	//! \pre application is ready to be runned
	virtual int DoRun();

	//! produces a nice output from the results
	//!:!param: time - the elapsed time to run all the stressers
	//!:!prec: all started StresserThreads have finished
	virtual void ShowResult(long time);

	Anything fResult;
	friend class StressAppTest;
};

#endif
