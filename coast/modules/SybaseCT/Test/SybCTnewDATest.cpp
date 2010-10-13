/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

//--- interface include --------------------------------------------------------
#include "SybCTnewDATest.h"

//--- module under test --------------------------------------------------------
#include "SybCTnewDA.h"

//--- test modules used --------------------------------------------------------
#include "TestSuite.h"

//--- standard modules used ----------------------------------------------------
#include "DiffTimer.h"
#include "SystemLog.h"
#include "Dbg.h"

//---- SybCTnewDATest ----------------------------------------------------------------
SybCTnewDATest::SybCTnewDATest(TString tstrName) : TestCaseType(tstrName)
{
	StartTrace(SybCTnewDATest.Ctor);
}

SybCTnewDATest::~SybCTnewDATest()
{
	StartTrace(SybCTnewDATest.Dtor);
}

void SybCTnewDATest::InitOpenSetConPropTest()
{
	Storage::PrintStatistic();
	{
		StartTrace(SybCTnewDATest.InitOpenSetConPropTest);
		StartTraceMem(SybCTnewDATest.InitOpenSetConPropTest);
		Anything anyCtxMessages(Storage::Global());
		ROAnything roaDbParams( GetConfig()["Sybase"] );
		String strInterfacesFileName = roaDbParams["InterfacesFile"].AsString();
		if ( t_assertm(strInterfacesFileName.Length(), "expected non-empty interfaces filename") ) {
			CS_CONTEXT *context;
			Context ctx;
			ParameterMapper aParamMapper("aParamMapper");
			ResultMapper aResultMapper("aResultMapper");
			t_assert(aParamMapper.Initialize("ParameterMapper"));
			t_assert(aResultMapper.Initialize("ResultMapper"));
			String strDAName(name());
			// create context
			if (t_assertm(SybCTnewDA::Init(&context, &anyCtxMessages, strInterfacesFileName, 5) == CS_SUCCEED, "Context should have been created")) {
				SybCTnewDA sybct(context);
				SybCTnewDA::DaParams myParams(&ctx, &aParamMapper, &aResultMapper, &strDAName);
				if (t_assertm(sybct.Open( myParams, roaDbParams["Username"].AsString(),
						roaDbParams["Password"].AsString(), roaDbParams["Server"].AsString(),
						  "SimpleQueryTest"), "dbOpen should have succeeded")) {
					SybCTnewDA::DaParams outParams;
					if ( t_assertm(sybct.GetConProps(CS_USERDATA, (CS_VOID **)&outParams, CS_SIZEOF(SybCTnewDA::DaParams)) == CS_SUCCEED, "expected setting of properties to succeed") ) {
						assertEqual((long)myParams.fpContext, (long)outParams.fpContext);
						assertEqual((long)myParams.fpIn, (long)outParams.fpIn);
						assertEqual((long)myParams.fpOut, (long)outParams.fpOut);
					}
					myParams.fpContext = (Context *)12345;
					myParams.fpIn = (ParameterMapper *)7777;
					myParams.fpOut = (ResultMapper *)9999;
					if ( t_assertm(sybct.SetConProps(CS_USERDATA, (CS_VOID *)&myParams, CS_SIZEOF(SybCTnewDA::DaParams)) == CS_SUCCEED, "expected setting of properties to succeed") ) {
						if ( t_assertm(sybct.GetConProps(CS_USERDATA, (CS_VOID **)&outParams, CS_SIZEOF(SybCTnewDA::DaParams)) == CS_SUCCEED, "expected setting of properties to succeed") ) {
							assertEqual(12345, (long)outParams.fpContext);
							assertEqual(7777, (long)outParams.fpIn);
							assertEqual(9999, (long)outParams.fpOut);
						}
					}
					TraceAny(ctx.GetTmpStore(), "TempStore");
					sybct.Close();
				}
				sybct.Finis(context);
				TraceAny(anyCtxMessages, "Messages");
				// trace messages which occurred without a connection
				while (anyCtxMessages.GetSize()) {
					SystemLog::Warning(String() << anyCtxMessages[0L].AsCharPtr() << "\n");
					anyCtxMessages.Remove(0L);
				}
			}
		}
	}
	Storage::PrintStatistic();
}

void SybCTnewDATest::SimpleQueryTest()
{
	Storage::PrintStatistic();
	{
		StartTrace(SybCTnewDATest.SimpleQueryTest);
		StartTraceMem(SybCTnewDATest.SimpleQueryTest);
		Anything anyCtxMessages(Storage::Global());
		ROAnything roaDbParams( GetConfig()["Sybase"] );
		String strInterfacesFileName = roaDbParams["InterfacesFile"].AsString();
		if ( t_assertm(strInterfacesFileName.Length(), "expected non-empty interfaces filename") ) {
			CS_CONTEXT *context;
			// create context
			Context ctx;
			ParameterMapper aParamMapper("NewDAInpuMapper");
			ResultMapper aResultMapper("SybCTnewDAImpl");
			t_assert(aParamMapper.Initialize("ParameterMapper"));
			t_assert(aResultMapper.Initialize("ResultMapper"));
			String strDAName(name());
			// create context
			if (t_assertm(SybCTnewDA::Init(&context, &anyCtxMessages, strInterfacesFileName, 5) == CS_SUCCEED, "Context should have been created")) {
				SybCTnewDA sybct(context);
				SybCTnewDA::DaParams myParams(&ctx, &aParamMapper, &aResultMapper, &strDAName);
				if (t_assertm(sybct.Open( myParams, roaDbParams["Username"].AsString(),
						roaDbParams["Password"].AsString(), roaDbParams["Server"].AsString(),
						  "SimpleQueryTest"), "dbOpen should have succeeded")) {
					String useDB("use ");
					if (t_assert(sybct.SqlExec(myParams, useDB << roaDbParams["Database"].AsString()))) {
						if ( t_assert(sybct.SqlExec(myParams, "select * from authors") ) ) {
							TraceAny(ctx.GetTmpStore()["TestOutput"], "TestOutput");
							assertEqual(23, ctx.GetTmpStore()["TestOutput"]["QueryCount"].AsLong(-1));
						}
					}
					TraceAny(ctx.GetTmpStore(), "TempStore");
					sybct.Close();
				}
				sybct.Finis(context);
				TraceAny(anyCtxMessages, "Messages");
				// trace messages which occurred without a connection
				while (anyCtxMessages.GetSize()) {
					SystemLog::Warning(String() << anyCtxMessages[0L].AsCharPtr() << "\n");
					anyCtxMessages.Remove(0L);
				}
			}
		}
	}
	Storage::PrintStatistic();
}

void SybCTnewDATest::LimitedMemoryTest()
{
	Storage::PrintStatistic();
	{
		StartTrace(SybCTnewDATest.LimitedMemoryTest);
		StartTraceMem(SybCTnewDATest.LimitedMemoryTest);
		Anything anyCtxMessages(Storage::Global());
		ROAnything roaDbParams( GetConfig()["Sybase"] );
		String strInterfacesFileName = roaDbParams["InterfacesFile"].AsString();
		if ( t_assertm(strInterfacesFileName.Length(), "expected non-empty interfaces filename") ) {
			CS_CONTEXT *context;
			// create context
			Context ctx;
			ParameterMapper aParamMapper("NewDAInpuMapper");
			ResultMapper aResultMapper("SybCTnewDAImpl");
			t_assert(aParamMapper.Initialize("ParameterMapper"));
			t_assert(aResultMapper.Initialize("ResultMapper"));
			String strDAName(name());
			// create context
			if (t_assertm(SybCTnewDA::Init(&context, &anyCtxMessages, strInterfacesFileName, 5) == CS_SUCCEED, "Context should have been created")) {
				SybCTnewDA sybct(context);
				SybCTnewDA::DaParams myParams(&ctx, &aParamMapper, &aResultMapper, &strDAName);
				if (t_assertm(sybct.Open( myParams, roaDbParams["Username"].AsString(),
						roaDbParams["Password"].AsString(), roaDbParams["Server"].AsString(),
						  "SimpleQueryTest"), "dbOpen should have succeeded")) {
					String useDB("use ");
					if ( t_assert(sybct.SqlExec(myParams, useDB << roaDbParams["Database"].AsString())) ) {
						// we must get a success here even though we bailed out due to a memory limit
						if ( t_assert( sybct.SqlExec(myParams, "select * from authors", "TitlesAlways", 4L) ) ) {
							TraceAny(ctx.GetTmpStore()["TestOutput"], "TestOutput");
							assertEqual(11, ctx.GetTmpStore()["TestOutput"]["QueryCount"].AsLong(-1));
						}
						ctx.GetTmpStore()["TestOutput"] = Anything();
						// we must get a success here even though we bailed out due to a row limit
						if ( t_assert( sybct.SqlExec(myParams, "select * from authors", "TitlesAlways", 0L, 5L) ) ) {
							TraceAny(ctx.GetTmpStore()["TestOutput"], "TestOutput");
							assertEqual(5, ctx.GetTmpStore()["TestOutput"]["QueryCount"].AsLong(-1));
						}
					}
					TraceAny(ctx.GetTmpStore(), "TempStore");
					sybct.Close();
				}
				sybct.Finis(context);
				TraceAny(anyCtxMessages, "Messages");
				// trace messages which occurred without a connection
				while (anyCtxMessages.GetSize()) {
					SystemLog::Warning(String() << anyCtxMessages[0L].AsCharPtr() << "\n");
					anyCtxMessages.Remove(0L);
				}
			}
		}
	}
	Storage::PrintStatistic();
}

void SybCTnewDATest::LoginTimeoutTest()
{
	StartTrace(SybCTnewDATest.LoginTimeoutTest);
	Anything anyCtxMessages(Storage::Global());
	String strInterfacesFileName = GetConfig()["Sybase"]["InterfacesFile"].AsString();
	if ( t_assertm(strInterfacesFileName.Length(), "expected non-empty interfaces filename") ) {
		long lMaxConnections = GetTestCaseConfig()["Connections"].AsLong(26L);
		// create context
		CS_CONTEXT *context;
		if (t_assertm(SybCTnewDA::Init(&context, &anyCtxMessages, strInterfacesFileName, lMaxConnections) == CS_SUCCEED, "Context should have been created")) {
			IntLoginTimeoutTest(context, lMaxConnections, 1L);
			TraceAny(anyCtxMessages, "Messages");
			// trace messages which occurred without a connection
			while (anyCtxMessages.GetSize()) {
				SystemLog::Warning(String() << anyCtxMessages[0L].AsCharPtr() << "\n");
				anyCtxMessages.Remove(0L);
			}
			SybCTnewDA::Finis(context);
		}
	}
}

void SybCTnewDATest::IntLoginTimeoutTest(CS_CONTEXT *context, long lMaxNumber, long lCurrent)
{
	StartTrace1(SybCTnewDATest.IntLoginTimeoutTest, String() << lCurrent);
	ParameterMapper aParamMapper("LoginTimeoutMapper");
	ResultMapper aResultMapper("SybCTnewDAImpl");
	t_assert(aParamMapper.Initialize("ParameterMapper"));
	t_assert(aResultMapper.Initialize("ResultMapper"));
	SybCTnewDA sybct(context);
	Context ctx;
	String strDAName(name());
	strDAName << lCurrent;
	SybCTnewDA::DaParams myParams(&ctx, &aParamMapper, &aResultMapper, &strDAName);
	DiffTimer aTimer;
	ROAnything roaDbParams( GetConfig()["Sybase"] );
	if ( sybct.Open( myParams, roaDbParams["Username"].AsString(), roaDbParams["Password"].AsString(),
			roaDbParams["Server"].AsString(), "LoginTimeoutTest") ) {
		long lDTime = aTimer.Diff();
		long lTimeout = GetTestCaseConfig()["Timeout"].AsLong(5L) * 1000L;
		Trace("time used in Open: " << lDTime << "ms, Timeout is: " << lTimeout << "ms");
		if ( lDTime < 400L ) {
			// regular case, no login timeout
			t_assert(true);
		} else {
			// login timeout occured
			t_assertm( lDTime >= lTimeout, TString("expected timeout to be more than ") << lTimeout << "ms");
		}
		if ( lCurrent <= lMaxNumber ) {
			IntLoginTimeoutTest(context, lMaxNumber, ++lCurrent);
		}
		TraceAny(ctx.GetTmpStore(), "TempStore");
		sybct.Close();
	}
}

void SybCTnewDATest::ResultTimeoutTest()
{
	StartTrace(SybCTnewDATest.ResultTimeoutTest);
	Anything anyCtxMessages(Storage::Global());
	ROAnything roaDbParams( GetConfig()["Sybase"] );
	String strInterfacesFileName = roaDbParams["InterfacesFile"].AsString();
	if ( t_assertm(strInterfacesFileName.Length(), "expected non-empty interfaces filename") ) {
		CS_CONTEXT *context;
		// create context
		Context ctx;
		ParameterMapper aParamMapper("ResultTimeoutMapper");
		ResultMapper aResultMapper("SybCTnewDAImpl");
		t_assert(aParamMapper.Initialize("ParameterMapper"));
		t_assert(aResultMapper.Initialize("ResultMapper"));
		String strDAName(name());
		// create context
		if (t_assertm(SybCTnewDA::Init(&context, &anyCtxMessages, strInterfacesFileName, 5) == CS_SUCCEED, "Context should have been created")) {
			SybCTnewDA sybct(context);
			SybCTnewDA::DaParams myParams(&ctx, &aParamMapper, &aResultMapper, &strDAName);
			if (t_assertm(sybct.Open( myParams, roaDbParams["Username"].AsString(),
					roaDbParams["Password"].AsString(), roaDbParams["Server"].AsString(),
					"ResultTimeoutTest"), "dbOpen should have succeeded")) {
				DiffTimer aTimer;
				String useDB("use ");
				if (t_assert(sybct.SqlExec(myParams, useDB << roaDbParams["Database"].AsString()))) {
					String strQuery("exec waitSomeSeconds \"00:00:");
					strQuery << GetTestCaseConfig()["TimeToWait"].AsString("20") << "\"";
					t_assertm( !sybct.SqlExec(myParams, strQuery), "exec should have failed because of result timeout" );
					TraceAny(ctx.GetTmpStore()["TestOutput"], "TestOutput");
					long lDTime = aTimer.Diff();
					long lResultTimeout = GetTestCaseConfig()["ResultTimeout"].AsLong(10L) * 1000L;
					long lWaitTimeout = GetTestCaseConfig()["TimeToWait"].AsLong(20L) * 1000L;
					t_assertm( ( ( abs( lDTime - lResultTimeout ) < 1000 ) && ( lDTime < lWaitTimeout ) ), TString("expected elapsed time ") << lDTime << "ms to be between " << lResultTimeout << "ms and " << lWaitTimeout << "ms");
				}
				TraceAny(ctx.GetTmpStore(), "TempStore");
				sybct.Close();
			}
			sybct.Finis(context);
			TraceAny(anyCtxMessages, "Messages");
			// trace messages which occurred without a connection
			while (anyCtxMessages.GetSize()) {
				SystemLog::Warning(String() << anyCtxMessages[0L].AsCharPtr() << "\n");
				anyCtxMessages.Remove(0L);
			}
		}
	}
}

// builds up a suite of testcases, add a line for each testmethod
Test *SybCTnewDATest::suite ()
{
	StartTrace(SybCTnewDATest.suite);
	TestSuite *testSuite = new TestSuite;
	ADD_CASE(testSuite, SybCTnewDATest, InitOpenSetConPropTest);
	ADD_CASE(testSuite, SybCTnewDATest, SimpleQueryTest);
	ADD_CASE(testSuite, SybCTnewDATest, LimitedMemoryTest);
	ADD_CASE(testSuite, SybCTnewDATest, LoginTimeoutTest);
	ADD_CASE(testSuite, SybCTnewDATest, ResultTimeoutTest);
	return testSuite;
}
