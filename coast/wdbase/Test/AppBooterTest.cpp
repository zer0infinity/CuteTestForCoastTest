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
#include "AppBooter.h"

//--- interface include --------------------------------------------------------
#include "AppBooterTest.h"

//--- standard modules used ----------------------------------------------------
#include "System.h"
#include "SimpleTestApp.h"
#include "Context.h"
#include "Dbg.h"

#if defined(ONLY_STD_IOSTREAM)
using namespace std;
#endif

//--- c-library modules used ---------------------------------------------------

//---- AppBooterTest ----------------------------------------------------------------
AppBooterTest::AppBooterTest(TString tname) : TestCaseType(tname)
{
	StartTrace(AppBooterTest.Ctor);
}

AppBooterTest::~AppBooterTest()
{
	StartTrace(AppBooterTest.Dtor);
}

void AppBooterTest::HandleNullArgsTest()
{
	StartTrace(AppBooterTest.HandleNullArgsTest);
	{
		// no argument case
		Anything result;
		Anything expected;

		AppBooter appBooter;

		appBooter.HandleArgs(0L, 0, result);
		assertAnyEqualm(expected, result, "expected result to be untouched");
	}
}

void AppBooterTest::HandleUnstructuredArgsTest()
{
	StartTrace(AppBooterTest.HandleUnstructuredArgsTest);
	{
		// unstructured argument
		Anything result;
		Anything expected;
		const char *argv[] = { "foo", "bah"};
		const int argc = 2;
		expected.Append("foo");
		expected.Append("bah");

		AppBooter appBooter;

		appBooter.HandleArgs(argc, argv, result);
		assertAnyEqualm(expected, result, "expected result to contain foo and bah");
	}
}

void AppBooterTest::HandleStructuredArgsTest()
{
	StartTrace(AppBooterTest.HandleStructuredArgsTest);
	{
		// structured argument
		Anything result;
		Anything expected;
		const char *argv[] = { "test1=foo", "test2=bah"};
		const int argc = 2;
		expected["test1"] = "foo";
		expected["test2"] = "bah";

		AppBooter appBooter;

		appBooter.HandleArgs(argc, argv, result);
		assertAnyEqualm(expected, result, "expected result to contain foo and bah");
	}
	{
		// structured argument
		Anything result;
		Anything expected;
		const char *argv[] = { "test1=foo", "test2=bah", "test1=yum", "test2=grmbl"};
		const int argc = 4;
		expected["test1"] = "foo";
		expected["test2"] = "bah";
		expected["test1"].Append("yum");
		expected["test2"].Append("grmbl");

		AppBooter appBooter;

		appBooter.HandleArgs(argc, argv, result);
		assertAnyEqualm(expected, result, "expected result to contain yum and grmbl");
	}
	{
		// structured argument
		Anything result;
		Anything expected;
		const char *argv[] = { "test1={ /foo bah /yum grmbl }", "test2=bah"};
		const int argc = 2;
		expected["test1"] = "{ /foo bah /yum grmbl }";
		expected["test2"] = "bah";

		AppBooter appBooter;

		appBooter.HandleArgs(argc, argv, result);
		assertAnyEqualm(expected, result, "expected result to contain yum and grmbl");
	}
}

void AppBooterTest::PrepareBootFileLoadingTest()
{
	StartTrace(AppBooterTest.PrepareBootFileLoadingTest);

	{
		// null test
		Anything config;
		AppBooter appBooter;

		assertEqualm("Config", appBooter.PrepareBootFileLoading(config), "expected default name");

		// this tests succeeds only if WD_PATH and WD_ROOT are not set in the environment
		// it is not testable in this way since every environment is different
//		assertEqualm(".", System::GetRootDir(), "expected local directory");
//		assertEqualm(".:config:src:", System::GetPathList(), "expected default path list");
	}

	{
		// config set test
		Anything config;
		//store away actual settins
		String actualRoot = System::GetRootDir();
		String actualPath = System::GetPathList();

		config["WD_BOOTFILE"] = "MyConfig";
		config["WD_PATH"] = "app:app_src:app_log";
		config["WD_ROOT"] = "/foo/bah/end";
		AppBooter appBooter;

		assertEqualm("MyConfig", appBooter.PrepareBootFileLoading(config), "expected default name");

		// this tests succeeds only if WD_PATH and WD_ROOT are not set in the environment
		assertEqualm("/foo/bah/end", System::GetRootDir(), "expected local directory");
		assertEqualm("app:app_src:app_log", System::GetPathList(), "expected default path list");
		//restore actual settings
		System::SetRootDir(actualRoot);
		System::SetPathList(actualPath);
	}
}

void AppBooterTest::MergeConfigWithArgsTest()
{
	StartTrace(AppBooterTest.MergeConfigWithArgsTest);
	{
		// empty argument list
		Anything args;
		Anything result;
		Anything expected;

		AppBooter appBooter;

		appBooter.MergeConfigWithArgs(result, args);
		assertAnyEqualm(expected, result, "expected result to empty");
	}

	{
		// some arguments disjunct config
		Anything args;
		Anything result;
		Anything expected;

		result["config0"] = "value0";
		result["config1"] = "value1";

		args["args0"] = "argsValue0";
		args["args1"] = "argsValue1";

		expected["config0"] = "value0";
		expected["config1"] = "value1";
		expected["Arguments"]["args0"] = "argsValue0";
		expected["Arguments"]["args1"] = "argsValue1";
		//skip arg[0] at top level; it usually is the programname
//		expected["args0"]= expected["Arguments"]["args0"];
		expected["args1"] = expected["Arguments"]["args1"];

		AppBooter appBooter;

		appBooter.MergeConfigWithArgs(result, args);
		assertAnyEqualm(expected, result, "arguments in top level and in Arguments slot");
	}

	{
		// some arguments with overlapping config
		Anything args;
		Anything result;
		Anything expected;

		result["config0"] = "value0";
		result["Application"] = "value1";

		args["args0"] = "argsValue0";
		args["Application"] = "argsValue1";
		args.Append("argsValue2");

		expected["config0"] = "value0";
		expected["Application"] = "value1";
		expected["Arguments"]["args0"] = "argsValue0";
		expected["Arguments"]["Application"] = "argsValue1";
		expected["Arguments"].Append("argsValue2");
		//skip arg[0] at top level; it usually is the programname
//		expected["args0"]= expected["Arguments"]["args0"];
		expected["Application"] = expected["Arguments"]["Application"];
		expected.Append(expected["Arguments"][2]);

		AppBooter appBooter;

		appBooter.MergeConfigWithArgs(result, args);
		assertAnyEqualm(expected, result, "arguments in top level and in Arguments slot");
	}
}

void AppBooterTest::OpenLibsTest()
{
	StartTrace(AppBooterTest.OpenLibsTest);

	//write pid to file to make it usable by scripts
	ostream *os = System::OpenOStream("config/wdbasetest", "pid");

	if ( os ) {
		Trace("PID File<config/wdbasetest> opened");
		long pid = System::getpid();
		(*os) << pid;
		os->flush();
		delete os;
	}

	{
		// null test
		Anything config;
		AppBooter appBooter;

		t_assertm(appBooter.OpenLibs(config), "expected to succeed, since nothing has to be done");
	}
	{
		// fake lib test
		Anything config;
		AppBooter appBooter;
		config["DLL"].Append("libfake");

		t_assertm(!appBooter.OpenLibs(config), "expected to fail, since library does not exist");
	}
	{
		// existing already loaded lib test
		Anything config;
		AppBooter appBooter;
		config["DLL"].Append("libCoastFoundation");

		t_assertm(appBooter.OpenLibs(config), "expected to quietly ignore already loaded library");
		assertEqualm("libCoastFoundation", appBooter.fLibArray.SlotName(0L), "expected loaded library to be stored in libArray");
		t_assertm(0 != appBooter.fLibArray["libCoastFoundation"].AsLong(0), "expected to store handle in libArray");
	}
	{
		// existing lib test
		Anything config;
		AppBooter appBooter;
		config["DLL"].Append("TestLib");

		t_assertm(appBooter.OpenLibs(config), "expected to load existing library");
		assertEqualm("TestLib", appBooter.fLibArray.SlotName(0L), "expected loaded library to be stored in libArray");
		t_assertm(0 != appBooter.fLibArray["TestLib"].AsLong(0), "expected to store handle in libArray");
	}
}

void AppBooterTest::RunTest()
{
	StartTrace(AppBooterTest.RunTest);
	{
		AppBooter booter;
		SimpleTestApp *sta = (SimpleTestApp *)Application::FindApplication("SimpleTestApp");
		if (t_assert(sta != NULL)) {
			sta->SetTest(this);
		}
		fSequence[0L] = "DoGlobalInit entered";
		fSequence[1L] = "DoInit entered";
		fSequence[2L] = "DoInit left";
		fSequence[3L] = "DoGlobalInit left";
		fSequence[4L] = "DoGlobalRun entered";
		fSequence[5L] = "DoRun entered";
		fSequence[6L] = "DoRun left";
		fSequence[7L] = "DoGlobalRun left";
		fSequence[8L] = "DoGlobalTerminate entered";
		fSequence[9L] = "DoTerminate entered";
		fSequence[10L] = "DoTerminate left";
		fSequence[11L] = "DoGlobalTerminate left";
		fStep = 0;
		assertEqual(0L, booter.Run(0, 0, false));
	}
	{
		AppBooter booter;
		SimpleTestServer *sta = SafeCast(Application::FindApplication("SimpleTestServer"), SimpleTestServer);
		if (t_assert(sta != NULL)) {
			sta->SetTest(this);
		}
		fSequence[0L] = "DoGlobalInit entered";
		fSequence[1L] = "DoInit entered";
		fSequence[2L] = "DoInit left";
		fSequence[3L] = "DoGlobalInit left";
		fSequence[4L] = "DoGlobalRun entered";
		fSequence[5L] = "DoRun entered";
		fSequence[6L] = "DoRun left";
		fSequence[7L] = "DoGlobalRun left";
		fSequence[8L] = "DoGlobalTerminate entered";
		fSequence[9L] = "DoTerminate entered";
		fSequence[10L] = "DoTerminate left";
		fSequence[11L] = "DoGlobalTerminate left";
		fStep = 0;
		const char *argv[] = { "RunTest", "Application=SimpleTestServer" };
		assertEqual(0L, booter.Run(2, argv, false));
	}
}

void AppBooterTest::Method(const String &str)
{
	StartTrace1(AppBooterTest.Method, "<" << str << ">");
	assertEqual(fSequence[fStep++].AsCharPtr(""), str);
}

Test *AppBooterTest::suite ()
{
	StartTrace(AppBooterTest.suite);
	TestSuite *testSuite = new TestSuite;

	ADD_CASE(testSuite, AppBooterTest, HandleNullArgsTest);
	ADD_CASE(testSuite, AppBooterTest, HandleUnstructuredArgsTest);
	ADD_CASE(testSuite, AppBooterTest, HandleStructuredArgsTest);
	ADD_CASE(testSuite, AppBooterTest, PrepareBootFileLoadingTest);
	ADD_CASE(testSuite, AppBooterTest, MergeConfigWithArgsTest);
	ADD_CASE(testSuite, AppBooterTest, OpenLibsTest);
	ADD_CASE(testSuite, AppBooterTest, RunTest);

	return testSuite;
}
