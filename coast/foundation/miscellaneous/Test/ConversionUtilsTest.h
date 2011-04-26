/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _ConversionUtilsTest_H
#define _ConversionUtilsTest_H

#include "TestCase.h"//lint !e537
class ConversionUtilsTest: public TestFramework::TestCase {
public:
	ConversionUtilsTest(TString tstrName) :
		TestCaseType(tstrName) {
	}
	static Test *suite();
	void GetValueFromBufferTest();
};

#endif
