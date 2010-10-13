/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

//--- test modules used --------------------------------------------------------
#include "TestSuite.h"

//--- module under test --------------------------------------------------------
#include "URI2FileNameMapper.h"

//--- interface include --------------------------------------------------------
#include "URI2FileNameTest.h"

//--- standard modules used ----------------------------------------------------
#include "SystemFile.h"
#include "Dbg.h"

using namespace Coast;

//--- c-library modules used ---------------------------------------------------

//---- URI2FileNameTest ----------------------------------------------------------------
URI2FileNameTest::URI2FileNameTest(TString tname) : TestCaseType(tname)
{
	StartTrace(URI2FileNameTest.Ctor);
}

URI2FileNameTest::~URI2FileNameTest()
{
	StartTrace(URI2FileNameTest.Dtor);
}

void URI2FileNameTest::Uri2FilenameTest()
{
	StartTrace(URI2FileNameTest.Uri2FilenameTest);
	MetaThing env;
	Anything content;
	content["env"] = env;
	Context ctx(content);
	Anything tmpStore(ctx.GetTmpStore());

	URI2FileNameMapper tst("test");
	String result, expected;
	String rootdir = System::GetRootDir();

	// uri == "/foobaz/foo/" DocumentRoot not set -> should prepend COAST_ROOT
	// call should fail and set error state
	env["REQUEST_URI"] = "/foobaz/foo/";
	t_assertm(!tst.Get("FileName", result, ctx), "expected Get to fail");
	assertEqual(404, ctx.Lookup("HTTPError", 200L));
	assertEqual("Not Found", ctx.Lookup("HTTPResponse", "Failed"));
	tmpStore.Remove("HTTPError");
	tmpStore.Remove("HTTPResponse");

	// uri == "/foobaz/foo" DocumentRoot not set -> should prepend COAST_ROOT
	// call should fail and set error state
	env["REQUEST_URI"] = "/foobaz/foo";
	t_assertm(!tst.Get("FileName", result, ctx), "expected Get to fail");
	assertEqual(404, ctx.Lookup("HTTPError", 200L));
	assertEqual("Not Found", ctx.Lookup("HTTPResponse", "Failed"));
	tmpStore.Remove("HTTPError");
	tmpStore.Remove("HTTPResponse");

	// uri == "/" DocumentRoot set relativ -> should prepend COAST_ROOT
	env["REQUEST_URI"] = "/config/";
	t_assertm(tst.Get("FileName", result, ctx), "expected Get to succeed");
	expected = rootdir;
	expected << "/config/index.html";
	assertEqual(expected, result);

	//check error settings: they should not be set
	assertEqual(200, ctx.Lookup("HTTPError", 200L));
	assertEqual("not set", ctx.Lookup("HTTPResponse", "not set"));
	tmpStore.Remove("HTTPError");
	tmpStore.Remove("HTTPResponse");

	// uri == "/" DocumentRoot set relativ -> should prepend COAST_ROOT
	env["REQUEST_URI"] = "/config";
	t_assertm(!tst.Get("FileName", result, ctx), "expected Get to fail");

	//check error settings: they should not be set
	assertEqual(301, ctx.Lookup("HTTPError", 200L));
	assertEqual("Permanently moved", ctx.Lookup("HTTPResponse", "not set"));
	expected = rootdir;
	expected << "/config/";
	assertEqual(expected, ctx.Lookup("HTTPLocation", "foo"));
	tmpStore.Remove("HTTPError");
	tmpStore.Remove("HTTPResponse");

	// uri == "/" DocumentRoot set relativ -> should prepend COAST_ROOT
	env["DocumentRoot"] = "config";
	env["DirectoryIndex"] = "Config.any";

	env["REQUEST_URI"] = "/";
	t_assertm(tst.Get("FileName", result, ctx), "expected Get to succeed");
	expected = rootdir;
	expected << "/config/Config.any";
	assertEqual(expected, result);

	//check error settings: they should not be set
	assertEqual(200, ctx.Lookup("HTTPError", 200L));
	assertEqual("not set", ctx.Lookup("HTTPResponse", "not set"));
	tmpStore.Remove("HTTPError");
	tmpStore.Remove("HTTPResponse");

	String pwd;
	System::GetCWD(pwd);
	System::ResolvePath(pwd);
#if defined(WIN32)
	char drive;
	if ( System::GetDriveLetter(pwd, drive)) {
		String drvrel;
		drvrel << drive << ":" << "config";
		env["DocumentRoot"] = drvrel;
		expected = drvrel << "/Config.any";
		t_assertm(tst.Get("FileName", result, ctx), "expected Get to succeed");
		assertEqual(expected, result);
		//check error settings: they should not be set
		assertEqual(200, ctx.Lookup("HTTPError", 200L));
		assertEqual("not set", ctx.Lookup("HTTPResponse", "not set"));
		tmpStore.Remove("HTTPError");
		tmpStore.Remove("HTTPResponse");
	}
#endif

	// uri == "/" DocumentRoot set absolute -> should not prepend COAST_ROOT
	env["DocumentRoot"] = pwd;
	env["DirectoryIndex"] = "Config.any";
	env["REQUEST_URI"] = "/config/";

	t_assertm(tst.Get("FileName", result, ctx), "expected Get to succeed");
	expected = pwd;
	expected << "/config/Config.any";
	assertEqual(expected, result);
}

// builds up a suite of testcases, add a line for each testmethod
Test *URI2FileNameTest::suite ()
{
	StartTrace(URI2FileNameTest.suite);
	TestSuite *testSuite = new TestSuite;
	ADD_CASE(testSuite, URI2FileNameTest, Uri2FilenameTest);
	return testSuite;
}
