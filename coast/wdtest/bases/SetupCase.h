/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _SetupCase_H
#define _SetupCase_H

//---- baseclass include -------------------------------------------------
#include "ConfiguredTestCase.h"
class Server;

//---- SetupCase ----------------------------------------------------------
//! No tests - but prepares the Modules and may initalize a Server
//! Loads to config files : Config.any and TestConfig.any and uses both to install modules.
//! Config.any gets installed first, therefore TestConfig may redefine some registered objects.
class SetupCase : public ConfiguredTestCase
{
public:
	//!TestCase constructor
	//! \param name name of the test
	SetupCase(TString tstrName);

	//!destroys the test case
	~SetupCase();

	//--- public api

	//!builds up a suite of testcases for this test
	static Test *suite ();

	//! If TestConfig.any contains a slot /InitServer, this case initializes the Server named by it.
	void dummy();

protected:
	//! The tests Configuration
	Anything fTestConfig;
	//! The 'real project' Configuration
	Anything fMainConfig;
	Server *fServer;
};

#endif
