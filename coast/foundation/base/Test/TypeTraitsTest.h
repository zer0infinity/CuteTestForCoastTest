/*
 * Copyright (c) 2006, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _TypeTraitsTest_H
#define _TypeTraitsTest_H

#include "TestCase.h"//lint !e537
class TypeTraitsTest: public TestFramework::TestCase {
public:
	TypeTraitsTest(TString tstrName) :
		TestCaseType(tstrName) {
	}
    static Test *suite();
	void TraitsTest();
	void ClassTraitsTest();
};

#endif
