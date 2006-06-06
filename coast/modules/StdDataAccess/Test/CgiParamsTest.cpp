/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

//--- standard modules used ----------------------------------------------------
#include "Anything.h"
#include "Context.h"
#include "Dbg.h"

//--- test modules used --------------------------------------------------------
#include "TestSuite.h"

//--- module under test --------------------------------------------------------
#include "CgiParams.h"

//--- interface include --------------------------------------------------------
#include "CgiParamsTest.h"

//---- CgiParamsTest ----------------------------------------------------------------
CgiParamsTest::CgiParamsTest(TString tstrName) : TestCaseType(tstrName)
{
	StartTrace(CgiParamsTest.Ctor);
}

CgiParamsTest::~CgiParamsTest()
{
	StartTrace(CgiParamsTest.Dtor);
}

void CgiParamsTest::EnvironTest()
{
	StartTrace(CgiParamsTest.EnvironTest);

	CgiParams cgip("testcgi");

	MetaThing env;
	Anything content;
	content["env"] = env;
	Context ctx(content);
	Anything tmpStore(ctx.GetTmpStore());

	env["REQUEST_URI"] = "/foobaz/foo?param=x";
	env["QUERY_STRING"] = "param=x";
	String result;
	t_assertm(!cgip.Get("FileName", result, ctx), "expected Get to fail");
	assertEqual(404, ctx.Lookup("HTTPError", 200L));
	assertEqual("Not Found", ctx.Lookup("HTTPResponse", "Failed"));
	tmpStore.Remove("HTTPError");
	tmpStore.Remove("HTTPResponse");
	Anything cgienv;
	t_assertm(cgip.Get("cgienv", cgienv, ctx), "expect Get cgienv to succeed");
	t_assert(cgienv.IsDefined("QUERY_STRING"));
	t_assert(cgienv.IsDefined("SERVER_PORT"));
	t_assert(cgienv.IsDefined("SCRIPT_NAME"));
}

// builds up a suite of testcases, add a line for each testmethod
Test *CgiParamsTest::suite ()
{
	StartTrace(CgiParamsTest.suite);
	TestSuite *testSuite = new TestSuite;

	ADD_CASE(testSuite, CgiParamsTest, EnvironTest);

	return testSuite;

}
