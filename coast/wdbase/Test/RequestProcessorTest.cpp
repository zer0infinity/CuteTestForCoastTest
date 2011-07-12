/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#include "RequestProcessorTest.h"
#include "TestSuite.h"
#include "FoundationTestTypes.h"
#include "StringStreamSocket.h"
#include "Server.h"
#include "Tracer.h"
#include "AnythingUtils.h"

void RequestProcessorTest::InitTest() {
	StartTrace(RequestProcessorTest.InitTest);
	RequestProcessor rp("test");

	t_assertm(!rp.GetServer(), "expected fServer to be null");
	rp.Init(Server::FindServer("Server"));
	t_assertm(rp.GetServer() != 0, "expected fServer not to be null");
	rp.Init(0);
	t_assertm(!rp.GetServer(), "expected fServer to be null");
}

void RequestProcessorTest::ProcessRequestTest() {
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
		Context ctx((Socket *) 0);

		rp.ProcessRequest(ctx);
		assertAnyEqualm(empty, ctx.Lookup("ClientInfo"), "expected ClientInfo to be empty");
	}
	Trace("String<" << str << ">");
	assertEqual("{\n  1\n}", str);
}

// builds up a suite of testcases, add a line for each testmethod
Test *RequestProcessorTest::suite() {
	StartTrace(RequestProcessorTest.suite);
	TestSuite *testSuite = new TestSuite;

	ADD_CASE(testSuite, RequestProcessorTest, InitTest);
	ADD_CASE(testSuite, RequestProcessorTest, ProcessRequestTest);

	return testSuite;
}
//--- LoopbackProcessor ----------------------------------------------
RegisterRequestProcessor(LoopbackProcessor);

LoopbackProcessor::LoopbackProcessor(const char *processorName) :
	RequestProcessor(processorName) {
	StartTrace(LoopbackProcessor.LoopbackProcessor)
	;
}

bool LoopbackProcessor::DoReadInput(std::iostream &Ios, Context &ctx) {
	StartTrace(LoopbackProcessor.DoReadInput);
	Anything request;
	request.Import(Ios);
	StorePutter::Operate(request, ctx, "Tmp", "TestRequest");
	return true;
}

bool LoopbackProcessor::DoProcessRequest(std::ostream &reply, Context &ctx) {
	StartTrace(LoopbackProcessor.DoProcessRequest);
	ROAnything request;
	if (ctx.Lookup("TestRequest", request)) {
		request.PrintOn(reply);
		return true;
	}
	return false;
}
