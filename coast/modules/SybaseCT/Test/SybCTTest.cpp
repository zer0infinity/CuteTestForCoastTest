/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

//--- interface include --------------------------------------------------------
#include "SybCTTest.h"

//--- module under test --------------------------------------------------------
#include "SybCT.h"

//--- test modules used --------------------------------------------------------
#include "TestSuite.h"

//--- standard modules used ----------------------------------------------------
#include "SysLog.h"
#include "System.h"
#include "Dbg.h"

//---- SybCTTest ----------------------------------------------------------------
SybCTTest::SybCTTest(TString tstrName)
	: TestCase(tstrName)
{
	StartTrace(SybCTTest.Ctor);
}

SybCTTest::~SybCTTest()
{
	StartTrace(SybCTTest.Dtor);
}

// setup for this TestCase
void SybCTTest::setUp ()
{
	StartTrace(SybCTTest.setUp);
	if ( t_assertm( System::LoadConfigFile(fConfig, getClassName(), "any"), TString("expected ") << getClassName() << " to be readable!" ) ) {
		fTestCaseConfig = fConfig[name()];
	}
}

void SybCTTest::tearDown ()
{
	StartTrace(SybCTTest.tearDown);
}

void SybCTTest::testSybCTTest()
{
#if defined(MEM_DEBUG)
	Storage::PrintStatistic();
#endif
	{
		StartTrace(SybCTTest.testSybCTTest);
		StartTraceMem(SybCTTest.testSybCTTest);
		Anything anyMessages(Storage::Global());
		Anything anyCtxMessages(Storage::Global());
		String strInterfacesFileName = fConfig["InterfacesFile"].AsString();
		if ( t_assertm(strInterfacesFileName.Length(), "expected non-empty interfaces filename") ) {
			CS_CONTEXT *context;
			// create context
			if (t_assertm(SybCT::Init(&context, &anyCtxMessages, strInterfacesFileName) == CS_SUCCEED, "Context should have been created")) {
				SybCT sybct(context);

				if (t_assertm(sybct.Open(&anyMessages, "wdtester", "all2test", "HIKU_INT2", "testSybCTTest"), "dbOpen should have succeeded")) {
					TraceAny(anyMessages, "Messages");
					if (t_assert(sybct.SqlExec("use pub2"))) {
						if (t_assert(sybct.SqlExec("select * from authors"))) {
							Anything result, titles;
							if (t_assert(sybct.GetResult(result, titles))) {
								t_assertm(result.GetSize() == 23, "expected 23 rows");
							}
							TraceAny(result, "Query results:");
						} else {
							Trace("SqlExec2 not successful!");
						}
					} else {
						Trace("SqlExec1 not successful!");
					}
					sybct.Close();
				}
				sybct.Finis(context);
				TraceAny(anyMessages, "Messages");
				// trace messages which occurred without a connection
				while (anyCtxMessages.GetSize()) {
					SysLog::Warning(String() << anyCtxMessages[0L].AsCharPtr() << "\n");
					anyCtxMessages.Remove(0L);
				}
			}
		}
	}
#if defined(MEM_DEBUG)
	Storage::PrintStatistic();
#endif
}

void SybCTTest::LimitedMemoryTest()
{
#if defined(MEM_DEBUG)
	Storage::PrintStatistic();
#endif
	{
		StartTrace(SybCTTest.LimitedMemoryTest);
		StartTraceMem(SybCTTest.LimitedMemoryTest);
		Anything anyMessages(Storage::Global());
		Anything anyCtxMessages(Storage::Global());
		String strInterfacesFileName = fConfig["InterfacesFile"].AsString();
		if ( t_assertm(strInterfacesFileName.Length(), "expected non-empty interfaces filename") ) {
			CS_CONTEXT *context;
			// create context
			if (t_assertm(SybCT::Init(&context, &anyCtxMessages, strInterfacesFileName) == CS_SUCCEED, "Context should have been created")) {
				SybCT sybct(context);

				if (t_assertm(sybct.Open(&anyMessages, "wdtester", "all2test", "HIKU_INT2", "testSybCTTest"), "dbOpen should have succeeded")) {
					TraceAny(anyMessages, "Messages");
					if (t_assert(sybct.SqlExec("use pub2"))) {
						// we must get a failure here because of the memory limit
						if ( t_assert(sybct.SqlExec("select * from authors", "TitlesAlways", 4L) == false) ) {
							// check results even if we know that the results got limited due to the low memory limit
							Anything result, titles;
							if (t_assert(sybct.GetResult(result, titles))) {
								t_assertm(result.GetSize() == 11, "expected 11 rows");
							}
							TraceAny(result, "Query results:");
						} else {
							Trace("SqlExec2 not successful!");
						}
					} else {
						Trace("SqlExec1 not successful!");
					}
					sybct.Close();
				}
				sybct.Finis(context);
				TraceAny(anyMessages, "Messages");
				// trace messages which occurred without a connection
				while (anyCtxMessages.GetSize()) {
					SysLog::Warning(String() << anyCtxMessages[0L].AsCharPtr() << "\n");
					anyCtxMessages.Remove(0L);
				}
			}
		}
	}
#if defined(MEM_DEBUG)
	Storage::PrintStatistic();
#endif
}

// builds up a suite of testcases, add a line for each testmethod
Test *SybCTTest::suite ()
{
	StartTrace(SybCTTest.suite);
	TestSuite *testSuite = new TestSuite;

	testSuite->addTest (NEW_CASE(SybCTTest, testSybCTTest));
	testSuite->addTest (NEW_CASE(SybCTTest, LimitedMemoryTest));

	return testSuite;

} // suite
