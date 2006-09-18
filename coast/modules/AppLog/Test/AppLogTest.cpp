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
#include "Timers.h"
#include "Server.h"
#include "Action.h"
#include "AnyIterators.h"

//--- c-library modules used ---------------------------------------------------

//---- AppLogTest ----------------------------------------------------------------
AppLogTest::AppLogTest(TString tname)
	: TestCaseType(tname)
{
	StartTrace(AppLogTest.AppLogTest);
}

AppLogTest::~AppLogTest()
{
	StartTrace(AppLogTest.Dtor);
}

void AppLogTest::ApplogModuleNotInitializedTest()
{
	StartTrace(AppLogTest.ApplogModuleNotInitializedTest);
	Server *server = NULL;
	if ( t_assert( ( server = Server::FindServer("TestServer") ) ) ) {
		Context ctx;
		ctx.SetServer(server);
		ctx.GetTmpStore()["TestMsg"] = "Access log Test 1";
		t_assertm(!AppLogModule::Log(ctx, "AccessLog"), "expected configured logger not to be found");
		t_assertm(!AppLogModule::Log(ctx, "ErrorLog"), "expected configured logger not to be found");
	}
}

void AppLogTest::LogOkTest()
{
	StartTrace(AppLogTest.LogOkTest);

	WDModule *pModule = WDModule::FindWDModule("AppLogModule");
	if ( t_assertm(pModule != NULL, "expected AppLogModule to be registered") ) {
		Server *server = NULL;
		if ( t_assert( ( server = Server::FindServer("TestServer") ) ) ) {
			Context ctx;
			ctx.SetServer(server);
			ctx.GetTmpStore()["TestMsg"] = "Access log Test 1";

			t_assertm(!AppLogModule::Log(ctx, 0), "logging with Null Pointer");

			t_assertm(AppLogModule::Log(ctx, "AccessLog"), "AccessLog 1");

			ctx.GetTmpStore()["TestMsg"] = "Access log Test 2";
			t_assertm(AppLogModule::Log(ctx, "AccessLog"), "AccessLog 2");

			ctx.GetTmpStore()["TestMsg"] = "NoHeader log Test 1";
			t_assertm(AppLogModule::Log(ctx, "NoHeaderLog"), "NoHeaderLog");
			ctx.GetTmpStore()["TestMsg"] = "";
			t_assertm(AppLogModule::Log(ctx, "NoHeaderLog"), "NoHeaderLog");

			ctx.GetTmpStore()["TestMsg"] = "EmptyHeader log Test 1";
			t_assertm(AppLogModule::Log(ctx, "EmptyHeaderLog"), "EmptyHeaderLog");
			ctx.GetTmpStore()["TestMsg"] = "";
			t_assertm(AppLogModule::Log(ctx, "EmptyHeaderLog"), "EmptyHeaderLog");

			ctx.GetTmpStore()["ErrorMsg"] = "Error 1";
			t_assertm(AppLogModule::Log(ctx, "ErrorLog"), "ErrorLog1");

			ctx.GetTmpStore()["ErrorMsg"] = "Error 2";
			t_assertm(AppLogModule::Log(ctx, "ErrorLog"), "ErrorLog 2");

			CheckFile(ctx, "AccessLog", "TestHeader\nAccess log Test 1 - Test\nAccess log Test 2 - Test\n");
			CheckFile(ctx, "NoHeaderLog", "NoHeader log Test 1\n\n");
			CheckFile(ctx, "EmptyHeaderLog", "\nEmptyHeader log Test 1\n");
			CheckFile(ctx, "ErrorLog", "ErrorlogTestHeader\nError 1 - Test\nError 2 - Test\n");

			CheckFile(ctx, "RelativeLogDir", "# Relative-File\n");
			CheckFile(ctx, "AbsoluteLogDir", "# Absolute-File\n");
		}
	}
}

void AppLogTest::LogOkToVirtualServerTest()
{
	StartTrace(AppLogTest.LogOkToVirtualServerTest);

	WDModule *pModule = WDModule::FindWDModule("AppLogModule");
	if ( t_assertm(pModule != NULL, "expected AppLogModule to be registered") ) {
		Server *server = NULL;
		if ( t_assert( ( server = Server::FindServer("AnotherServerWithoutLogConfigButShouldUseChannelsOfTestServer") ) ) ) {
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
		}
	}
}

void AppLogTest::LogRotatorTest()
{
	StartTrace(AppLogTest.LogRotatorTest);

	WDModule *pModule = WDModule::FindWDModule("AppLogModule");
	if ( t_assertm(pModule != NULL, "expected AppLogModule to be registered") ) {
		Server *server = NULL;
		Anything anyModuleConfig;
		anyModuleConfig["AppLogModule"] = GetTestCaseConfig()["AppLogModule"].DeepClone();
		// set rotation time to simulate behavior
		time_t now = time(0);
		struct tm res, *tt;
		tt = System::LocalTime(&now, &res);
		long lDeltaSec = GetTestCaseConfig()["SecondsToWaitOnRotate"].AsLong(5);
		long lRotationTime = ( ( ( ( tt->tm_hour * 60 ) + tt->tm_min ) * 60 ) + tt->tm_sec + lDeltaSec) % 86400;
		anyModuleConfig["AppLogModule"]["RotateSecond"] = lRotationTime;
		if ( t_assertm(pModule->Init(anyModuleConfig), "Module initialization should have succeeded!") ) {
			if ( t_assert( ( server = Server::FindServer("TestServer") ) ) ) {
				Context ctx;
				ctx.SetServer(server);
				ctx.GetTmpStore()["TestMsg"] = "Rotate log Test 1";
				t_assertm(AppLogModule::Log(ctx, "RotateLog"), "RotateLog Test");
				t_assertm(AppLogModule::Log(ctx, "DoNotRotateLog"), "DoNotRotateLog Test");
				// just be sure we gave some time to rotate the log
				Trace("### waiting " << (lDeltaSec * 2) << "s on rotation to happen... ###");
				Thread::Wait(lDeltaSec * 2);
				Trace("### ...waiting done ###");
				ctx.GetTmpStore()["TestMsg"] = "Rotate log Test 2";
				t_assertm(AppLogModule::Log(ctx, "RotateLog"), "RotateLog Test");

				// check for the files
				ROAnything roaFileName;
				if ( t_assert(GetTestCaseConfig().LookupPath(roaFileName, "AppLogModule.Servers.TestServer.RotateLog.FileName") ) ) {
					String strFileName = System::GetFilePath(roaFileName.AsString(), "");
					t_assertm( System::IsRegularFile(strFileName), TString("expected existing logfile [") << strFileName << "]");
					// check for rotated file in rotate-subdirectory
					AppLogChannel *pChannel = AppLogModule::FindLogger(ctx, "RotateLog");
					if ( t_assertm( pChannel != NULL, "expected valid log channel") ) {
						String logdir, rotatedir;
						if ( t_assertm(pChannel->GetLogDirectories(pChannel->GetChannelInfo(), logdir, rotatedir), "expected GetLogDirectories to be successful") ) {
							Anything anyRotateList = System::DirFileList(rotatedir, "");
							TraceAny(anyRotateList, "Entries of [" << rotatedir << "] directory");
							long lFound = 0;
							String strEntry, strRotateFileName = roaFileName.AsString();
							strRotateFileName << ".20";
							for (long lEntry = anyRotateList.GetSize() - 1; lEntry >= 0; --lEntry) {
								strEntry = anyRotateList[lEntry].AsString();
								if ( strEntry.StartsWith(strRotateFileName) ) {
									++lFound;
									Trace("entry [" << strEntry << "] matched!");
								}

							}
							assertEqualm( 1, lFound, TString("expected exactly one rotated file of [") << strFileName << "] to exist in rotate directory [" << rotatedir << "]");
						}
					}
				}
				if ( t_assert(GetTestCaseConfig().LookupPath(roaFileName, "AppLogModule.Servers.TestServer.DoNotRotateLog.FileName") ) ) {
					String strFileName = System::GetFilePath(roaFileName.AsString(), "");
					t_assertm( System::IsRegularFile(strFileName), TString("expected existing logfile [") << strFileName << "]");
					// check for non-existence of DoNotRotateLog in rotate directory
					AppLogChannel *pChannel = AppLogModule::FindLogger(ctx, "DoNotRotateLog");
					if ( t_assertm( pChannel != NULL, "expected valid log channel") ) {
						String logdir, rotatedir;
						if ( t_assertm(pChannel->GetLogDirectories(pChannel->GetChannelInfo(), logdir, rotatedir), "expected GetLogDirectories to be successful") ) {
							Anything anyRotateList = System::DirFileList(rotatedir, "");
							TraceAny(anyRotateList, "Entries of [" << rotatedir << "] directory");
							long lFound = 0;
							String strEntry, strRotateFileName = roaFileName.AsString();
							strRotateFileName << ".20";
							for (long lEntry = anyRotateList.GetSize() - 1; lEntry >= 0; --lEntry) {
								strEntry = anyRotateList[lEntry].AsString();
								if ( strEntry.StartsWith(strRotateFileName) ) {
									++lFound;
									Trace("entry [" << strEntry << "] matched!");
								}

							}
							assertEqualm( 0, lFound, TString("expected no rotated file of [") << strFileName << "] to exist in rotate directory [" << rotatedir << "]");
						}
					}
				}
			}
			t_assertm(pModule->Finis(), "Finis should have succeeded");
		}
	}
}

void AppLogTest::LogRotationTimeTest()
{
	StartTrace(AppLogTest.LogRotationTimeTest);

	WDModule *pModule = WDModule::FindWDModule("AppLogModule");
	if ( t_assertm(pModule != NULL, "expected AppLogModule to be registered") ) {
		AppLogModule *pAppLogModule = SafeCast(pModule, AppLogModule);
		if ( t_assertm(pAppLogModule != NULL, "expected type of AppLogModule to be configured") ) {
			Anything anyModuleConfig;
			anyModuleConfig["AppLogModule"] = GetTestCaseConfig()["AppLogModule"].DeepClone();

			ROAnything roaCaseConfig;
			AnyExtensions::Iterator<ROAnything> aIterator(GetTestCaseConfig()["TestCases"]);
			while ( aIterator.Next(roaCaseConfig) ) {
				anyModuleConfig["AppLogModule"]["RotateTime"] = roaCaseConfig["RotateTime"].AsString();

				if ( t_assertm(pAppLogModule->Init(anyModuleConfig), "Module initialization should have succeeded!") ) {
					if ( t_assertm(pAppLogModule->fRotator != NULL, "expected rotator instance") ) {
						assertEqualm(roaCaseConfig["ExpectedSeconds"].AsLong(-1L), pAppLogModule->fRotator->fRotateSecond, "expected same rotation second");
					}
					t_assertm(pAppLogModule->Finis(), "Finis should have succeeded");
				}
			}
		}
	}
}

void AppLogTest::LoggingActionTest()
{
	StartTrace(AppLogTest.LoggingActionTest);

	WDModule *pModule = WDModule::FindWDModule("AppLogModule");
	if ( t_assertm(pModule != NULL, "expected AppLogModule to be registered") ) {
		Server *server = NULL;
		if ( t_assert( ( server = Server::FindServer("TestServer") ) ) ) {
			Context ctx;
			ctx.SetServer(server);
			ctx.GetTmpStore()["TestMsg"] = "Action logging Test";
			String token("AccessTestAction");
			t_assertm(Action::ExecAction(token, ctx, GetConfig()[token]), "Action Logging 1");
			assertEqual("AccessTestAction", token);

			ctx.GetTmpStore()["ErrorMsg"] = "Action logging Error 1";
			token = "ErrorlogTestAction";
			t_assertm(Action::ExecAction(token, ctx, GetConfig()[token]), "Action Logging 2");
			assertEqual("ErrorlogTestAction", token);

			ctx.GetTmpStore()["ErrorMsg"] = "Action logging Error 2";
			token = "NoChannelTestAction";
			t_assertm(!Action::ExecAction(token, ctx, GetConfig()[token]), "Action Logging 3");
			assertEqual("NoChannelTestAction", token);

			CheckFile(ctx, "AccessLog2", "Access2Header\nAction logging Test - Test\n");
			CheckFile(ctx, "ErrorLog2", "Errorlog2TestHeader\nAction logging Error 1 - Test\n");
		}
	}
}

void AppLogTest::RotateSpecificLogTest()
{
	StartTrace(AppLogTest.RotateSpecificLogTest);

	WDModule *pModule = WDModule::FindWDModule("AppLogModule");
	if ( t_assertm(pModule != NULL, "expected AppLogModule to be registered") ) {
		Server *server = NULL;
		Anything anyModuleConfig;
		anyModuleConfig["AppLogModule"] = GetTestCaseConfig()["AppLogModule"].DeepClone();
		if ( t_assertm(pModule->Init(anyModuleConfig), "Module initialization should have succeeded!") ) {
			if ( t_assert( ( server = Server::FindServer("TestServer") ) ) ) {
				Context ctx;
				ctx.SetServer(server);
				ctx.GetTmpStore()["TestMsg"] = "Rotate log Test 1";
				t_assertm(AppLogModule::Log(ctx, "RotateSpecificLog"), "RotateSpecificLog Test");
				// Now request explict log rotation
				t_assertm(AppLogModule::RotateSpecificLog(ctx, "RotateSpecificLog"), "RotateSpecificLog now!");
				ctx.GetTmpStore()["TestMsg"] = "Rotate log Test 2";
				t_assertm(AppLogModule::Log(ctx, "RotateSpecificLog"), "RotateSpecificLog Test");
				// check for the files
				ROAnything roaFileName;
				if ( t_assert(GetTestCaseConfig().LookupPath(roaFileName, "AppLogModule.Servers.TestServer.RotateSpecificLog.FileName") ) ) {
					String strFileName = System::GetFilePath(roaFileName.AsString(), "");
					t_assertm( System::IsRegularFile(strFileName), TString("expected existing logfile [") << strFileName << "]");
					// check for rotated file in rotate-subdirectory
					AppLogChannel *pChannel = AppLogModule::FindLogger(ctx, "RotateSpecificLog");
					if ( t_assertm( pChannel != NULL, "expected valid log channel") ) {
						String logdir, rotatedir;
						if ( t_assertm(pChannel->GetLogDirectories(pChannel->GetChannelInfo(), logdir, rotatedir), "expected GetLogDirectories to be successful") ) {
							Anything anyRotateList = System::DirFileList(rotatedir, "");
							TraceAny(anyRotateList, "Entries of [" << rotatedir << "] directory");
							long lFound = 0;
							String strEntry, strRotateFileName = roaFileName.AsString();
							strRotateFileName << ".20";
							for (long lEntry = anyRotateList.GetSize() - 1; lEntry >= 0; --lEntry) {
								strEntry = anyRotateList[lEntry].AsString();
								if ( strEntry.StartsWith(strRotateFileName) ) {
									++lFound;
									Trace("entry [" << strEntry << "] matched!");
								}

							}
							assertEqualm( 1, lFound, TString("expected exactly one rotated file of [") << strFileName << "] to exist in rotate directory [" << rotatedir << "]");
						}
					}
				}

			}
			t_assertm(pModule->Finis(), "Finis should have succeeded");
		}
	}
}

void AppLogTest::TimeLoggingActionTest()
{
	StartTrace(AppLogTest.TimeLoggingActionTest);

	WDModule *pModule = WDModule::FindWDModule("AppLogModule");
	if ( t_assertm(pModule != NULL, "expected AppLogModule to be registered") ) {
		Server *server = NULL;
		if ( t_assert( ( server = Server::FindServer("TestServer") ) ) ) {
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
			t_assertm(Action::ExecAction(token, ctx, GetTestCaseConfig()[token]), "Action Time Logging 1");
			assertEqual("TimeLogTestAction", token);

			CheckFile(ctx, "TimeLog1", "TimeLogTestHeader\n<Method.Test.SubA>: AppLogTimeTest->10 ms\n<Method.Test.SubB>: AppLogTimeTest->10 ms\n<Request.Cycle>: AppLogTimeTest->10 ms\n");
		}
	}
}

void AppLogTest::CheckFile(Context &ctx, const char *channelname, String expected, bool bExpectSuccess)
{
	StartTrace(AppLogTest.CheckFile);

	AppLogChannel *pChannel = AppLogModule::FindLogger(ctx, channelname);
	if ( t_assertm(pChannel != NULL, "channel not found") ) {
		ROAnything roaChannelConfig = pChannel->GetChannelInfo();
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
	ADD_CASE(testSuite, AppLogTest, LogRotatorTest);
	ADD_CASE(testSuite, AppLogTest, LogRotationTimeTest);
	ADD_CASE(testSuite, AppLogTest, RotateSpecificLogTest);
	ADD_CASE(testSuite, AppLogTest, LoggingActionTest);
	ADD_CASE(testSuite, AppLogTest, TimeLoggingActionTest);
	return testSuite;
}
