/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#include "HTTPResponseMapperTest.h"
#include "TestSuite.h"
#include "HTTPResponseMapper.h"
#include "StringStream.h"
#include "Context.h"
#include "HTTPConstants.h"

void HTTPResponseMapperTest::TestParsedResponse() {
	StartTrace(HTTPResponseMapperTest.TestParsedResponse);
	String strIn("HTTP/1.1 200 Ok\r\n");
	IStringStream is(strIn);
	HTTPResponseMapper m("HTTPResponseMapper");
	m.Initialize("ResultMapper");
	Context ctx;
	t_assert(m.Put("", is, ctx));
	Anything result(ctx.GetTmpStore()["Mapper"]["HTTPResponse"]);
	assertEqual(200L, result[Coast::HTTP::constants::protocolCodeSlotname].AsLong(0));
	assertEqual("Ok", result[Coast::HTTP::constants::protocolMsgSlotname].AsCharPtr());
	assertEqual("HTTP/1.1", result[Coast::HTTP::constants::protocolVersionSlotname].AsCharPtr());
}
void HTTPResponseMapperTest::TestBadResponseLine() {
	StartTrace(HTTPResponseMapperTest.TestBadResponseLine);
	String strIn("HTTP/1.1 ");
	IStringStream is(strIn);
	HTTPResponseMapper m("HTTPResponseMapper");
	m.Initialize("ResultMapper");
	Context ctx;
	t_assert(!m.Put("", is, ctx));
	Anything result(ctx.GetTmpStore()["Mapper"]["HTTPResponse"]);
	assertEqual(-1L, result[Coast::HTTP::constants::protocolCodeSlotname].AsLong(-1));
	assertEqual("undefined", result[Coast::HTTP::constants::protocolMsgSlotname].AsCharPtr("undefined"));
	assertEqual("HTTP/1.1", result[Coast::HTTP::constants::protocolVersionSlotname].AsCharPtr());
}

Test *HTTPResponseMapperTest::suite() {
	StartTrace(HTTPResponseMapperTest.suite);
	TestSuite *testSuite = new TestSuite;
	ADD_CASE(testSuite, HTTPResponseMapperTest, TestParsedResponse);
	ADD_CASE(testSuite, HTTPResponseMapperTest, TestBadResponseLine);
	return testSuite;
}
