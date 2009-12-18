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
#include "AnyUtils.h"

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
		t_assertm(!AppLogModule::Log(ctx, "AccessLog", AppLogModule::eINFO), "expected configured logger not to be found");
		t_assertm(!AppLogModule::Log(ctx, "ErrorLog", AppLogModule::eINFO), "expected configured logger not to be found");
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

			t_assertm(!AppLogModule::Log(ctx, 0, AppLogModule::eINFO), "logging with Null Pointer");

			t_assertm(AppLogModule::Log(ctx, "AccessLog", AppLogModule::eINFO), "AccessLog 1");

			ctx.GetTmpStore()["TestMsg"] = "Access log Test 2";
			t_assertm(AppLogModule::Log(ctx, "AccessLog", AppLogModule::eINFO), "AccessLog 2");

			ctx.GetTmpStore()["TestMsg"] = "NoHeader log Test 1";
			t_assertm(AppLogModule::Log(ctx, "NoHeaderLog", AppLogModule::eINFO), "NoHeaderLog");
			ctx.GetTmpStore()["TestMsg"] = "";
			t_assertm(AppLogModule::Log(ctx, "NoHeaderLog", AppLogModule::eINFO), "NoHeaderLog");

			ctx.GetTmpStore()["NoRenderingLog"] = "NoRenderingLog log Test 1";
			t_assertm(AppLogModule::Log(ctx, "NoRenderingLog", AppLogModule::eINFO), "NoRenderingLog");

			ctx.GetTmpStore()["TestMsg"] = "EmptyHeader log Test 1";
			t_assertm(AppLogModule::Log(ctx, "EmptyHeaderLog", AppLogModule::eINFO), "EmptyHeaderLog");
			ctx.GetTmpStore()["TestMsg"] = "";
			t_assertm(AppLogModule::Log(ctx, "EmptyHeaderLog", AppLogModule::eINFO), "EmptyHeaderLog");

			ctx.GetTmpStore()["TestMsg"] = "NoLogItemsWrite log Test 1";
			t_assertm(AppLogModule::Log(ctx, "NoLogItemsWriteLog", AppLogModule::eINFO), "NoLogItemsWriteLog");

			ctx.GetTmpStore()["ErrorMsg"] = "Error 1";
			t_assertm(AppLogModule::Log(ctx, "ErrorLog", AppLogModule::eINFO), "ErrorLog1");

			ctx.GetTmpStore()["ErrorMsg"] = "Error 2";
			t_assertm(AppLogModule::Log(ctx, "ErrorLog", AppLogModule::eINFO), "ErrorLog 2");

			CheckFile(ctx, "AccessLog", "TestHeader\nAccess log Test 1 - Test\nAccess log Test 2 - Test\n");
			CheckFile(ctx, "NoHeaderLog", "NoHeader log Test 1\n\n");
			CheckFile(ctx, "NoRenderingLog", "NoRenderingTestHeader\nNoRenderingLog log Test 1\n");
			CheckFile(ctx, "EmptyHeaderLog", "\nEmptyHeader log Test 1\n");
			CheckFile(ctx, "ErrorLog", "ErrorlogTestHeader\nError 1 - Test\nError 2 - Test\n");
			CheckFile(ctx, "NoLogItemsWriteLog", "NoLogItemsWriteLogTestHeader\n");

			CheckFile(ctx, "RelativeLogDir", "# Relative-File\n");
			CheckFile(ctx, "AbsoluteLogDir", "# Absolute-File\n");
		}
	}
}

void AppLogTest::BufferItemsTest()
{
	StartTrace(AppLogTest.BufferItemsTest);

	WDModule *pModule = WDModule::FindWDModule("AppLogModule");
	Context ctx;
	if ( t_assertm(pModule != NULL, "expected AppLogModule to be registered") ) {
		String strLogFilename;
		Server *server = NULL;
		Anything anyModuleConfig;
		anyModuleConfig["AppLogModule"] = GetTestCaseConfig()[fName].DeepClone();
		if ( t_assertm(pModule->Init(anyModuleConfig), "Module initialization should have succeeded!") ) {
			if ( t_assert( ( server = Server::FindServer("TestServer") ) ) ) {
				ctx.SetServer(server);
				ctx.GetTmpStore()["TestMsg"] = "BufferItemsLogTest log Test 1";
				t_assertm(AppLogModule::Log(ctx, "BufferItemsLog", AppLogModule::eINFO), "BufferItems 1");
				ctx.GetTmpStore()["TestMsg"] = "BufferItemsLogTest log Test 2";
				t_assertm(AppLogModule::Log(ctx, "BufferItemsLog", AppLogModule::eINFO), "BufferItems 2");
				ctx.GetTmpStore()["TestMsg"] = "BufferItemsLogTest log Test 3";
				t_assertm(AppLogModule::Log(ctx, "BufferItemsLog", AppLogModule::eINFO), "BufferItems 3");
				ctx.GetTmpStore()["TestMsg"] = "BufferItemsLogTest log Test 4";
				t_assertm(AppLogModule::Log(ctx, "BufferItemsLog", AppLogModule::eINFO), "BufferItems 4");
				ctx.GetTmpStore()["TestMsg"] = "BufferItemsLogTest log Test 5";
				t_assertm(AppLogModule::Log(ctx, "BufferItemsLog", AppLogModule::eINFO), "BufferItems 5");

				// Only 3 messages are written to log file 'cause BufferItems is set to 3!
				CheckFile(ctx, "BufferItemsLog", "BufferItemsLogTestHeader\nBufferItemsLogTest log Test 1\n"
						  "BufferItemsLogTest log Test 2\n"
						  "BufferItemsLogTest log Test 3\n");
			}
			// Need to extract logdir from channel before it is terminated because
			// it will be passed as argument to CheckFileAfterChannelTermination
			AppLogChannel *pChannel = AppLogModule::FindLogger(ctx, "BufferItemsLog");
			ROAnything roaChannelConfig = pChannel->GetChannelInfo();
			String logdir, rotatedir;
			t_assert(pChannel->GetLogDirectories(roaChannelConfig, logdir, rotatedir));

			strLogFilename = logdir;
			strLogFilename << System::Sep() << roaChannelConfig["FileName"].AsCharPtr();
		}
		t_assertm(pModule->Finis(), "Finis should have succeeded");
		// Now the pending 2 messages are written to log file 'cause Buffers are flushed on module termination!
		CheckFileAfterChannelTermination(ctx, strLogFilename, "BufferItemsLogTestHeader\nBufferItemsLogTest log Test 1\n"
										 "BufferItemsLogTest log Test 2\n"
										 "BufferItemsLogTest log Test 3\n"
										 "BufferItemsLogTest log Test 4\n"
										 "BufferItemsLogTest log Test 5\n");
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

			t_assertm(!AppLogModule::Log(ctx, 0, AppLogModule::eINFO), "logging with Null Pointer");

			t_assertm(AppLogModule::Log(ctx, "AccessLog", AppLogModule::eINFO), "AccessLog 1");

			ctx.GetTmpStore()["TestMsg"] = "Access log Test 2";
			t_assertm(AppLogModule::Log(ctx, "AccessLog", AppLogModule::eINFO), "AccessLog 2");

			ctx.GetTmpStore()["ErrorMsg"] = "Error 1";
			t_assertm(AppLogModule::Log(ctx, "ErrorLog", AppLogModule::eINFO), "ErrorLog1");

			ctx.GetTmpStore()["ErrorMsg"] = "Error 2";
			t_assertm(AppLogModule::Log(ctx, "ErrorLog", AppLogModule::eINFO), "ErrorLog 2");

			CheckFile(ctx, "AccessLog", "TestHeader\nAccess log Test 1 - Test\nAccess log Test 2 - Test\n");
			CheckFile(ctx, "ErrorLog", "ErrorlogTestHeader\nError 1 - Test\nError 2 - Test\n");
			CheckFile(ctx, "RelativeLogDir", "# Relative-File\n");
			CheckFile(ctx, "AbsoluteLogDir", "# Absolute-File\n");
		}
	}
}

void AppLogTest::LogRotatorGmtTest()
{
	StartTrace(AppLogTest.LogRotatorGmtTest);
	LogRotatorTestsCommon();
}
void AppLogTest::LogRotatorLocalTimeTest()
{
	StartTrace(AppLogTest.LogRotatorLocalTimeTest);
	LogRotatorTestsCommon();
}

void AppLogTest::LogRotationEveryNSecondsTest()
{
	StartTrace(AppLogTest.LogRotationEveryNSecondsTest);
	LogRotatorTestsCommon();
}

void AppLogTest::LogRotatorTestsCommon()
{
	StartTrace(AppLogTest.LogRotatorTestsCommon);

	WDModule *pModule = WDModule::FindWDModule("AppLogModule");
	if ( t_assertm(pModule != NULL, "expected AppLogModule to be registered") ) {
		Server *server = NULL;
		Anything anyModuleConfig;
		anyModuleConfig["AppLogModule"] = GetTestCaseConfig()["AppLogModule"].DeepClone();
		// set rotation time to simulate behavior
		time_t now = time(0);
		struct tm res, *tt;
		if ( !(anyModuleConfig["AppLogModule"]["RotateTimeIsGmTime"].AsBool(0L)) ) {
			tt = System::LocalTime(&now, &res);
		} else {
			tt = System::GmTime(&now, &res);
		}
		long lDeltaSec = GetTestCaseConfig()["SecondsToWaitOnRotate"].AsLong(5);
		long multiRotations = GetTestCaseConfig()["MultiRotations"].AsLong(0);
		if ( !multiRotations ) {
			long lRotationTime = ( ( ( ( tt->tm_hour * 60 ) + tt->tm_min ) * 60 ) + tt->tm_sec + lDeltaSec) % 86400;
			anyModuleConfig["AppLogModule"]["RotateSecond"] = lRotationTime;
		}
		if ( t_assertm(pModule->Init(anyModuleConfig), "Module initialization should have succeeded!") ) {
			if ( t_assert( ( server = Server::FindServer("TestServer") ) ) ) {
				Context ctx;
				ctx.SetServer(server);
				ctx.GetTmpStore()["TestMsg"] = "Rotate log Test 1";
				t_assertm(AppLogModule::Log(ctx, "RotateLog", AppLogModule::eINFO), "RotateLog Test");
				t_assertm(AppLogModule::Log(ctx, "DoNotRotateLog", AppLogModule::eINFO), "DoNotRotateLog Test");
				// just be sure we gave some time to rotate the log
				Trace("### waiting " << (lDeltaSec * 2) << "s on rotation to happen... ###");
				Thread::Wait(lDeltaSec * 2);
				Trace("### ...waiting done ###");
				ctx.GetTmpStore()["TestMsg"] = "Rotate log Test 2";
				t_assertm(AppLogModule::Log(ctx, "RotateLog", AppLogModule::eINFO), "RotateLog Test");

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
							if ( !multiRotations ) {
								assertEqualm( 1, lFound, TString("expected exactly one rotated file of [") << strFileName << "] to exist in rotate directory [" << rotatedir << "]");
							} else {
								t_assertm(lFound >= multiRotations, TString("Minimum :[ ") << multiRotations << "] rotates expected, but found [" << lFound << "]");
							}
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
				t_assertm(AppLogModule::Log(ctx, "RotateSpecificLog", AppLogModule::eINFO), "RotateSpecificLog Test");
				// Now request explict log rotation
				t_assertm(AppLogModule::RotateSpecificLog(ctx, "RotateSpecificLog"), "RotateSpecificLog now!");
				ctx.GetTmpStore()["TestMsg"] = "Rotate log Test 2";
				t_assertm(AppLogModule::Log(ctx, "RotateSpecificLog", AppLogModule::eINFO), "RotateSpecificLog Test");
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
				// basically the same as above, but DoNotRotate flag is set in log channel config which will be
				// overriden by explicitly requesting a log rotation
				ctx.GetTmpStore()["TestMsg"] = "RotateOverride log Test 1";
				t_assertm(AppLogModule::Log(ctx, "RotateSpecificLogOverride", AppLogModule::eINFO), "RotateSpecificLogOverride Test");
				// Now request explict log rotation
				t_assertm(AppLogModule::RotateSpecificLog(ctx, "RotateSpecificLogOverride"), "RotateSpecificLogOverride now!");
				ctx.GetTmpStore()["TestMsg"] = "RotateOverride log Test 2";
				t_assertm(AppLogModule::Log(ctx, "RotateSpecificLogOverride", AppLogModule::eINFO), "RotateSpecificLogOverride Test");
				// check for the files
				if ( t_assert(GetTestCaseConfig().LookupPath(roaFileName, "AppLogModule.Servers.TestServer.RotateSpecificLogOverride.FileName") ) ) {
					String strFileName = System::GetFilePath(roaFileName.AsString(), "");
					t_assertm( System::IsRegularFile(strFileName), TString("expected existing logfile [") << strFileName << "]");
					// check for rotated file in rotate-subdirectory
					AppLogChannel *pChannel = AppLogModule::FindLogger(ctx, "RotateSpecificLogOverride");
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
				Anything dataA, dataB, dataBA, dataBB, dataBAA, dataC, *data;
				dataA[TimeLogger::eTime] = 10L;
				dataA[TimeLogger::eMsg] = "AppLogTimeTest";
				dataA[TimeLogger::eUnit] = "ms";
				dataA[TimeLogger::eNestingLevel] = 0;
				dataB = dataA.DeepClone();
				dataBAA = dataA.DeepClone();
				dataBA = dataA.DeepClone();
				dataBB = dataA.DeepClone();
				dataC = dataA.DeepClone();
				data = &dataA;
				(*data)[TimeLogger::eKey] = "Test.SubA";
				(*data)[TimeLogger::eSection] = "Method";
				data = &dataB;
				(*data)[TimeLogger::eKey] = "Test.SubB";
				(*data)[TimeLogger::eSection] = "Method";
				data = &dataC;
				(*data)[TimeLogger::eKey] = "Cycle";
				(*data)[TimeLogger::eSection] = "Request";
				data = &dataBA;
				(*data)[TimeLogger::eKey] = "Test.SubBA";
				(*data)[TimeLogger::eSection] = "Method";
				(*data)[TimeLogger::eNestingLevel] = 1;
				(*data)[TimeLogger::eTime] = 2L;
				data = &dataBB;
				(*data)[TimeLogger::eKey] = "Test.SubBB";
				(*data)[TimeLogger::eSection] = "Method";
				(*data)[TimeLogger::eNestingLevel] = 1;
				(*data)[TimeLogger::eTime] = 2L;
				data = &dataBAA;
				(*data)[TimeLogger::eKey] = "Test.SubBAA";
				(*data)[TimeLogger::eSection] = "Method";
				(*data)[TimeLogger::eUnit] = "us";
				(*data)[TimeLogger::eNestingLevel] = 2;
				(*data)[TimeLogger::eTime] = 2L;
				expected.Append(dataA);
				expected.Append(dataBB);
				expected.Append(dataBAA);
				expected.Append(dataBA);
				expected.Append(dataB);
				expected.Append(dataC);

				//call different methods
				{
					// trigger the destructor by defining its own scope
					MethodTimer(Test.SubA, msg, ctx);
				}
				{
					// trigger the destructor by defining its own scope
					MethodTimer(Test.SubB, msg, ctx);
					{
						{
							// trigger the destructor by defining its own scope
							MethodTimer(Test.SubBA, msg, ctx);
							{
								// trigger the destructor by defining its own scope
								MethodTimerUnit(Test.SubBAA, msg, ctx, TimeLogger::eMicroseconds);
							}
						}
						{
							// trigger the destructor by defining its own scope
							MethodTimer(Test.SubBB, msg, ctx);
						}
					}
				}
				{
					// trigger the destructor by defining its own scope
					RequestTimer(Cycle, msg, ctx);
				}
				TraceAny(ctx.GetTmpStore(), "TmpStore");
				TraceAny(expected, "TmpStore");
				assertAnyCompareEqual(expected, ctx.Lookup("Log.Times"), "TmpStore.Log.Times", '.', ':');
			}

			String token("TimeLogTestAction");
			t_assertm(Action::ExecAction(token, ctx, GetTestCaseConfig()[token]), "Action Time Logging 1");
			assertEqual("TimeLogTestAction", token);

			CheckFile(ctx, "TimeLog1", "TimeLogTestHeader\n"
					  "<Test.SubA>: AppLogTimeTest->10 ms 0\n"
					  "<Test.SubBAA>: AppLogTimeTest->2 us 2\n"
					  "<Test.SubBA>: AppLogTimeTest->2 ms 1\n"
					  "<Test.SubBB>: AppLogTimeTest->2 ms 1\n"
					  "<Test.SubB>: AppLogTimeTest->10 ms 0\n"
					  "<Cycle>: AppLogTimeTest->10 ms 0\n");
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

void AppLogTest::CheckFileAfterChannelTermination(Context &ctx, const char *strLogFilename, String expected, bool bExpectSuccess)
{
	StartTrace(AppLogTest.CheckFileAfterChannelTermination);
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

// builds up a suite of testcases, add a line for each testmethod
Test *AppLogTest::suite ()
{
	StartTrace(AppLogTest.suite);
	TestSuite *testSuite = new TestSuite;
	ADD_CASE(testSuite, AppLogTest, ApplogModuleNotInitializedTest);
	ADD_CASE(testSuite, AppLogTest, LogOkTest);
	ADD_CASE(testSuite, AppLogTest, BufferItemsTest);
	ADD_CASE(testSuite, AppLogTest, LogOkToVirtualServerTest);
	ADD_CASE(testSuite, AppLogTest, LogRotatorLocalTimeTest);
	ADD_CASE(testSuite, AppLogTest, LogRotatorGmtTest);
	ADD_CASE(testSuite, AppLogTest, LogRotationTimeTest);
	ADD_CASE(testSuite, AppLogTest, LogRotationEveryNSecondsTest);
	ADD_CASE(testSuite, AppLogTest, RotateSpecificLogTest);
	ADD_CASE(testSuite, AppLogTest, LoggingActionTest);
	ADD_CASE(testSuite, AppLogTest, TimeLoggingActionTest);
	return testSuite;
}
