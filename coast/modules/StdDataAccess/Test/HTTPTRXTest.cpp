/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

//--- standard modules used ----------------------------------------------------
#include "Anything.h"
#include "Threads.h"
#include "HTTPDAImpl.h"
#include "Context.h"
#include "Mapper.h"
#include "Server.h"
#include "Session.h"
#include "Role.h"
#include "SSLModule.h"
#include "Dbg.h"

//--- test modules used --------------------------------------------------------
#include "TestSuite.h"

//--- module under test --------------------------------------------------------
#include "HTTPTRX.h"

//--- interface include --------------------------------------------------------
#include "HTTPTRXTest.h"

//---- HTTPTRXTest ----------------------------------------------------------------
HTTPTRXTest::HTTPTRXTest(TString tname) : TestCase(tname)
{
}

HTTPTRXTest::~HTTPTRXTest()
{
}

void HTTPTRXTest::setUp ()
// setup connector for this TestCase
{
} // setUp

void HTTPTRXTest::tearDown ()
{
} // tearDown

void HTTPTRXTest::fdPutArgsTest()
{
	Anything params;
	Anything input;
	Anything dummy;

	params["URI_PARAM"] = "/finVAL/XS.ISA?Action=Login";
	params["env"]["header"]["testcase"] = "foobah";
	input["Data"] = params;

	HTTPTRX impl("fdputargs");
	InputMapper	inpMapper("datainput");

	Context ctx(input, dummy, (Server *)0, (Session *)0, (Role *)0);
	impl.PutArgs(ctx, &inpMapper);

	String act("<");
	act.Append(impl.fData2Send.AsCharPtr("")).Append(">");
	assertEqual("<GET /finVAL/XS.ISA?Action=Login HTTP/1.0\r\ntestcase: foobah\r\nContent-length: 0\r\n\r\n>", act);

	Anything params2;
	params2["URI_PARAM"] = "/finVAL/XS.ISA";
	params2["query"]["postparams"] = "Action=Login+Shutdown";
	params2["env"]["header"]["testcase"] = "foobah";
	input["Data"] = params2;

	HTTPTRX impl2("fdputargs2");
	InputMapper	inpMapper2("anotherinput");

	impl2.PutArgs(ctx, &inpMapper2);

	String act2(impl2.fData2Send.AsCharPtr(""));
	assertEqual("POST /finVAL/XS.ISA HTTP/1.0\r\ntestcase: foobah\r\nContent-length: 21\r\n\r\nAction=Login+Shutdown", act2);
}

void HTTPTRXTest::webAppPutArgsTest()
{
	Anything params;
	Anything input;
	Anything dummy;

	params["pid"] = "t028307";
	params["password"] = "foo bah";
	input["Data"] = params;

	HTTPTRX impl("webappputargs");
	InputMapper	inpMapper("webappputargs");

	Context ctx(input, dummy, (Server *)0, (Session *)0, (Role *)0);
	impl.PutArgs(ctx, &inpMapper);

	String act("<");
	act.Append(impl.fData2Send.AsCharPtr("")).Append(">");
	assertEqual("<GET /udb/udbdemo?cmd=ModUser&pid=t028307&password=foo%20bah HTTP/1.0\r\nAuthorization: Basic AAAAAA==\r\nContent-length: 0\r\n\r\n>", act);

	HTTPTRX impl2("webappputargs2");
	impl2.PutArgs(ctx, &inpMapper);
	String act2("<");
	act2.Append(impl2.fData2Send.AsCharPtr("")).Append(">");
	assertEqual("<POST /udb/udbdemo HTTP/1.0\r\nAuthorization: Basic AAAAAA==\r\nContent-length: 42\r\n\r\ncmd=ModUser&pid=t028307&password=foo%20bah>", act2);
}

Test *HTTPTRXTest::suite ()
// collect all test cases for the SocketStream
{
	TestSuite *testSuite = new TestSuite;

	testSuite->addTest (NEW_CASE(HTTPTRXTest, fdPutArgsTest));
	testSuite->addTest (NEW_CASE(HTTPTRXTest, webAppPutArgsTest));

	return testSuite;
} // suite

//---- HTTPDAImplTest ----------------------------------------------------------------
HTTPDAImplTest::HTTPDAImplTest(TString tname) : TestCase(tname)
{
}

HTTPDAImplTest::~HTTPDAImplTest()
{
}

void HTTPDAImplTest::setUp ()
// setup connector for this TestCase
{
	Anything dummy;
	WDModule *ssl = WDModule::FindWDModule("SSLModule");
	ssl->Init(dummy);
} // setUp

void HTTPDAImplTest::tearDown ()
{
} // tearDown

void HTTPDAImplTest::useSSLTest()
{
	InputMapper in("useSSL");
	in.CheckConfig("InputMapper");
	OutputMapper out("useSSL");
	out.CheckConfig("OutputMapper");

	HTTPDAImpl httpDAImpl("useSSLTest");
	httpDAImpl.CheckConfig("DataAccessImpl");

	Anything dummy;
	Context ctx(dummy, dummy, 0, 0, 0, 0);

	t_assert(httpDAImpl.Exec(ctx, &in, &out));
}

void HTTPDAImplTest::ErrorHandlingTest()
{
	StartTrace(HTTPDAImplTest.ErrorHandlingTest);
	InputMapper in("useSSL");
	in.CheckConfig("InputMapper");
	OutputMapper out("useSSL");
	out.CheckConfig("OutputMapper");

	HTTPDAImpl httpDAImpl("ErrorHandlingTest");
	httpDAImpl.CheckConfig("DataAccessImpl");

	Anything dummy;
	Context ctx(dummy, dummy, 0, 0, 0, 0);

	t_assert(!httpDAImpl.Exec(ctx, &in, &out));
	TString strMsg("connection to  [Server Name:ErrorHandlingTest IP:");
	strMsg << ctx.Lookup("Backend.Server", "") << " Port:" << ctx.Lookup("Backend.Port", "") << "] failed";
	TraceAny(ctx.GetTmpStore()["Mapper"]["Error"], "Mapper.Error");
	assertEqual(strMsg, ctx.GetTmpStore()["Mapper"]["Error"].AsCharPtr("bad"));
}

Test *HTTPDAImplTest::suite ()
// collect all test cases for the SocketStream
{
	TestSuite *testSuite = new TestSuite;

	testSuite->addTest (NEW_CASE(HTTPDAImplTest, useSSLTest));
	testSuite->addTest (NEW_CASE(HTTPDAImplTest, ErrorHandlingTest));

	return testSuite;
} // suite
