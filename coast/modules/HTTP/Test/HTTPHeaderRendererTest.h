/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _HTTPHeaderRendererTest_H
#define _HTTPHeaderRendererTest_H

#include "TestCase.h"

class HTTPHeaderRendererTest: public testframework::TestCase {
public:
	HTTPHeaderRendererTest(TString tname) :
		TestCaseType(tname) {
	}
	static Test *suite();
	void SingleLine();
	void SingleLineMultiValue();
	void MultiLine();
	void WholeHeaderConfig();
	void Issue299MissingFilenamePrefix();
};

#endif
