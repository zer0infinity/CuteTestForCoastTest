/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _SysLogTest_H
#define _SysLogTest_H

#include "TestCase.h"//lint !e537

class SysLogTest: public TestFramework::TestCase {
protected:
	void setUp();
public:
	SysLogTest(TString tstrName);
	~SysLogTest();
	static Test *suite();
	void tearDown();
	void TestFlags();
};

#endif
