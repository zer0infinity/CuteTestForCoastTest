/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _ServiceDispatcherTest_H
#define _ServiceDispatcherTest_H

#include "WDBaseTestPolicies.h"

class ServiceDispatcherTest: public TestFramework::TestCaseWithGlobalConfigDllAndModuleLoading {
public:
	ServiceDispatcherTest(TString tstrName);
	~ServiceDispatcherTest();
	static Test *suite();
	void setUp();
	void FindTests();
	void ServiceDispatcherModuleTest();
};

#endif
