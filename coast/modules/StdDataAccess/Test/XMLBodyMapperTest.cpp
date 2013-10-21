/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#include "XMLBodyMapperTest.h"
#include "TestSuite.h"
#include "FoundationTestTypes.h"
#include "StringStream.h"
#include "SystemFile.h"
#include "Context.h"

void XMLBodyMapperTest::setUp() {
	String configFilename("XMLBodyMapperTestConfig");
	std::istream *ifp = coast::system::OpenStream(configFilename, "any");

	if (ifp) { // found
		fConfig.Import(*ifp, configFilename);
		delete ifp;
	}
	fXMLBodyMapper.Initialize("ResultMapper");
}

// Produce an Anything out of an XML
void XMLBodyMapperTest::PutTest() {
	Anything dummy;
	Context c(dummy, fConfig, 0, 0, 0, 0);
	String input = fConfig["TeamXML"].AsString("X");
	IStringStream in(input);
	fXMLBodyMapper.Put("Output", in, c);
	assertAnyEqual(fConfig["TeamAny"], c.Lookup("Mapper.Output"));
}

Test *XMLBodyMapperTest::suite() {
	TestSuite *testSuite = new TestSuite;
	ADD_CASE(testSuite, XMLBodyMapperTest, PutTest);
	return testSuite;
}
