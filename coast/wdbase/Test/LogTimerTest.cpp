/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

//--- interface include --------------------------------------------------------
#include "LogTimerTest.h"

//--- module under test --------------------------------------------------------
#include "Timers.h"

//--- test modules used --------------------------------------------------------
#include "TestSuite.h"

//--- standard modules used ----------------------------------------------------
#include "Dbg.h"

//--- c-library modules used ---------------------------------------------------

//---- LogTimerTest ----------------------------------------------------------------
LogTimerTest::LogTimerTest(TString tname)
	: TestCaseType(tname)
{
	StartTrace(LogTimerTest.LogTimerTest);
}

LogTimerTest::~LogTimerTest()
{
	StartTrace(LogTimerTest.Dtor);
}

void LogTimerTest::MethodTimerTest()
{
	StartTrace(LogTimerTest.MethodTimerTest);
	Context ctx;
	String msg("Test MethodTimer");
	{
		MetaThing expected;
		MethodTimer(NoResult, msg, ctx);
		assertAnyEqual(expected, ctx.GetTmpStore());
	}
	{
		Anything setup;
		setup["EnabledValues"]["Log"]["Times"]["Method"]["Test"] = true;
		setup["SimulatedValues"]["Log"]["Times"]["Method"]["Test"] = 10;
		Context::PushPopEntry aEntry(ctx, "setup", setup);
		Anything expected;
		Anything data;
		data["Time"] = 10L;
		data["Msg"] = "Test MethodTimer";
		expected["Log"]["Times"]["Method"]["Test"].Append(data);

		{
			// trigger the destructor by defining its own scope
			MethodTimer(Test, msg, ctx);
		}
		assertAnyEqual(expected, ctx.GetTmpStore());
		TraceAny(ctx.GetTmpStore(), "TmpStore");
	}
	{
		ctx.GetTmpStore() = MetaThing();
		Anything setup;
		setup["EnabledValues"]["Log"]["Times"]["Method"]["Test"] = true;
		setup["SimulatedValues"]["Log"]["Times"]["Method"]["Test"] = 10;
		Context::PushPopEntry aEntry(ctx, "setup", setup);
		Anything expected;
		Anything data;
		data["Time"] = 10L;
		data["Msg"] = "Test MethodTimer";
		expected["Log"]["Times"]["Method"]["Test"].Append(data.DeepClone());
		expected["Log"]["Times"]["Method"]["Test"].Append(data.DeepClone());

		//call a method twice
		{
			// trigger the destructor by defining its own scope
			MethodTimer(Test, msg, ctx);
		}
		{
			// trigger the destructor by defining its own scope
			MethodTimer(Test, msg, ctx);
		}
		TraceAny(ctx.GetTmpStore(), "TmpStore");
		assertAnyEqual(expected, ctx.GetTmpStore());
	}
	{
		ctx.GetTmpStore() = MetaThing();
		Anything setup;
		setup["EnabledValues"]["Log"]["Times"]["Method"]["Test"]["SubA"] = true;
		setup["EnabledValues"]["Log"]["Times"]["Method"]["Test"]["SubB"] = true;
		setup["SimulatedValues"]["Log"]["Times"]["Method"]["Test"]["SubA"] = 10;
		setup["SimulatedValues"]["Log"]["Times"]["Method"]["Test"]["SubB"] = 10;
		Context::PushPopEntry aEntry(ctx, "setup", setup);
		Anything expected;
		Anything data;
		data["Time"] = 10L;
		data["Msg"] = "Test MethodTimer";
		expected["Log"]["Times"]["Method"]["Test"]["SubA"].Append(data.DeepClone());
		expected["Log"]["Times"]["Method"]["Test"]["SubB"].Append(data.DeepClone());

		//call different methods of a class
		{
			// trigger the destructor by defining its own scope
			MethodTimer(Test.SubA, msg, ctx);
		}
		{
			// trigger the destructor by defining its own scope
			MethodTimer(Test.SubB, msg, ctx);
		}
		TraceAny(ctx.GetTmpStore(), "TmpStore");
		assertAnyEqual(expected, ctx.GetTmpStore());
	}
}

// builds up a suite of testcases, add a line for each testmethod
Test *LogTimerTest::suite ()
{
	StartTrace(LogTimerTest.suite);
	TestSuite *testSuite = new TestSuite;
	ADD_CASE(testSuite, LogTimerTest, MethodTimerTest);
	return testSuite;
}
