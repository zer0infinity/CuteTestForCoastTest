/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _ConfigMapperTest_H
#define _ConfigMapperTest_H

#include "WDBaseTestPolicies.h"

class ConfigMapperTest: public TestFramework::TestCaseWithGlobalConfigDllAndModuleLoading {
public:
	ConfigMapperTest(TString tstrName) :
		TestCaseType(tstrName) {
	}
	//! builds up a suite of tests
	static Test *suite();
	TString getConfigFileName() {
		return "ConfigMapperTestConfig";
	}
	//! testing config mapper
	void ConfigTest();
};

#endif
