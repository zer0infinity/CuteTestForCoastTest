/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

//--- interface include --------------------------------------------------------
#include "HTTPMapperTest.h"

//--- module under test --------------------------------------------------------
#include "HTTPMapper.h"

//--- test modules used --------------------------------------------------------
#include "TestSuite.h"

//--- standard modules used ----------------------------------------------------
#include "StringStream.h"
#include "SystemFile.h"
#include "Dbg.h"

//---- HTTPMapperTest ----------------------------------------------------------------
HTTPMapperTest::HTTPMapperTest(TString tname) : TestCaseType(tname)
{
}

HTTPMapperTest::~HTTPMapperTest()
{
}

void HTTPMapperTest::FDTest1()
{
	StartTrace(HTTPMapperTest.FDTest1);

	String httpOutput;

	OStringStream os(httpOutput);
	Anything dummy;
	Anything testInput;
	GetTestInput(dummy, "TestData");
	testInput = dummy["TestFDPost1"];
	TraceAny(testInput, "TestFDPost1 input: ");

	Context ctx(testInput, dummy, (Server *)0, (Session *)0, (Role *)0);
	ParameterMapper httpmapper("finvalstatic1");
	httpmapper.Initialize("ParameterMapper");
	t_assert(httpmapper.Get("Input", os, ctx));

	String result;
	result = PrepareResults(dummy["TestFDPost1Result"]);
	assertEqual(result, httpOutput);
	os << std::flush;
	Trace("Resulting httpRequest: " << httpOutput);
	Trace("Expected  httpRequest: " << result);
}

void HTTPMapperTest::FDTest2()
{
	StartTrace(HTTPMapperTest.FDTest2);

	String httpOutput;

	OStringStream os(httpOutput);
	Anything dummy;
	Anything testInput;
	GetTestInput(dummy, "TestData");
	testInput = dummy["TestFDPost2"];
	TraceAny(testInput, "TestFDPost2 input: ");

	Context ctx(testInput, dummy, (Server *)0, (Session *)0, (Role *)0);
	ParameterMapper httpmapper("finvalstatic2");
	httpmapper.Initialize("ParameterMapper");
	t_assert(httpmapper.Get("Input", os, ctx));

	String result;
	result = PrepareResults(dummy["TestFDPost2Result"]);
	assertEqual(result, httpOutput);
	os << std::flush;
	Trace("Resulting httpRequest: " << httpOutput);
	Trace("Expected  httpRequest: " << result);
}

void HTTPMapperTest::FDTest3()
{
	StartTrace(HTTPMapperTest.FDTest3);

	String httpOutput;

	OStringStream os(httpOutput);
	Anything dummy;
	GetTestInput(dummy, "TestData");

	Context ctx(dummy["TestFDPost3"]["env"], dummy["TestFDPost3"]["query"], (Server *)0, (Session *)0, (Role *)0);
	ParameterMapper httpmapper("finvalstatic3");
	httpmapper.Initialize("ParameterMapper");

	Anything tmpStore(ctx.GetTmpStore());

	// should fail because postparams is empty
	t_assert(!httpmapper.Get("Input", os, ctx));
}

void HTTPMapperTest::FDTest4()
{
	StartTrace(HTTPMapperTest.FDTest4);

	String httpOutput;

	OStringStream os(httpOutput);
	Anything dummy;
	GetTestInput(dummy, "TestData");

	Context ctx(dummy["TestFDGet4"]["env"], dummy["TestFDGet4"]["query"], (Server *)0, (Session *)0, (Role *)0);
	ParameterMapper httpmapper("fdtest4");
	httpmapper.Initialize("ParameterMapper");

	Anything tmpStore(ctx.GetTmpStore());

	t_assert(httpmapper.Get("Input", os, ctx));

	String result;
	result = PrepareResults(dummy["TestFDGet4Result"]);
	assertEqual(result, httpOutput);
	os << std::flush;
	Trace("Resulting httpRequest: " << httpOutput);
	Trace("Expected  httpRequest: " << result);

}

void HTTPMapperTest::GetTestInput(Anything &testInput, const char *testname)
{
	std::iostream *Ios = Coast::System::OpenStream(testname, "any");
	if ( Ios ) {
		testInput.Import((*Ios));
		delete Ios;
	}
}

String HTTPMapperTest::PrepareResults(ROAnything resultsAsAny)
{
	String results;
	long l;
	for ( l = 0; l < resultsAsAny.GetSize() - 1; l ++ ) {
		results << resultsAsAny[l].AsString() << "\r\n";
	}
	results << resultsAsAny[l].AsString();
	return results;
}

void HTTPMapperTest::HTTPBodyMapperBadStream()
{
	String strBuf("Bad\nBody\n");
	StringStream is(strBuf);
	is.setstate(std::ios::failbit | std::ios::badbit);
	HTTPBodyResultMapper bm("testhttpbodymapper");
	bm.Initialize("ResultMapper");
	Context dummyctx;
	t_assert(bm.Put("body", is, dummyctx));
	String b;
	t_assert(dummyctx.Lookup("body", b) != NULL);
}

Test *HTTPMapperTest::suite ()
{
	TestSuite *testSuite = new TestSuite;

	ADD_CASE(testSuite, HTTPMapperTest, FDTest1);
	ADD_CASE(testSuite, HTTPMapperTest, FDTest2);
	ADD_CASE(testSuite, HTTPMapperTest, FDTest3);
	ADD_CASE(testSuite, HTTPMapperTest, FDTest4);
	ADD_CASE(testSuite, HTTPMapperTest, HTTPBodyMapperBadStream);

	return testSuite;

}
