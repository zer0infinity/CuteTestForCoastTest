/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

//--- interface include --------------------------------------------------------
#include "RequestProcessorTest.h"

//--- test modules used --------------------------------------------------------
#include "TestSuite.h"

//--- module under test --------------------------------------------------------
#include "RequestProcessor.h"

//--- standard modules used ----------------------------------------------------
#include "StringStreamSocket.h"
#include "Server.h"
#include "Dbg.h"

//--- c-library modules used ---------------------------------------------------

//---- RequestProcessorTest ----------------------------------------------------------------
RequestProcessorTest::RequestProcessorTest(TString tname) : TestCaseType(tname)
{
	StartTrace(RequestProcessorTest.Ctor);
}

RequestProcessorTest::~RequestProcessorTest()
{
	StartTrace(RequestProcessorTest.Dtor);
}

void RequestProcessorTest::InitTest()
{
	StartTrace(RequestProcessorTest.InitTest);
	RequestProcessor rp("test");

	t_assertm(!rp.fServer, "expected fServer to be null");
	rp.Init(Server::FindServer("Server"));
	t_assertm(rp.fServer != 0, "expected fServer not to be null");
	rp.Init(0);
	t_assertm(!rp.fServer, "expected fServer to be null");
}

void RequestProcessorTest::ProcessRequestTest()
{
	StartTrace(RequestProcessorTest.ProcessRequestTest);
	Anything empty;
	Anything clientinfo;
	clientinfo["REMOTE_ADDR"] = "localhost";
	clientinfo["HTTPS"] = false;

	//--- normal case ------------------
	String str("{ 1 }");
	{
		LoopbackProcessor rp("test");
		rp.Init(Server::FindServer("Server"));
		StringStreamSocket sss(str);
		Context ctx(&sss);

		rp.ProcessRequest(ctx);
		assertAnyEqualm(clientinfo, ctx.Lookup("ClientInfo"), "expected ClientInfo from Socket");
	}
	Trace("String<" << str << ">");
	assertEqual("{\n  1\n}", str);

	//--- no socket case ---------
	{
		LoopbackProcessor rp("test");
		rp.Init(Server::FindServer("Server"));
		Context ctx;

		rp.ProcessRequest(ctx);
		assertAnyEqualm(empty, ctx.Lookup("ClientInfo"), "expected ClientInfo to be empty");

	}

	//--- null socket pointer case ---------
	{
		LoopbackProcessor rp("test");
		rp.Init(Server::FindServer("Server"));
		Context ctx((Socket *)0);

		rp.ProcessRequest(ctx);
		assertAnyEqualm(empty, ctx.Lookup("ClientInfo"), "expected ClientInfo to be empty");
	}
	Trace("String<" << str << ">");
	assertEqual("{\n  1\n}", str);
}

// builds up a suite of testcases, add a line for each testmethod
Test *RequestProcessorTest::suite ()
{
	StartTrace(RequestProcessorTest.suite);
	TestSuite *testSuite = new TestSuite;

	ADD_CASE(testSuite, RequestProcessorTest, InitTest);
	ADD_CASE(testSuite, RequestProcessorTest, ProcessRequestTest);

	return testSuite;
}

//--- LoopbackProcessor ----------------------------------------------
RegisterRequestProcessor(LoopbackProcessor);

LoopbackProcessor::LoopbackProcessor(const char *processorName)
	: RequestProcessor(processorName)
{
	StartTrace(LoopbackProcessor.LoopbackProcessor);
}

void LoopbackProcessor::DoReadInput(iostream &Ios, Context &ctx)
{
	StartTrace(LoopbackProcessor.DoReadInput);
	Anything request;
	request.Import(Ios);

	ctx.PushStore("Request", request);
}

void LoopbackProcessor::DoProcessRequest(ostream &reply, Context &ctx)
{
	StartTrace(LoopbackProcessor.DoProcessRequest);
	Anything request;

	if ( ctx.GetStore("Request", request) ) {
		request.PrintOn(reply);
	}
}
