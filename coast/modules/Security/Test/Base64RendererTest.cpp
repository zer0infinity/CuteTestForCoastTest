/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

//--- interface include --------------------------------------------------------
#include "Base64RendererTest.h"

//--- test modules used --------------------------------------------------------
#include "TestSuite.h"

//--- module under test --------------------------------------------------------
#include "Base64Renderer.h"

//--- standard modules used ----------------------------------------------------
#include "Dbg.h"

//--- c-modules used -----------------------------------------------------------

//---- Base64RendererTest ----------------------------------------------------------------
Base64RendererTest::Base64RendererTest(TString name) : TestCase(name)
{
	StartTrace(Base64RendererTest.Ctor);
}

Base64RendererTest::~Base64RendererTest()
{
	StartTrace(Base64RendererTest.Dtor);
}

// setup for this TestCase
void Base64RendererTest::setUp ()
{
	StartTrace(Base64RendererTest.setUp);
}

void Base64RendererTest::tearDown ()
{
	StartTrace(Base64RendererTest.tearDown);
}

void Base64RendererTest::RenderAllTest()
{
	StartTrace(Base64RendererTest.RenderAllTest);
	Base64Renderer aRenderer("Base64Renderer");
	Context ctx;
	Anything anyConfig;
	anyConfig[0L] = "\377\377alphbushyourself/&#?";
	{
		String str;
		StringStream aStream(str);
		aRenderer.RenderAll(aStream, ctx, anyConfig);
		aStream << flush;
		assertEqual("//9hbHBoYnVzaHlvdXJzZWxmLyYjPw==", str);
	}
}

// builds up a suite of testcases, add a line for each testmethod
Test *Base64RendererTest::suite ()
{
	StartTrace(Base64RendererTest.suite);
	TestSuite *testSuite = new TestSuite;

	ADD_CASE(testSuite, Base64RendererTest, RenderAllTest);

	return testSuite;
}
