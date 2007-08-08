/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _TestRunnerApplication_h_
#define _TestRunnerApplication_h_

#include "Application.h"

//---- TestRunnerApplication -----------------------------------------------------------
//! comment
class TestRunnerApplication : public Application
{
public:
	TestRunnerApplication();
	TestRunnerApplication(const char *AppName);
	~TestRunnerApplication();

protected:
	//! does the work
	//! Runs the application
	//! \return return code to pass up to calling process
	//! \pre application is ready to be runned
	virtual int DoRun();

	// GlobalInit: keeps argc and argv for later use
	virtual int DoGlobalInit(int argc, char *argv[], const ROAnything config);

	int fgArgc;
	char **fgArgv;
};

#endif
