/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _SimpleDAServiceTest_H
#define _SimpleDAServiceTest_H

#include "WDBaseTestPolicies.h"

//!TestCases description
class SimpleDAServiceTest: public TestFramework::TestCaseWithGlobalConfigDllAndModuleLoading {
public:
	SimpleDAServiceTest(TString tname) :
		TestCaseType(tname) {
	}
	//!builds up a suite of testcases for this test
	static Test *suite();
	//!use the ServiceDispatcher to obtain a simple service
	void SimpleDispatch();
	//! try to use it with a very simple DataAccess backend
	void SimpleServiceCall();
	//! try to use it with a missing DataAccess backend
	void FailedServiceCall();
};

#endif
