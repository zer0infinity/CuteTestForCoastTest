/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _XMLBodyMapperTest_h_
#define _XMLBodyMapperTest_h_

#include "TestCase.h"
#include "XMLBodyMapper.h"

class Anything;

class XMLBodyMapperTest: public testframework::TestCase {
public:
	XMLBodyMapperTest(TString tstrName) :
		TestCaseType(tstrName), fXMLBodyMapper("TestedMapper") {
	}
	void setUp();
	static Test *suite();

	// Produce an Anything out of an XML
	void PutTest();

protected:
	Anything fConfig;
	XMLBodyMapper fXMLBodyMapper;
};

#endif
