/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _AppLogTest_H
#define _AppLogTest_H

#include "WDBaseTestPolicies.h"
class Context;

class AppLogTest: public TestFramework::TestCaseWithCaseConfigDllAndModuleLoading {
public:
	AppLogTest(TString tstrName) :
		TestCaseType(tstrName) {
	}
	//!builds up a suite of testcases for this test
	static Test *suite();

	//!log into two channels and checks the files afterwards
	void LogOkTest();
	//!log into one channel and check the file after AppLog module is terminated
	void BufferItemsTest();
	//!log into channels of 'virtual' server without own config but using TestServer config
	void LogOkToVirtualServerTest();
	//! test logfile rotation using absolute seconds
	void LogRotatorLocalTimeTest();
	//! test logfile rotation using absolute seconds, using GMT
	void LogRotatorGmtTest();
	//! test logfile rotation that happend every nth intervall
	void LogRotationEveryNSecondsTest();
	//! test logfile rotation using RotateTime
	void LogRotationTimeTest();
	//! Request log rotation for a channel
	void RotateSpecificLogTest();
	//!pass null pointer a channel
	void ApplogModuleNotInitializedTest();
	//!test the logging action
	void LoggingActionTest();
	//!test the time logging action
	void TimeLoggingActionTest();

protected:
	//! utility method to check the log files
	void CheckFile(Context &ctx, const char *channelname, String expected, bool bExpectSuccess = true);
	void CheckFileAfterChannelTermination(Context &ctx, const char *strLogFilename, String expected, bool bExpectSuccess = true);
	void LogRotatorTestsCommon();
};

#endif
