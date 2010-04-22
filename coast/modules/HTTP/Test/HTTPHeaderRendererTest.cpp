/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

//--- interface include --------------------------------------------------------
#include "HTTPHeaderRendererTest.h"

//--- test modules used --------------------------------------------------------
#include "TestSuite.h"

//--- module under test --------------------------------------------------------
#include "HTTPHeaderRenderer.h"

//--- standard modules used ----------------------------------------------------
#include "Dbg.h"

//---- HTTPHeaderRendererTest ----------------------------------------------------------------
HTTPHeaderRendererTest::HTTPHeaderRendererTest(TString name) : TestCaseType(name)
{
	StartTrace(HTTPHeaderRendererTest.Ctor);
}

HTTPHeaderRendererTest::~HTTPHeaderRendererTest()
{
	StartTrace(HTTPHeaderRendererTest.Dtor);
}

// setup for this TestCase
void HTTPHeaderRendererTest::setUp ()
{
	StartTrace(HTTPHeaderRendererTest.setUp);
}

void HTTPHeaderRendererTest::tearDown ()
{
	StartTrace(HTTPHeaderRendererTest.tearDown);
}
void HTTPHeaderRendererTest::WholeHeaderConfig()
{
	StartTrace(HTTPHeaderRendererTest.WholeHeaderConfig);
	HTTPHeaderRenderer r("");
	Context c;
	c.GetTmpStore()["header"] = "adummyheaderline";
	Anything cfg;
	cfg["HeaderSlot"] = "header";
	StringStream result;
	r.RenderAll(result, c, cfg);
	assertEqual("adummyheaderline", result.str());
}

void HTTPHeaderRendererTest::SingleLine()
{
	StartTrace(HTTPHeaderRendererTest.SingleLine);
	HTTPHeaderRenderer r("");
	Context c;
	c.GetTmpStore()["header"]["WWW-authenticate"] = "NTLM";
	Anything cfg;
	cfg["HeaderSlot"] = "header";
	StringStream result;
	r.RenderAll(result, c, cfg);
	assertEqual("WWW-authenticate: NTLM\r\n", result.str());
}
void HTTPHeaderRendererTest::SingleLineMultiValue()
{
	StartTrace(HTTPHeaderRendererTest.SingleLineMultiValue);
	HTTPHeaderRenderer r("");
	Context c;
	MetaThing values;
	values.Append("NTLM");
	values.Append("Negotiate");
	values.Append("Basic");
	c.GetTmpStore()["header"]["WWW-authenticate"] = values;
	Anything cfg;
	cfg["HeaderSlot"] = "header";
	StringStream result;
	r.RenderAll(result, c, cfg);
	assertEqual("WWW-authenticate: NTLM, Negotiate, Basic\r\n", result.str());
}

void HTTPHeaderRendererTest::MultiLine()
{
	StartTrace(HTTPHeaderRendererTest.MultiLine);
	HTTPHeaderRenderer r("");
	Context c;
	c.GetTmpStore()["header"]["WWW-Authenticate"] = "NTLM";
	c.GetTmpStore()["header"]["Connection"] = "close";
	c.GetTmpStore()["header"]["Content-Type"] = "text/html";
	Anything cfg;
	cfg["HeaderSlot"] = "header";
	StringStream result;
	r.RenderAll(result, c, cfg);
	assertEqual("WWW-Authenticate: NTLM\r\n"
				"Connection: close\r\n"
				"Content-Type: text/html\r\n", result.str());
}

// builds up a suite of testcases, add a line for each testmethod
Test *HTTPHeaderRendererTest::suite ()
{
	StartTrace(HTTPHeaderRendererTest.suite);
	TestSuite *testSuite = new TestSuite;

	ADD_CASE(testSuite, HTTPHeaderRendererTest, SingleLine);
	ADD_CASE(testSuite, HTTPHeaderRendererTest, MultiLine);
	ADD_CASE(testSuite, HTTPHeaderRendererTest, SingleLineMultiValue);
	ADD_CASE(testSuite, HTTPHeaderRendererTest, WholeHeaderConfig);

	return testSuite;
}
