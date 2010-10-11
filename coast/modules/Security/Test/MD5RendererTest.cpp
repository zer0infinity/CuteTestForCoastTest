/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

//--- interface include --------------------------------------------------------
#include "MD5RendererTest.h"

//--- test modules used --------------------------------------------------------
#include "TestSuite.h"

//--- module under test --------------------------------------------------------
#include "MD5Renderer.h"

//--- standard modules used ----------------------------------------------------
#include "Dbg.h"

//---- MD5RendererTest ----------------------------------------------------------------
MD5RendererTest::MD5RendererTest(TString tname) : TestCaseType(tname)
{
	StartTrace(MD5RendererTest.Ctor);
}

MD5RendererTest::~MD5RendererTest()
{
	StartTrace(MD5RendererTest.Dtor);
}

// setup for this TestCase
void MD5RendererTest::setUp ()
{
	StartTrace(MD5RendererTest.setUp);
}

void MD5RendererTest::tearDown ()
{
	StartTrace(MD5RendererTest.tearDown);
}

void MD5RendererTest::testCase()
{
	StartTrace(MD5RendererTest.testCase);
}

void MD5RendererTest::testOutputAsBase64()
{

	StartTrace(MD5RendererTest.testOutputAsBase64);
	MD5Renderer md5r(fName);
	Context ctx;
	{
		Anything anyConfig;
		anyConfig["ToSign"] = "Ali Baba und die 40 Indianer";
		String renderedResult;
		StringStream aStream(renderedResult);
		md5r.RenderAll(aStream, ctx, anyConfig);
		aStream << std::flush; // sync with String object
		Trace("Rendered Result: " << renderedResult);
		assertEqual("KFcBRonj6FHndQldt8854g==", renderedResult);
	}
	{
		Anything anyConfig;
		anyConfig["Mode"] = "Base64";
		anyConfig["ToSign"] = "Ali Baba und die 40 Indianer";
		String renderedResult;
		StringStream aStream(renderedResult);
		md5r.RenderAll(aStream, ctx, anyConfig);
		aStream << std::flush; // sync with String object
		Trace("Rendered Result: " << renderedResult);
		assertEqual("KFcBRonj6FHndQldt8854g==", renderedResult);
	}
	{
		Anything anyConfig;
		anyConfig["Mode"] = "Base64";
		anyConfig["ToSign"] = "Kim Dojo goes wild today";
		String renderedResult;
		StringStream aStream(renderedResult);
		md5r.RenderAll(aStream, ctx, anyConfig);
		aStream << std::flush; // sync with String object
		Trace("Rendered Result: " << renderedResult);
		assertEqual("jeBX5JJXdB4QW9p71t7a2Q==", renderedResult);
	}
}

void MD5RendererTest::testOutputAsHex()
{

	StartTrace(MD5RendererTest.testOutputAsHex);
	MD5Renderer md5r(fName);
	Context ctx;
	// Input from rfc1321 (MD5) test suite
	{
		Anything anyConfig;
		anyConfig["Mode"] = "Hex";
		anyConfig["ToSign"] = "";
		String renderedResult;
		StringStream aStream(renderedResult);
		md5r.RenderAll(aStream, ctx, anyConfig);
		aStream << std::flush; // sync with String object
		t_assert(renderedResult.Length() == 32);
		Trace("Rendered Result: " <<  renderedResult);
		assertEqual("d41d8cd98f00b204e9800998ecf8427e", renderedResult.ToLower());
	}
	{
		Anything anyConfig;
		anyConfig["Mode"] = "Hex";
		anyConfig["ToSign"] = "a";
		String renderedResult;
		StringStream aStream(renderedResult);
		md5r.RenderAll(aStream, ctx, anyConfig);
		aStream << std::flush; // sync with String object
		Trace("Rendered Result: " << renderedResult);
		assertEqual("0cc175b9c0f1b6a831c399e269772661", renderedResult.ToLower());
	}
	{
		Anything anyConfig;
		anyConfig["Mode"] = "Hex";
		anyConfig["ToSign"] = "abc";
		String renderedResult;
		StringStream aStream(renderedResult);
		md5r.RenderAll(aStream, ctx, anyConfig);
		aStream << std::flush; // sync with String object
		Trace("Rendered Result: " << renderedResult);
		assertEqual("900150983cd24fb0d6963f7d28e17f72", renderedResult.ToLower());
	}
	{
		Anything anyConfig;
		anyConfig["Mode"] = "Hex";
		anyConfig["ToSign"] = "message digest";
		String renderedResult;
		StringStream aStream(renderedResult);
		md5r.RenderAll(aStream, ctx, anyConfig);
		aStream << std::flush; // sync with String object
		Trace("Rendered Result: " << renderedResult);
		assertEqual("f96b697d7cb7938d525a2f31aaf161d0", renderedResult.ToLower());
	}
	{
		Anything anyConfig;
		anyConfig["Mode"] = "Hex";
		anyConfig["ToSign"] = "abcdefghijklmnopqrstuvwxyz";
		String renderedResult;
		StringStream aStream(renderedResult);
		md5r.RenderAll(aStream, ctx, anyConfig);
		aStream << std::flush; // sync with String object
		Trace("Rendered Result: " << renderedResult);
		assertEqual("c3fcd3d76192e4007dfb496cca67e13b", renderedResult.ToLower());
	}
	{
		Anything anyConfig;
		anyConfig["Mode"] = "Hex";
		anyConfig["ToSign"] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789";
		String renderedResult;
		StringStream aStream(renderedResult);
		md5r.RenderAll(aStream, ctx, anyConfig);
		aStream << std::flush; // sync with String object
		Trace("Rendered Result: " << renderedResult);
		assertEqual("d174ab98d277d9f5a5611c2c9f419d9f", renderedResult.ToLower());
	}
	{
		Anything anyConfig;
		anyConfig["Mode"] = "Hex";
		anyConfig["ToSign"] = "12345678901234567890123456789012345678901234567890123456789012345678901234567890";
		String renderedResult;
		StringStream aStream(renderedResult);
		md5r.RenderAll(aStream, ctx, anyConfig);
		aStream << std::flush; // sync with String object
		Trace("Rendered Result: " << renderedResult);
		assertEqual("57edf4a22be3c955ac49da2e2107b67a", renderedResult.ToLower());
	}
}

// builds up a suite of testcases, add a line for each testmethod
Test *MD5RendererTest::suite ()
{
	StartTrace(MD5RendererTest.suite);
	TestSuite *testSuite = new TestSuite;

	ADD_CASE(testSuite, MD5RendererTest, testCase);
	ADD_CASE(testSuite, MD5RendererTest, testOutputAsBase64);
	ADD_CASE(testSuite, MD5RendererTest, testOutputAsHex);
	return testSuite;
}
