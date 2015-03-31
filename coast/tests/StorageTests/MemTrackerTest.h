/*
 * Copyright (c) 2007, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _MemTrackerTest_H
#define _MemTrackerTest_H

#include "TestCase.h"

class MemTrackerTest: public testframework::TestCase {
public:
	MemTrackerTest(TString tstrName);
	static Test *suite();
	void TrackAllocFreeTest();
};

#endif
