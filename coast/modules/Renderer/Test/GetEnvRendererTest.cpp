/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

//--- interface include --------------------------------------------------------
#include "GetEnvRendererTest.h"

//--- test modules used --------------------------------------------------------
#include "TestSuite.h"

//--- module under test --------------------------------------------------------

//--- standard modules used ----------------------------------------------------
#include "Dbg.h"
#include "System.h"
#include "Context.h"
#include "Renderer.h"

//--- c-modules used -----------------------------------------------------------

//---- GetEnvRendererTest ----------------------------------------------------------------
GetEnvRendererTest::GetEnvRendererTest(TString name) : TestCase(name)
{
	StartTrace(GetEnvRendererTest.Ctor);
}

GetEnvRendererTest::~GetEnvRendererTest()
{
	StartTrace(GetEnvRendererTest.Dtor);
}

// setup for this TestCase
void GetEnvRendererTest::setUp ()
{
	StartTrace(GetEnvRendererTest.setUp);
}

void GetEnvRendererTest::tearDown ()
{
	StartTrace(GetEnvRendererTest.tearDown);
}

void GetEnvRendererTest::testCase()
{
	StartTrace(GetEnvRendererTest.testCase);
	String user = System::EnvGet("USER");
	Context ctx;
	Anything config;
	config["GetEnvRenderer"] = "USER";
	assertEqual(user, Renderer::RenderToString(ctx, config));

	config["GetEnvRenderer"] = "USERBLAHBLAHXXX";
	assertEqual("", Renderer::RenderToString(ctx, config));
}

// builds up a suite of testcases, add a line for each testmethod
Test *GetEnvRendererTest::suite ()
{
	StartTrace(GetEnvRendererTest.suite);
	TestSuite *testSuite = new TestSuite;

	ADD_CASE(testSuite, GetEnvRendererTest, testCase);

	return testSuite;
}
