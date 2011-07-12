/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#include "GetEnvRendererTest.h"
#include "TestSuite.h"
#include "Tracer.h"
#include "SystemBase.h"
#include "Context.h"
#include "Renderer.h"

//---- GetEnvRendererTest ----------------------------------------------------------------
GetEnvRendererTest::GetEnvRendererTest(TString tstrName) : TestCaseType(tstrName)
{
	StartTrace(GetEnvRendererTest.Ctor);
}

GetEnvRendererTest::~GetEnvRendererTest()
{
	StartTrace(GetEnvRendererTest.Dtor);
}

void GetEnvRendererTest::EnvTest()
{
	StartTrace(GetEnvRendererTest.EnvTest);
	String user = Coast::System::EnvGet("USER");
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
	ADD_CASE(testSuite, GetEnvRendererTest, EnvTest);
	return testSuite;
}
