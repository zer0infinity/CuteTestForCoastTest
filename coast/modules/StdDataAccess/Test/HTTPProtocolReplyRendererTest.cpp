/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#include "HTTPProtocolReplyRendererTest.h"
#include "HTTPProtocolReplyRenderer.h"
#include "TestSuite.h"
#include "StringStream.h"
#include "HTTPConstants.h"

void HTTPProtocolReplyRendererTest::ReasonLessErrorReplyLine() {
	StartTrace(HTTPProtocolReplyRendererTest.ReasonLessErrorReplyLine);
	Context c;
	Anything tmpStore(c.GetTmpStore());
	tmpStore["HTTPStatus"][coast::http::constants::protocolCodeSlotname] = 500L;
	OStringStream response;
	Anything anyConfig;
	anyConfig["HTTPProtocolReplyRenderer"] = Anything();
	Renderer::Render(response, c, anyConfig);
	assertEqual("HTTP/1.1 500 Server Error\r\nConnection: close\r\n", response.str());
	assertEqual(500L, c.Lookup(String("HTTPStatus.").Append(coast::http::constants::protocolCodeSlotname), -1L));
}

void HTTPProtocolReplyRendererTest::ReasonLessReplyLine() {
	StartTrace(HTTPProtocolReplyRendererTest.ReasonLessReplyLine);
	Context c;
	OStringStream response;
	Anything anyConfig;
	anyConfig["HTTPProtocolReplyRenderer"] = Anything();
	Renderer::Render(response, c, anyConfig);
	assertEqual("HTTP/1.1 200 OK\r\nConnection: close\r\n", response.str());
	assertEqual(200L, c.Lookup(String("HTTPStatus.").Append(coast::http::constants::protocolCodeSlotname), -1L));
}

void HTTPProtocolReplyRendererTest::ConnectionCloseTest() {
	StartTrace(HTTPProtocolReplyRendererTest.ConnectionCloseTest);
	Anything config;
	config["PersistentConnections"] = 1L;
	Context c(config);
	OStringStream response;
	Anything anyConfig;
	anyConfig["HTTPProtocolReplyRenderer"] = Anything();
	Renderer::Render(response, c, anyConfig);
	assertEqual("HTTP/1.1 200 OK\r\nConnection: close\r\n", response.str());
	Anything tmpStore(c.GetTmpStore());
	tmpStore["Keep-Alive"] = 1L;
	OStringStream response2;
	Renderer::Render(response2, c, anyConfig);
	assertEqual("HTTP/1.1 200 OK\r\n", response2.str());
}

void HTTPProtocolReplyRendererTest::RequestSuccessfulReplyLine() {
	StartTrace(HTTPProtocolReplyRendererTest.RequestSuccessfulReplyLine);
	Context c;
	Anything tmpStore(c.GetTmpStore());
	tmpStore["HTTPStatus"][coast::http::constants::protocolVersionSlotname] = "HTTP/1.1";
	tmpStore["HTTPStatus"][coast::http::constants::protocolCodeSlotname] = 200L;
	tmpStore["HTTPStatus"][coast::http::constants::protocolMsgSlotname] = "OK";
	OStringStream response;
	Anything anyConfig;
	anyConfig["HTTPProtocolReplyRenderer"] = Anything();
	Renderer::Render(response, c, anyConfig);
	assertEqual("HTTP/1.1 200 OK\r\nConnection: close\r\n", response.str());
	OStringStream response2;
	tmpStore["HTTPStatus"][coast::http::constants::protocolVersionSlotname] = "HTTP/1.0";
	tmpStore["HTTPStatus"][coast::http::constants::protocolMsgSlotname] = "OKEYDOKEY";
	Renderer::Render(response2, c, anyConfig);
	assertEqual("HTTP/1.0 200 OKEYDOKEY\r\n", response2.str());
}

void HTTPProtocolReplyRendererTest::DefaultReasonPhraseTest() {
	StartTrace(HTTPProtocolReplyRendererTest.DefaultReasonPhraseTest);
	HTTPProtocolReplyRenderer r("HTTPProtocolReplyRenderer");
	assertEqual("OK", r.DefaultReasonPhrase(200L));
	assertEqual("Unknown Error", r.DefaultReasonPhrase(555));
}

// builds up a suite of testcases, add a line for each testmethod
Test *HTTPProtocolReplyRendererTest::suite() {
	StartTrace(HTTPProtocolReplyRendererTest.suite);
	TestSuite *testSuite = new TestSuite;
	ADD_CASE(testSuite, HTTPProtocolReplyRendererTest, RequestSuccessfulReplyLine);
	ADD_CASE(testSuite, HTTPProtocolReplyRendererTest, ReasonLessReplyLine);
	ADD_CASE(testSuite, HTTPProtocolReplyRendererTest, ConnectionCloseTest);
	ADD_CASE(testSuite, HTTPProtocolReplyRendererTest, ReasonLessErrorReplyLine);
	ADD_CASE(testSuite, HTTPProtocolReplyRendererTest, DefaultReasonPhraseTest);
	return testSuite;
}
