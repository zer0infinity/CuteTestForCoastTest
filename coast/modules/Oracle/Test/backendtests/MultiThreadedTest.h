/*
 * Copyright (c) 2009, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _MultiThreadedTest_H
#define _MultiThreadedTest_H

#include "WDBaseTestPolicies.h"

class MultiThreadedTest: public TestFramework::TestCaseWithGlobalConfigDllAndModuleLoading {
public:
	MultiThreadedTest(TString tstrName) :
		TestCaseType(tstrName) {
	}
	static Test *suite();
	void DAImplTest();
	void DoTest(ROAnything roaTestConfig, const char *goodDAName, const char *failDAName);
	void Run(long id, const char *goodDAName, const char *failDAName, long lLoops, long lWait, ROAnything roaExpected);
	bool fbWasInitialized;
};

#endif
