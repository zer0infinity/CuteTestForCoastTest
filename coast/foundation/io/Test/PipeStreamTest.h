/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _PipeStreamTest_H
#define _PipeStreamTest_H

#include "TestCase.h"//lint !e537
class PipeStreamTest: public testframework::TestCase {
public:
	PipeStreamTest(TString tstrName) :
		TestCaseType(tstrName) {
	}
	static Test *suite();
	void SimpleWriteandRead();
	void MoreWriteandRead();
	void LoopWriteandRead();
	void PipeAndFork();
};

#endif
