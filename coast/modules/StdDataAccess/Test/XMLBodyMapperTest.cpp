/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

//--- standard modules used ----------------------------------------------------
#include "Anything.h"
#include "StringStream.h"
#include "System.h"
#include "Context.h"

//--- test modules used --------------------------------------------------------
#include "TestSuite.h"

//--- module under test --------------------------------------------------------

//--- interface include --------------------------------------------------------
#include "XMLBodyMapperTest.h"

XMLBodyMapperTest::XMLBodyMapperTest(TString tname)
	: TestCaseType(tname)
	, fXMLBodyMapper("TestedMapper")
{};

XMLBodyMapperTest::~XMLBodyMapperTest() {};

void XMLBodyMapperTest::setUp ()
{
	String configFilename("XMLBodyMapperTestConfig");
	istream *ifp = System::OpenStream(configFilename, "any");

	if (ifp) {	// found
		fConfig.Import(*ifp, configFilename);
		delete ifp;
	}
	fXMLBodyMapper.Initialize("ResultMapper");
}

// Produce an Anything out of an XML
void XMLBodyMapperTest::PutTest()
{
	Anything dummy;
	Context c(dummy, fConfig, 0, 0, 0, 0);
	String input = fConfig["TeamXML"].AsString("X");
	IStringStream in(input);
	fXMLBodyMapper.DoPutStream("Output", in, c, fConfig["MapperConfig"]);
	Anything tmpStore = c.GetTmpStore();
	assertAnyEqual(fConfig["TeamAny"], tmpStore["Mapper"]["Output"]);
}

Test *XMLBodyMapperTest::suite ()
{
	TestSuite *testSuite = new TestSuite;
	ADD_CASE(testSuite, XMLBodyMapperTest, PutTest);
	return testSuite;
}
