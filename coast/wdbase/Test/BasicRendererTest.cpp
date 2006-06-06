/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

//--- interface include --------------------------------------------------------
#include "BasicRendererTest.h"

//--- test modules used --------------------------------------------------------
#include "TestSuite.h"

//--- module under test --------------------------------------------------------
#include "Renderer.h"

//--- standard modules used ----------------------------------------------------
#include "Dbg.h"

//--- c-modules used -----------------------------------------------------------

//---- BasicRendererTest ----------------------------------------------------------------
BasicRendererTest::BasicRendererTest(TString tstrName) : TestCaseType(tstrName)
{
	StartTrace(BasicRendererTest.Ctor);
}

BasicRendererTest::~BasicRendererTest()
{
	StartTrace(BasicRendererTest.Dtor);
}

void BasicRendererTest::setUp ()
{
	StartTrace(BasicRendererTest.setUp);
	//FIXME: depends on AppBooterTest loading libtestlib.so
	// should init renderer module and dll loading itself.
}

void BasicRendererTest::RenderASimpleString()
{
	StartTrace(BasicRendererTest.RenderASimpleString);

	OStringStream os;
	Context ctx;
	Anything info("output");
	Renderer::Render(os, ctx, info);
	assertEqual("output", os.str());
}

void BasicRendererTest::RenderASimpleList()
{
	StartTrace(BasicRendererTest.RenderASimpleList);

	String result;
	Context ctx;
	Anything info;
	info.Append("This ");
	info.Append("is a ");
	info.Append(5L);
	info.Append(" element ");
	info.Append("list!");;
	Renderer::RenderOnString(result, ctx, info);
	assertEqual("This is a 5 element list!", result);
}

void BasicRendererTest::RenderOnStringTest()
{
	StartTrace(BasicRendererTest.RenderOnStringTest);
	String result;
	Context ctx;
	Anything info("output");
	Renderer::RenderOnString(result, ctx, info);
	assertEqual("output", result);
}
void BasicRendererTest::RenderOnStringDefaultTest()
{
	StartTrace(BasicRendererTest.RenderOnStringDefaultTest);
	String result;
	Context ctx;
	Anything info("default output");
	Renderer::RenderOnStringWithDefault(result, ctx, Anything(), info);
	assertEqual("default output", result);
	result = "";
	Renderer::RenderOnStringWithDefault(result, ctx, Anything("something"), info);
	assertEqual("something", result);

}
void BasicRendererTest::RenderWithConfig()
{
	StartTrace(BasicRendererTest.RenderWithConfig);

	String result;
	Context ctx;
	Anything trcfg;
	trcfg["TestRenderer"] = "dummy";
	Anything notfoundcfg;
	notfoundcfg["renderernotfound"] = " configuration!";
	Anything info;
	info.Append("This ");
	info.Append("is ");
	info.Append(trcfg);
	info.Append(notfoundcfg); // expect a syslog warning
	Renderer::RenderOnString(result, ctx, info);
	assertEqual("This is output from TestRenderer configuration!", result);
}

// builds up a suite of testcases, add a line for each testmethod
Test *BasicRendererTest::suite ()
{
	StartTrace(BasicRendererTest.suite);
	TestSuite *testSuite = new TestSuite;

	ADD_CASE(testSuite, BasicRendererTest, RenderASimpleString);
	ADD_CASE(testSuite, BasicRendererTest, RenderOnStringTest);
	ADD_CASE(testSuite, BasicRendererTest, RenderOnStringDefaultTest);
	ADD_CASE(testSuite, BasicRendererTest, RenderASimpleList);
	ADD_CASE(testSuite, BasicRendererTest, RenderWithConfig);

	return testSuite;
}
