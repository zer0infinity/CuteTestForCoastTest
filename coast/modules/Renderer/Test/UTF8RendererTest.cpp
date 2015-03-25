/*
 * Copyright (c) 2009, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#include "UTF8RendererTest.h"
#include "TestSuite.h"
#include "UTF8Renderer.h"
#include "StringStream.h"

//---- UTF8RendererTest ----------------------------------------------------------------
void UTF8RendererTest::conversionTest() {
	StartTrace(UTF8RendererTest.conversionTest);
	const char in[] = { 'H', static_cast<char>(0xe4), 'l', 'l', static_cast<char>(0xf6), '\0' };
	const char utf8exp[] = { 'H', static_cast<char>(0xc3), static_cast<char>(0xa4), 'l', 'l', static_cast<char>(0xc3), static_cast<char>(0xb6), '\0' };
	String strExpected(utf8exp), strReply;
	Context ctx;
	Anything anyConfig;
	anyConfig["UTF8Renderer"]["String"] = in;
	{
		OStringStream reply(strReply);
		Renderer::Render(reply, ctx, anyConfig);
	}
	assertEqual(strExpected, strReply);
}

// builds up a suite of testcases, add a line for each testmethod
Test *UTF8RendererTest::suite() {
	StartTrace(UTF8RendererTest.suite);
	TestSuite *testSuite = new TestSuite;
	ADD_CASE(testSuite, UTF8RendererTest, conversionTest);
	return testSuite;
}
