/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

//--- interface include --------------------------------------------------------
#include "AppLogTest.h"

//--- module under test --------------------------------------------------------
#include "AppLog.h"

//--- test modules used --------------------------------------------------------
#include "TestSuite.h"

//--- standard modules used ----------------------------------------------------
#include "System.h"
#include "Timers.h"
#include "Server.h"
#include "Action.h"
#include "MmapStream.h"
#include "Dbg.h"

//--- c-library modules used ---------------------------------------------------

//---- AppLogTest ----------------------------------------------------------------
AppLogTest::AppLogTest(TString tname)
	: ConfiguredTestCase(tname, "Config")
{
	StartTrace(AppLogTest.Ctor);
}

AppLogTest::~AppLogTest()
{
	StartTrace(AppLogTest.Dtor);
}

void AppLogTest::ApplogModuleNotInitializedTest()
{
	StartTrace(AppLogTest.ApplogModuleNotInitializedTest);

	Server *server = Server::FindServer("TestServer");
	Context ctx;
	ctx.SetServer(server);
	ctx.GetTmpStore()["TestMsg"] = "Access log Test 1";
	t_assertm(!AppLogModule::Log(ctx, "AccessLog"), "expected configured logger not to be found");
	t_assertm(!AppLogModule::Log(ctx, "ErrorLog"), "expected configured logger not to be found");
}

void AppLogTest::LogOkTest()
{
	StartTrace(AppLogTest.LogOkTest);

	WDModule *pModule = WDModule::FindWDModule("AppLogModule");
	if ( t_assertm(pModule != NULL, "expected AppLogModule to be registered") ) {
		if ( t_assert(pModule->Init(fConfig)) ) {
			Server *server = Server::FindServer("TestServer");

			Context ctx;
			ctx.SetServer(server);
			ctx.GetTmpStore()["TestMsg"] = "Access log Test 1";

			t_assertm(!AppLogModule::Log(ctx, 0), "logging with Null Pointer");

			t_assertm(AppLogModule::Log(ctx, "AccessLog"), "AccessLog 1");

			ctx.GetTmpStore()["TestMsg"] = "Access log Test 2";
			t_assertm(AppLogModule::Log(ctx, "AccessLog"), "AccessLog 2");

			ctx.GetTmpStore()["ErrorMsg"] = "Error 1";
			t_assertm(AppLogModule::Log(ctx, "ErrorLog"), "ErrorLog1");

			ctx.GetTmpStore()["ErrorMsg"] = "Error 2";
			t_assertm(AppLogModule::Log(ctx, "ErrorLog"), "ErrorLog 2");

			CheckFile(ctx, "AccessLog", "TestHeader\nAccess log Test 1 - Test\nAccess log Test 2 - Test\n");
			CheckFile(ctx, "ErrorLog", "ErrorlogTestHeader\nError 1 - Test\nError 2 - Test\n");

			CheckFile(ctx, "RelativeLogDir", "# Relative-File\n");
			CheckFile(ctx, "AbsoluteLogDir", "# Absolute-File\n");

			pModule->Finis();
		}
	}
}

void AppLogTest::LogOkToVirtualServerTest()
{
	StartTrace(AppLogTest.LogOkToVirtualServerTest);

	WDModule *pModule = WDModule::FindWDModule("AppLogModule");
	if ( t_assertm(pModule != NULL, "expected AppLogModule to be registered") ) {
		if ( t_assert(pModule->Init(fConfig)) ) {
			Server *server = Server::FindServer("AnotherServerWithoutLogConfigButShouldUseChannelsOfTestServer");

			Context ctx;
			ctx.SetServer(server);
			ctx.GetTmpStore()["TestMsg"] = "Access log Test 1";

			t_assertm(!AppLogModule::Log(ctx, 0), "logging with Null Pointer");

			t_assertm(AppLogModule::Log(ctx, "AccessLog"), "AccessLog 1");

			ctx.GetTmpStore()["TestMsg"] = "Access log Test 2";
			t_assertm(AppLogModule::Log(ctx, "AccessLog"), "AccessLog 2");

			ctx.GetTmpStore()["ErrorMsg"] = "Error 1";
			t_assertm(AppLogModule::Log(ctx, "ErrorLog"), "ErrorLog1");

			ctx.GetTmpStore()["ErrorMsg"] = "Error 2";
			t_assertm(AppLogModule::Log(ctx, "ErrorLog"), "ErrorLog 2");

			CheckFile(ctx, "AccessLog", "TestHeader\nAccess log Test 1 - Test\nAccess log Test 2 - Test\n");
			CheckFile(ctx, "ErrorLog", "ErrorlogTestHeader\nError 1 - Test\nError 2 - Test\n");
			CheckFile(ctx, "RelativeLogDir", "# Relative-File\n");
			CheckFile(ctx, "AbsoluteLogDir", "# Absolute-File\n");

			pModule->Finis();
		}
	}
}

void AppLogTest::LoggingActionTest()
{
	StartTrace(AppLogTest.LoggingActionTest);

	WDModule *pModule = WDModule::FindWDModule("AppLogModule");
	if ( t_assertm(pModule != NULL, "expected AppLogModule to be registered") ) {
		if ( t_assert(pModule->Init(fConfig)) ) {
			Server *server = Server::FindServer("TestServer");

			Context ctx;
			ctx.SetServer(server);
			ctx.GetTmpStore()["TestMsg"] = "Action logging Test";
			String token("AccessTestAction");
			t_assertm(Action::ExecAction(token, ctx, fConfig[token]), "Action Logging 1");
			assertEqual("AccessTestAction", token);

			ctx.GetTmpStore()["ErrorMsg"] = "Action logging Error 1";
			token = "ErrorlogTestAction";
			t_assertm(Action::ExecAction(token, ctx, fConfig[token]), "Action Logging 2");
			assertEqual("ErrorlogTestAction", token);

			ctx.GetTmpStore()["ErrorMsg"] = "Action logging Error 2";
			token = "NoChannelTestAction";
			t_assertm(!Action::ExecAction(token, ctx, fConfig[token]), "Action Logging 3");
			assertEqual("NoChannelTestAction", token);

			CheckFile(ctx, "AccessLog2", "Access2Header\nAction logging Test - Test\n");
			CheckFile(ctx, "ErrorLog2", "Errorlog2TestHeader\nAction logging Error 1 - Test\n");
			pModule->Finis();
		}
	}
}

void AppLogTest::TimeLoggingActionTest()
{
	StartTrace(AppLogTest.TimeLoggingActionTest);

	WDModule *pModule = WDModule::FindWDModule("AppLogModule");
	if ( t_assertm(pModule != NULL, "expected AppLogModule to be registered") ) {
		if ( t_assert(pModule->Init(fConfig)) ) {
			Server *server = Server::FindServer("TestServer");

			Context ctx;
			ctx.SetServer(server);

			{
				String msg("AppLogTimeTest");
				ctx.GetTmpStore() = MetaThing();
				Anything expected;
				Anything data;
				data["Time"] = 10L;
				data["Msg"] = "AppLogTimeTest";
				expected["Log"]["Times"]["Method"]["Test"]["SubA"].Append(data.DeepClone());
				expected["Log"]["Times"]["Method"]["Test"]["SubB"].Append(data.DeepClone());
				expected["Log"]["Times"]["Request"]["Cycle"].Append(data.DeepClone());

				//call different methods
				{
					// trigger the destructor by defining its own scope
					MethodTimer(Test.SubA, msg, ctx);
				}
				{
					// trigger the destructor by defining its own scope
					MethodTimer(Test.SubB, msg, ctx);
				}

				{
					// trigger the destructor by defining its own scope
					RequestTimer(Cycle, msg, ctx);
				}
				TraceAny(ctx.GetTmpStore(), "TmpStore");
				assertAnyEqual(expected, ctx.GetTmpStore());
			}

			String token("TimeLogTestAction");
			t_assertm(Action::ExecAction(token, ctx, fConfig[token]), "Action Time Logging 1");
			assertEqual("TimeLogTestAction", token);

			CheckFile(ctx, "TimeLog1", "TimeLogTestHeader\n<Method.Test.SubA>: AppLogTimeTest->10 ms\n<Method.Test.SubB>: AppLogTimeTest->10 ms\n<Request.Cycle>: AppLogTimeTest->10 ms\n");
			pModule->Finis();
		}
	}
}

void AppLogTest::CheckFile(Context &ctx, const char *channelname, String expected, bool bExpectSuccess)
{
	StartTrace(AppLogTest.CheckFile);

	AppLogChannel *pChannel = AppLogModule::FindLogger(ctx, channelname);
	if ( t_assertm(pChannel != NULL, "channel not found") ) {
		ROAnything roaChannelConfig = pChannel->fChannelInfo;
		String logdir, rotatedir;
		t_assert(pChannel->GetLogDirectories(roaChannelConfig, logdir, rotatedir));

		String strLogFilename = logdir;
		strLogFilename << System::Sep() << roaChannelConfig["FileName"].AsCharPtr();
		t_assertm( System::IsRegularFile(strLogFilename), "expected log file to be there");

		iostream *fp = System::OpenIStream(strLogFilename, NULL);
		if ( t_assertm(fp != NULL, name()) ) {
			String fileContent;
			char c;
			while ( !fp->get(c).eof() ) {
				fileContent.Append(c);
			}
			Trace("Content of [" << strLogFilename << "] : [" << fileContent << "]");
			assertEqualm(expected, fileContent, name());
		}
		delete fp;
	}
}

// builds up a suite of testcases, add a line for each testmethod
Test *AppLogTest::suite ()
{
	StartTrace(AppLogTest.suite);
	TestSuite *testSuite = new TestSuite;

	ADD_CASE(testSuite, AppLogTest, ApplogModuleNotInitializedTest);
	ADD_CASE(testSuite, AppLogTest, LogOkTest);
	ADD_CASE(testSuite, AppLogTest, LogOkToVirtualServerTest);
	ADD_CASE(testSuite, AppLogTest, LoggingActionTest);
	ADD_CASE(testSuite, AppLogTest, TimeLoggingActionTest);

	return testSuite;

} // suite
