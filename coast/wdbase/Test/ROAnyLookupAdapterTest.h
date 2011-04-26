/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _ROAnyLookupAdapterTest_H
#define _ROAnyLookupAdapterTest_H

#include "TestCase.h"

class ROAnyLookupAdapterTest: public TestFramework::TestCase {
public:
	ROAnyLookupAdapterTest(TString tstrName) :
		TestCaseType(tstrName) {
	}
	static Test *suite();
	void LookupTest();
};

#endif
