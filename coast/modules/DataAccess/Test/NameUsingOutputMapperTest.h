/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _NameUsingOutputMapperTest_H
#define _NameUsingOutputMapperTest_H

#include "TestCase.h"

class ROAnything;
class Context;
class ResultMapper;
class NameUsingOutputMapperTest: public testframework::TestCase {
public:
	NameUsingOutputMapperTest(TString tstrName) :
		TestCaseType(tstrName) {
	}
	static Test *suite();
	void ConfiguredDestinationTest();
	void NonConfiguredDestinationTest();
protected:
	void DoPut(ResultMapper &mapper, Context &c);
	void DoCheck(const ROAnything &result);
};

#endif
