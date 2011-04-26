/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _HTMLParserTest_H
#define _HTMLParserTest_H

#include "FoundationTestTypes.h"

class HTMLParserTest: public TestFramework::TestCase {
public:
	HTMLParserTest(TString tstrName) :
		TestCaseType(tstrName) {
	}
	static Test *suite();
	void ParseFileTest();
};

#endif
