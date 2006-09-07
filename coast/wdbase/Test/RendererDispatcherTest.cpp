/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

//--- interface include --------------------------------------------------------
#include "RendererDispatcherTest.h"

//--- test modules used --------------------------------------------------------
#include "TestSuite.h"

//--- module under test --------------------------------------------------------
#include "ServiceDispatcher.h"

//--- standard modules used ----------------------------------------------------
#include "Dbg.h"

//--- c-library modules used ---------------------------------------------------

//---- RendererDispatcherTest ----------------------------------------------------------------
RendererDispatcherTest::RendererDispatcherTest(TString tname) : TestCaseType(tname)
{
	StartTrace(RendererDispatcherTest.Ctor);
}

RendererDispatcherTest::~RendererDispatcherTest()
{
	StartTrace(RendererDispatcherTest.Dtor);
}

void RendererDispatcherTest::FindServiceNameTest()
{
	StartTrace(RendererDispatcherTest.FindServiceNameTest);
	{
		// test with no configuration should behave like superclass
		Context ctx;
		RendererDispatcher sd("TestServiceDispatcher");
		t_assertm(sd.FindServiceHandler(ctx) != 0, "expected to find standard handler");
		assertEqualm("WebAppService", sd.FindServiceName(ctx), "expected to find standard service name");
	}
	{
		// test with invalid context entry
		Anything args;
		args["env"]["DefaultService"] = "NoService";
		Context ctx(args);
		RendererDispatcher sd("TestServiceDispatcher");
		t_assertm(sd.FindServiceHandler(ctx) == 0, "expected to find no handler");
		assertEqualm("NoService", sd.FindServiceName(ctx), "expected to find invalid service name");
	}
	{
		// test with invalid context entry
		Anything args;
		args["env"]["REQUEST_URI"] = "/simpleServiceURL/";
		Context ctx(args);
		RendererDispatcher sd("TestRenderDispatcher");
		sd.CheckConfig("ServiceDispatcher");
		ctx.Push("ServiceDispatcher", &sd);
		assertEqual("SimpleService", ctx.Lookup("URIPrefix2ServiceMap./simpleServiceURL/", "NoService"));
		t_assertm(sd.FindServiceHandler(ctx) == 0, "expected to find no handler");
		assertEqualm("SimpleService", sd.FindServiceName(ctx), "expected to find SimpleService name");
	}
	{
		// test with invalid context entry
		Anything args;
		args["env"]["REQUEST_URI"] = "/rendererdServiceURL/";
		Context ctx(args);
		RendererDispatcher sd("TestRenderDispatcher");
		sd.CheckConfig("ServiceDispatcher");
		ctx.Push("ServiceDispatcher", &sd);
		t_assertm(sd.FindServiceHandler(ctx) == 0, "expected to find no handler");
		assertEqualm("LookupedService", sd.FindServiceName(ctx), "expected to find LookupedService name");
	}
}

// builds up a suite of testcases, add a line for each testmethod
Test *RendererDispatcherTest::suite ()
{
	StartTrace(RendererDispatcherTest.suite);
	TestSuite *testSuite = new TestSuite;

	ADD_CASE(testSuite, RendererDispatcherTest, FindServiceNameTest);

	return testSuite;
}
