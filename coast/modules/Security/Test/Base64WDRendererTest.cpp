/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

//--- interface include --------------------------------------------------------
#include "Base64WDRendererTest.h"

//--- test modules used --------------------------------------------------------
#include "TestSuite.h"

//--- module under test --------------------------------------------------------
#include "Base64WDRenderer.h"

//--- standard modules used ----------------------------------------------------
#include "Dbg.h"

//---- Base64WDRendererTest ----------------------------------------------------------------
Base64WDRendererTest::Base64WDRendererTest(TString tname) : TestCaseType(tname)
{
	StartTrace(Base64WDRendererTest.Ctor);
}

Base64WDRendererTest::~Base64WDRendererTest()
{
	StartTrace(Base64WDRendererTest.Dtor);
}

// setup for this TestCase
void Base64WDRendererTest::setUp ()
{
	StartTrace(Base64WDRendererTest.setUp);
}

void Base64WDRendererTest::tearDown ()
{
	StartTrace(Base64WDRendererTest.tearDown);
}

void Base64WDRendererTest::RenderAllTest()
{
	StartTrace(Base64WDRendererTest.RenderAllTest);
	Base64WDRenderer aRenderer("Base64WDRenderer");
	Context ctx;
	Anything anyConfig;
	anyConfig[0L] = "\377\377alpha";
	{
		String str;
		StringStream aStream(str);
		aRenderer.RenderAll(aStream, ctx, anyConfig);
		aStream << std::flush;
		assertEqual("$$9hbHBoYQ==", str);
	}
	String aStr("X\357\376");
	aStr.PutAt(0, '\0');
	anyConfig[0L] = aStr;
	{
		String str;
		StringStream aStream(str);
		aRenderer.RenderAll(aStream, ctx, anyConfig);
		aStream << std::flush;
		assertEqual("AO$-", str);
	}
}

// builds up a suite of testcases, add a line for each testmethod
Test *Base64WDRendererTest::suite ()
{
	StartTrace(Base64WDRendererTest.suite);
	TestSuite *testSuite = new TestSuite;

	ADD_CASE(testSuite, Base64WDRendererTest, RenderAllTest);

	return testSuite;
}
