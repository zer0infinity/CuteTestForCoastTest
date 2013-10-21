/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _ObjectListTest_H
#define _ObjectListTest_H

#include "TestCase.h"

class ObjectListTest: public testframework::TestCase {
public:
	ObjectListTest(TString tstrName) :
		TestCaseType(tstrName) {
	}
	static Test *suite();
	void CtorTest();
	void DtorTest();
};

#endif
