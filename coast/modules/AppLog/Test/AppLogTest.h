/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _AppLogTest_H
#define _AppLogTest_H

//---- baseclass include -------------------------------------------------
#include "ConfiguredTestCase.h"

class Context;

//---- AppLogTest ----------------------------------------------------------
//!TestCases description
class AppLogTest : public ConfiguredTestCase
{
public:
	//--- constructors

	//!TestCase constructor
	//! \param name name of the test
	AppLogTest(TString name);

	//!destroys the test case
	~AppLogTest();

	//--- public api

	//!builds up a suite of testcases for this test
	static Test *suite ();

	//!log into two channels and checks the files afterwards
	void LogOkTest();
	//!log into channels of 'virtual' server without own config but using TestServer config
	void LogOkToVirtualServerTest();
	//!pass null pointer a channel
	void ApplogModuleNotInitializedTest();
	//!test the logging action
	void LoggingActionTest();
	//!test the time logging action
	void TimeLoggingActionTest();

protected:
	//! utility method to check the log files
	void CheckFile(Context &ctx, const char *channelname, String expected, bool bExpectSuccess = true);
};

#endif
