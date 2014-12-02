/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#include "HTTPHeaderRendererTest.h"
#include "TestSuite.h"
#include "StringStream.h"
#include "Renderer.h"
#include "HTTPConstants.h"

void HTTPHeaderRendererTest::WholeHeaderConfig() {
	StartTrace(HTTPHeaderRendererTest.WholeHeaderConfig);
	Context c;
	c.GetTmpStore()["header"] = "adummyheaderline";
	Anything cfg;
	cfg["HTTPHeaderRenderer"]["HeaderSlot"] = "header";
	StringStream result;
	Renderer::Render(result, c, cfg);
	assertEqual("adummyheaderline\r\n", result.str());
}

void HTTPHeaderRendererTest::SingleLine() {
	StartTrace(HTTPHeaderRendererTest.SingleLine);
	Context c;
	c.GetTmpStore()["header"]["WWW-authenticate"] = "NTLM";
	Anything cfg;
	cfg["HTTPHeaderRenderer"]["HeaderSlot"] = "header";
	StringStream result;
	Renderer::Render(result, c, cfg);
	assertEqual("WWW-authenticate: NTLM\r\n", result.str());
}
void HTTPHeaderRendererTest::SingleLineMultiValue() {
	StartTrace(HTTPHeaderRendererTest.SingleLineMultiValue);
	Context c;
	Anything values = Anything(Anything::ArrayMarker());
	values.Append("NTLM");
	values.Append("Negotiate");
	values.Append("Basic");
	c.GetTmpStore()["header"]["WWW-authenticate"] = values;
	Anything cfg;
	cfg["HTTPHeaderRenderer"]["HeaderSlot"] = "header";
	StringStream result;
	Renderer::Render(result, c, cfg);
	assertEqual("WWW-authenticate: NTLM, Negotiate, Basic\r\n", result.str());
}

void HTTPHeaderRendererTest::MultiLine() {
	StartTrace(HTTPHeaderRendererTest.MultiLine);
	Context c;
	c.GetTmpStore()["header"]["WWW-Authenticate"] = "NTLM";
	c.GetTmpStore()["header"]["Connection"] = "close";
	c.GetTmpStore()["header"]["Content-Type"] = "text/html";
	Anything cfg;
	cfg["HTTPHeaderRenderer"]["HeaderSlot"] = "header";
	StringStream result;
	Renderer::Render(result, c, cfg);
	assertEqual("WWW-Authenticate: NTLM\r\n"
			"Connection: close\r\n"
			"Content-Type: text/html\r\n", result.str());
}

void HTTPHeaderRendererTest::Issue299MissingFilenamePrefix() {
	StartTrace(HTTPHeaderRendererTest.Issue299MissingFilenamePrefix);
	Context c;
	c.GetTmpStore()["header"][coast::http::constants::contentDispositionSlotname][0] = "attachment";
	c.GetTmpStore()["header"][coast::http::constants::contentDispositionSlotname]["FILENAME"] = "12166_reservation_07.12.2011.pdf";
	Anything cfg;
	cfg["HTTPHeaderRenderer"]["HeaderSlot"] = "header";
	StringStream result;
	Renderer::Render(result, c, cfg);
	assertEqual(String(coast::http::constants::contentDispositionSlotname).Append(": attachment; FILENAME=12166_reservation_07.12.2011.pdf\r\n")
			, result.str());
}

// builds up a suite of testcases, add a line for each testmethod
Test *HTTPHeaderRendererTest::suite() {
	StartTrace(HTTPHeaderRendererTest.suite);
	TestSuite *testSuite = new TestSuite;
	ADD_CASE(testSuite, HTTPHeaderRendererTest, SingleLine);
	ADD_CASE(testSuite, HTTPHeaderRendererTest, MultiLine);
	ADD_CASE(testSuite, HTTPHeaderRendererTest, SingleLineMultiValue);
	ADD_CASE(testSuite, HTTPHeaderRendererTest, WholeHeaderConfig);
	ADD_CASE(testSuite, HTTPHeaderRendererTest, Issue299MissingFilenamePrefix);
	return testSuite;
}
