/*
 * Copyright (c) 2009, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

//--- interface include --------------------------------------------------------
#include "UTF8RendererTest.h"

//--- test modules used --------------------------------------------------------
#include "TestSuite.h"

//--- module under test --------------------------------------------------------

//--- standard modules used ----------------------------------------------------
#include "Dbg.h"
#include "System.h"
#include "Context.h"
#include "Renderer.h"

//--- c-modules used -----------------------------------------------------------

//---- UTF8RendererTest ----------------------------------------------------------------
UTF8RendererTest::UTF8RendererTest(TString name) : TestCase(name)
{
	StartTrace(UTF8RendererTest.Ctor);
}

UTF8RendererTest::~UTF8RendererTest()
{
	StartTrace(UTF8RendererTest.Dtor);
}

// setup for this TestCase
void UTF8RendererTest::setUp ()
{
	StartTrace(UTF8RendererTest.setUp);
}

void UTF8RendererTest::tearDown ()
{
	StartTrace(UTF8RendererTest.tearDown);
}

void UTF8RendererTest::conversionTest()
{
	StartTrace(UTF8RendererTest.conversionTest);
	char in[] = {'H', 0xe4, 'l', 'l', 0xf6, '\0' };
	char utf8exp[] = {'H', 0xc3, 0xa4, 'l', 'l', 0xc3, 0xb6, '\0' };
	String str(in), strExpected(utf8exp);
	assertEqual(strExpected, str.AsUTF8());
}

// builds up a suite of testcases, add a line for each testmethod
Test *UTF8RendererTest::suite ()
{
	StartTrace(UTF8RendererTest.suite);
	TestSuite *testSuite = new TestSuite;

	ADD_CASE(testSuite, UTF8RendererTest, conversionTest);

	return testSuite;
}
