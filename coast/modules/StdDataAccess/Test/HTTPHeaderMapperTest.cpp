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
#include "Context.h"
#include "Dbg.h"

//--- test modules used --------------------------------------------------------
#include "TestSuite.h"

//--- module under test --------------------------------------------------------
#include "HTTPMapper.h"

//--- interface include --------------------------------------------------------
#include "HTTPHeaderMapperTest.h"

//---- HTTPHeaderMapperTest ----------------------------------------------------------------
HTTPHeaderMapperTest::HTTPHeaderMapperTest(TString tname) : TestCase(tname)
{
}

HTTPHeaderMapperTest::~HTTPHeaderMapperTest()
{
}

void HTTPHeaderMapperTest::setUp ()
// setup connector for this TestCase
{
}

void HTTPHeaderMapperTest::tearDown ()
{
}

void HTTPHeaderMapperTest::GetHeaderSuppressTest()
{
	StartTrace(HTTPHeaderMapperTest.GetHeaderSuppressTest);

	Anything inputArgs, headerFields;

	headerFields["testcase"] = "foobah";
	headerFields["test1"]   = "test1";
	headerFields["test2"]   = "test2";
	headerFields["Test3"]   = "test3";
	headerFields["Content-Length"] = 0;

	inputArgs["Header"] = headerFields;

	Context ctx(inputArgs, inputArgs, (Server *)0, (Session *)0, (Role *)0);

	ParameterMapper http("HeaderSuppressTestMapper");
	http.CheckConfig("ParameterMapper");

	String input("<");
	OStringStream os(&input);

	t_assert(http.Get("Header", os, ctx));
	os << flush;
	input << ">";
	Trace("Input: " << input);
	assertEqual("<testcase: foobah\r\nContent-Length: 0\r\n>", input);
}

void HTTPHeaderMapperTest::PutHeaderSuppressTest()
{
	StartTrace(HTTPHeaderMapperTest.PutHeaderSuppressTest);
	String httpOutput("HTTP/1.1 200 Ok\r\ntest1: test1\r\ntest2: test2\r\nTest3: test3\r\nContent-Type: text/html\r\n\r\n<html><body></body></html>");
	IStringStream is(httpOutput);
	Anything dummy;
	Context ctx(dummy, dummy, (Server *)0, (Session *)0, (Role *)0);
	ResultMapper http("HeaderSuppressTestMapper");
	http.CheckConfig("ResultMapper");

	Anything tmpStore(ctx.GetTmpStore());

	t_assert(http.Put("Header", is, ctx));

	TraceAny(tmpStore, "tmpStore: ");

	assertEqual("HTTP/1.1", tmpStore["Mapper"]["HTTPHeader"]["HTTP-Version"].AsString());
	assertEqual(200, tmpStore["Mapper"]["HTTPHeader"]["ResponseCode"].AsLong());
	assertEqual("Ok", tmpStore["Mapper"]["HTTPHeader"]["ResponseMsg"].AsString());
	assertEqual("text/html", tmpStore["Mapper"]["HTTPHeader"]["content-type"].AsString());
	assertEqual(false, tmpStore["Mapper"].IsDefined("test1"));
	assertEqual(false, tmpStore["Mapper"].IsDefined("test2"));
	assertEqual(false, tmpStore["Mapper"].IsDefined("test3"));
}

Test *HTTPHeaderMapperTest::suite ()
// collect all test cases for the SocketStream
{
	TestSuite *testSuite = new TestSuite;

	testSuite->addTest (NEW_CASE(HTTPHeaderMapperTest, GetHeaderSuppressTest));
	testSuite->addTest (NEW_CASE(HTTPHeaderMapperTest, PutHeaderSuppressTest));

	return testSuite;
} // suite
