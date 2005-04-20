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
#include "SysLog.h"
#include "System.h"
#include "Dbg.h"

//---- SybCTnewDATest ----------------------------------------------------------------
SybCTnewDATest::SybCTnewDATest(TString name) : TestCase(name)
{
	StartTrace(SybCTnewDATest.Ctor);
}

SybCTnewDATest::~SybCTnewDATest()
{
	StartTrace(SybCTnewDATest.Dtor);
}

// setup for this TestCase
void SybCTnewDATest::setUp ()
{
	StartTrace(SybCTnewDATest.setUp);
} // setUp

void SybCTnewDATest::tearDown ()
{
	StartTrace(SybCTnewDATest.tearDown);
} // tearDown

void SybCTnewDATest::InitOpenSetConPropTest()
{
#if defined(MEM_DEBUG)
	Storage::PrintStatistic();
#endif
	{
		StartTrace(SybCTnewDATest.InitOpenSetConPropTest);
		StartTraceMem(SybCTnewDATest.InitOpenSetConPropTest);
		Anything anyCtxMessages(Storage::Global());
		String strInterfacesFileName("config/interfaces");
		CS_CONTEXT *context;
		Context ctx;
		ParameterMapper aParamMapper("aParamMapper");
		ResultMapper aResultMapper("aResultMapper");
		String strDAName(name());
		// create context
		if (t_assertm(SybCTnewDA::Init(&context, &anyCtxMessages, strInterfacesFileName) == CS_SUCCEED, "Context should have been created")) {
			SybCTnewDA sybct(context);
			SybCTnewDA::DaParams myParams(&ctx, &aParamMapper, &aResultMapper, &strDAName);
			if (t_assertm(sybct.Open( myParams, "wdtester", "all2test", "HIKU_INT2", "SimpleQueryTest"), "dbOpen should have succeeded")) {
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
				SysLog::Warning(String() << anyCtxMessages[0L].AsCharPtr() << "\n");
				anyCtxMessages.Remove(0L);
			}
		}
	}
#if defined(MEM_DEBUG)
	Storage::PrintStatistic();
#endif
}

void SybCTnewDATest::SimpleQueryTest()
{
#if defined(MEM_DEBUG)
	Storage::PrintStatistic();
#endif
	{
		StartTrace(SybCTnewDATest.SimpleQueryTest);
		StartTraceMem(SybCTnewDATest.SimpleQueryTest);
		Anything anyCtxMessages(Storage::Global());
		String strInterfacesFileName("config/interfaces");
		CS_CONTEXT *context;
		// create context
		Context ctx;
		ParameterMapper aParamMapper("NewDAInpuMapper");
		aParamMapper.CheckConfig("InputMapper");
		ResultMapper aResultMapper("ListResultMapper");
		aResultMapper.CheckConfig("OutputMapper");
		String strDAName(name());
		// create context
		if (t_assertm(SybCTnewDA::Init(&context, &anyCtxMessages, strInterfacesFileName) == CS_SUCCEED, "Context should have been created")) {
			SybCTnewDA sybct(context);
			SybCTnewDA::DaParams myParams(&ctx, &aParamMapper, &aResultMapper, &strDAName);
			if (t_assertm(sybct.Open( myParams, "wdtester", "all2test", "HIKU_INT2", "SimpleQueryTest"), "dbOpen should have succeeded")) {
				if (t_assert(sybct.SqlExec(myParams, "use pub2"))) {
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
				SysLog::Warning(String() << anyCtxMessages[0L].AsCharPtr() << "\n");
				anyCtxMessages.Remove(0L);
			}
		}
	}
#if defined(MEM_DEBUG)
	Storage::PrintStatistic();
#endif
}

void SybCTnewDATest::LimitedMemoryTest()
{
#if defined(MEM_DEBUG)
	Storage::PrintStatistic();
#endif
	{
		StartTrace(SybCTnewDATest.LimitedMemoryTest);
		StartTraceMem(SybCTnewDATest.LimitedMemoryTest);
		Anything anyCtxMessages(Storage::Global());
		String strInterfacesFileName("config/interfaces");
		CS_CONTEXT *context;
		// create context
		Context ctx;
		ParameterMapper aParamMapper("NewDAInpuMapper");
		aParamMapper.CheckConfig("InputMapper");
		ResultMapper aResultMapper("ListResultMapper");
		aResultMapper.CheckConfig("OutputMapper");
		String strDAName(name());
		// create context
		if (t_assertm(SybCTnewDA::Init(&context, &anyCtxMessages, strInterfacesFileName) == CS_SUCCEED, "Context should have been created")) {
			SybCTnewDA sybct(context);
			SybCTnewDA::DaParams myParams(&ctx, &aParamMapper, &aResultMapper, &strDAName);
			if (t_assertm(sybct.Open( myParams, "wdtester", "all2test", "HIKU_INT2", "SimpleQueryTest"), "dbOpen should have succeeded")) {
				if ( t_assert(sybct.SqlExec(myParams, "use pub2")) ) {
					// we must get a failure here because of the memory limit
					if ( t_assert(sybct.SqlExec(myParams, "select * from authors", "TitlesAlways", 4L) == false) ) {
						TraceAny(ctx.GetTmpStore()["TestOutput"], "TestOutput");
						assertEqual(11, ctx.GetTmpStore()["TestOutput"]["QueryCount"].AsLong(-1));
					}
				}
				TraceAny(ctx.GetTmpStore(), "TempStore");
				sybct.Close();
			}
			sybct.Finis(context);
			TraceAny(anyCtxMessages, "Messages");
			// trace messages which occurred without a connection
			while (anyCtxMessages.GetSize()) {
				SysLog::Warning(String() << anyCtxMessages[0L].AsCharPtr() << "\n");
				anyCtxMessages.Remove(0L);
			}
		}
	}
#if defined(MEM_DEBUG)
	Storage::PrintStatistic();
#endif
}

// builds up a suite of testcases, add a line for each testmethod
Test *SybCTnewDATest::suite ()
{
	StartTrace(SybCTnewDATest.suite);
	TestSuite *testSuite = new TestSuite;

	ADD_CASE(testSuite, SybCTnewDATest, InitOpenSetConPropTest);
	ADD_CASE(testSuite, SybCTnewDATest, SimpleQueryTest);
	ADD_CASE(testSuite, SybCTnewDATest, LimitedMemoryTest);

	return testSuite;
}
