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
#include "System.h"
#include "Dbg.h"

//---- HTTPMapperTest ----------------------------------------------------------------
HTTPMapperTest::HTTPMapperTest(TString tname) : TestCase(tname)
{
}

HTTPMapperTest::~HTTPMapperTest()
{
}

void HTTPMapperTest::setUp ()
// setup connector for this TestCase
{
} // setUp

void HTTPMapperTest::tearDown ()
{
} // tearDown
void HTTPMapperTest::FDTest1()
{
	StartTrace(HTTPMapperTest.FDTest1);

	String httpOutput("<");

	OStringStream os(httpOutput);
	Anything dummy;
	Anything testInput;
	GetTestInput(dummy, "TestData");
	testInput = dummy["TestFDPost1"];
	TraceAny(testInput, "TestFDPost input: ");

	Context ctx(testInput, dummy, (Server *)0, (Session *)0, (Role *)0);
	InputMapper httpmapper("finvalstatic1");
	httpmapper.CheckConfig("InputMapper");

	t_assert(httpmapper.Get("Input", os, ctx));
	os << ">" << flush;

	Trace("httpRequest: " << httpOutput);
}

void HTTPMapperTest::FDTest2()
{
	StartTrace(HTTPMapperTest.FDTest2);

	String httpOutput("<");

	OStringStream os(httpOutput);
	Anything dummy;
	Anything testInput;
	GetTestInput(dummy, "TestData");
	testInput = dummy["TestFDPost2"];
	TraceAny(testInput, "TestFDPost input: ");

	Context ctx(testInput, dummy, (Server *)0, (Session *)0, (Role *)0);
	InputMapper httpmapper("finvalstatic2");
	httpmapper.CheckConfig("InputMapper");
	t_assert(httpmapper.Get("Input", os, ctx));
	os << ">" << flush;

	Trace("httpRequest: " << httpOutput);
}

void HTTPMapperTest::FDTest3()
{
	StartTrace(HTTPMapperTest.FDTest3);

	String httpOutput("<");

	OStringStream os(httpOutput);
	Anything dummy;
	GetTestInput(dummy, "TestData");

	Context ctx(dummy["TestFDPost3"]["env"], dummy["TestFDPost3"]["query"], (Server *)0, (Session *)0, (Role *)0);
	InputMapper httpmapper("finvalstatic3");
	httpmapper.CheckConfig("InputMapper");

	Anything tmpStore(ctx.GetTmpStore());

	// should fail because postparams is empty
	t_assert(!httpmapper.Get("Input", os, ctx));
	os << ">" << flush;

	Trace("httpRequest: " << httpOutput);
}

void HTTPMapperTest::GetTestInput(Anything &testInput, const char *testname)
{
	iostream *Ios = System::OpenStream(testname, "any");
	if ( Ios ) {
		testInput.Import((*Ios));
		delete Ios;
	}
}

void HTTPMapperTest::HTTPBodyMapperBadStream()
{
	StringStream is("Bad\nBody\n");
#if defined(WIN32) && !defined(ONLY_STD_IOSTREAM)
	is.clear(ios::failbit | ios::badbit | is.rdstate());
#else
	is.setstate(ios::failbit | ios::badbit);
#endif
	HTTPBodyResultMapper bm("testhttpbodymapper");
	Context dummyctx;
	t_assert(bm.Put("body", is, dummyctx));
	String b;
	t_assert(dummyctx.Lookup("body", b) != NULL);
}

Test *HTTPMapperTest::suite ()
// collect all test cases for the SocketStream
{
	TestSuite *testSuite = new TestSuite;

	testSuite->addTest (NEW_CASE(HTTPMapperTest, FDTest1));
	testSuite->addTest (NEW_CASE(HTTPMapperTest, FDTest2));
	testSuite->addTest (NEW_CASE(HTTPMapperTest, FDTest3));
	testSuite->addTest (NEW_CASE(HTTPMapperTest, HTTPBodyMapperBadStream));

	return testSuite;

} // suite
