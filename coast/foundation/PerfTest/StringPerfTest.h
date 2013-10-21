/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _StringPerfTest_H
#define _StringPerfTest_H

#include "FoundationTestTypes.h"//lint !e537
class StringPerfTest: public testframework::TestCaseWithStatistics {
public:
	StringPerfTest(TString tstrName) :
		TestCaseType(tstrName) {
	}
	static Test *suite();
	void referenceTest();
protected:
	void RunLoop(const char *str, const long iterations);
	void RunPreallocLoop(const char *str, const long iterations);
	void RunPoolAllocLoop(const char *str, const long iterations);
};

#endif
