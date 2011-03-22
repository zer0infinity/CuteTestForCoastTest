/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#include "HTTPHeaderRendererTest.h"
#include "TestSuite.h"
#include "HTTPHeaderRenderer.h"
#include "StringStream.h"

//---- HTTPHeaderRendererTest ----------------------------------------------------------------
HTTPHeaderRendererTest::HTTPHeaderRendererTest(TString tname) : TestCaseType(tname)
{
	StartTrace(HTTPHeaderRendererTest.Ctor);
}

HTTPHeaderRendererTest::~HTTPHeaderRendererTest()
{
	StartTrace(HTTPHeaderRendererTest.Dtor);
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
	Anything values = Anything(Anything::ArrayMarker());
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
