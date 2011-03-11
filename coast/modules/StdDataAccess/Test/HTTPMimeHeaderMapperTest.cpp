/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

//--- interface include --------------------------------------------------------
#include "HTTPMimeHeaderMapperTest.h"

//--- module under test --------------------------------------------------------
#include "HTTPMimeHeaderMapper.h"
//--- test modules used --------------------------------------------------------
#include "TestSuite.h"

//--- standard modules used ----------------------------------------------------
#include "Dbg.h"
#include "StringStream.h"

void HTTPMimeHeaderMapperTest::ConfiguredTests()
{
	StartTrace(HTTPMimeHeaderMapperTest.ConfiguredTests);
	ROAnything caseConfig;
	AnyExtensions::Iterator<ROAnything, ROAnything, TString> aEntryIterator(GetTestCaseConfig());
	while (aEntryIterator.Next(caseConfig)) {
		TString caseName;
		if ( !aEntryIterator.SlotName(caseName) ) {
			caseName << "At index: " << aEntryIterator.Index();
		}
		String strIn = caseConfig["Input"].AsString();
		IStringStream is(strIn);
		HTTPMimeHeaderMapper m(caseConfig["MapperName"].AsString("mapper name missing"));
		m.Initialize("ResultMapper");
		Context ctx;
		t_assertm(((ResultMapper &)m).Put("", is, ctx), caseName);
		assertAnyEqualm(caseConfig["Expected"], ctx.GetTmpStore()["Mapper"]["HTTPHeader"], caseName);
	}
}

void HTTPMimeHeaderMapperTest::CorrectedDateHeadersTest()
{
	StartTrace(HTTPMimeHeaderMapperTest.CorrectedDateHeadersTest);
	Anything dateformats2correct(GetTestCaseConfig()["Input"].DeepClone());
	HTTPMimeHeaderMapper::CorrectDateFormats(dateformats2correct);
	assertAnyEqual(GetTestCaseConfig()["Expected"], dateformats2correct);
}

// builds up a suite of tests, add a line for each testmethod
Test *HTTPMimeHeaderMapperTest::suite ()
{
	StartTrace(HTTPMimeHeaderMapperTest.suite);
	TestSuite *testSuite = new TestSuite;
	ADD_CASE(testSuite, HTTPMimeHeaderMapperTest, ConfiguredTests);
	ADD_CASE(testSuite, HTTPMimeHeaderMapperTest, CorrectedDateHeadersTest);
	return testSuite;
}
