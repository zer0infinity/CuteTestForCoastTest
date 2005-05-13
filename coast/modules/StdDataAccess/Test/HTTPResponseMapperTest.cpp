/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

//--- interface include --------------------------------------------------------
#include "HTTPResponseMapperTest.h"

//--- test modules used --------------------------------------------------------
#include "TestSuite.h"

//--- module under test --------------------------------------------------------
#include "HTTPResponseMapper.h"
//--- standard modules used ----------------------------------------------------
#include "Dbg.h"
#include "StringStream.h"
#include "Context.h"
//--- c-modules used -----------------------------------------------------------

//---- HTTPResponseMapperTest ----------------------------------------------------------------
HTTPResponseMapperTest::HTTPResponseMapperTest(TString name) : TestCase(name)
{
	StartTrace(HTTPResponseMapperTest.Ctor);
}

HTTPResponseMapperTest::~HTTPResponseMapperTest()
{
	StartTrace(HTTPResponseMapperTest.Dtor);
}

// setup for this TestCase
void HTTPResponseMapperTest::setUp ()
{
	StartTrace(HTTPResponseMapperTest.setUp);
}

void HTTPResponseMapperTest::tearDown ()
{
	StartTrace(HTTPResponseMapperTest.tearDown);
}

void HTTPResponseMapperTest::TestParsedResponse()
{
	StartTrace(HTTPResponseMapperTest.TestParsedResponse);
	IStringStream is("HTTP/1.1 200 Ok\r\n");
	HTTPResponseMapper m("HTTPResponseMapper");
	m.CheckConfig("ResultMapper");
	Context ctx;
	t_assert(m.Put("", is, ctx));
	Anything result(ctx.GetTmpStore()["Mapper"]["HTTPResponse"]);
	assertEqual(200L, result["Code"].AsLong(0));
	assertEqual("Ok", result["Msg"].AsCharPtr());
	assertEqual("HTTP/1.1", result["HTTP-Version"].AsCharPtr());
}
void HTTPResponseMapperTest::TestBadResponseLine()
{
	StartTrace(HTTPResponseMapperTest.TestBadResponseLine);
	IStringStream is("HTTP/1.1 ");
	HTTPResponseMapper m("HTTPResponseMapper");
	m.CheckConfig("ResultMapper");
	Context ctx;
	t_assert(!m.Put("", is, ctx));
	Anything result(ctx.GetTmpStore()["Mapper"]["HTTPResponse"]);
	assertEqual(-1L, result["Code"].AsLong(-1));
	assertEqual("undefined", result["Msg"].AsCharPtr("undefined"));
	assertEqual("HTTP/1.1", result["HTTP-Version"].AsCharPtr());
}

// builds up a suite of testcases, add a line for each testmethod
Test *HTTPResponseMapperTest::suite ()
{
	StartTrace(HTTPResponseMapperTest.suite);
	TestSuite *testSuite = new TestSuite;

	ADD_CASE(testSuite, HTTPResponseMapperTest, TestParsedResponse);
	ADD_CASE(testSuite, HTTPResponseMapperTest, TestBadResponseLine);

	return testSuite;
}
