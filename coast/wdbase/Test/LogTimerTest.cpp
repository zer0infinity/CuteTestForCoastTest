/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#include "LogTimerTest.h"
#include "Timers.h"
#include "TestSuite.h"
#include "FoundationTestTypes.h"

void LogTimerTest::MethodTimerTest() {
	StartTrace(LogTimerTest.MethodTimerTest);
	Context ctx;
	TimeLoggingModule::fgDoTiming = true;
	String msg("Test MethodTimer");
	{
		Anything expected = Anything(Anything::ArrayMarker());
		MethodTimer(NoResult, msg, ctx);
		assertAnyEqual(expected, ctx.GetTmpStore());
	}
	{
		Anything setup;
		setup["EnabledValues"]["Method"]["Test"] = true;
		setup["SimulatedValues"]["Method"]["Test"] = 10;
		Context::PushPopEntry<Anything> aEntry(ctx, "setup", setup);
		Anything expected;
		Anything data;
		data[TimeLogger::eSection] = "Method";
		data[TimeLogger::eKey] = "Test";
		data[TimeLogger::eTime] = 10L;
		data[TimeLogger::eMsg] = "Test MethodTimer";
		data[TimeLogger::eUnit] = "ms";
		data[TimeLogger::eNestingLevel] = 0L;
		expected["Log"]["Times"].Append(data);

		{
			// trigger the destructor by defining its own scope
			MethodTimer(Test, msg, ctx)
			;
		}
		assertAnyEqual(expected, ctx.GetTmpStore());
		TraceAny(ctx.GetTmpStore(), "TmpStore");
	}
	{
		ctx.GetTmpStore() = Anything(Anything::ArrayMarker());
		Anything setup;
		setup["EnabledValues"]["Method"]["Test"] = true;
		setup["SimulatedValues"]["Method"]["Test"] = 10;
		Context::PushPopEntry<Anything> aEntry(ctx, "setup", setup);
		Anything expected;
		Anything data;
		data[TimeLogger::eSection] = "Method";
		data[TimeLogger::eKey] = "Test";
		data[TimeLogger::eTime] = 10L;
		data[TimeLogger::eMsg] = "Test MethodTimer";
		data[TimeLogger::eUnit] = "ms";
		data[TimeLogger::eNestingLevel] = 0L;
		expected["Log"]["Times"].Append(data.DeepClone());
		expected["Log"]["Times"].Append(data.DeepClone());

		//call a method twice
		{
			// trigger the destructor by defining its own scope
			MethodTimer(Test, msg, ctx)
			;
		}
		{
			// trigger the destructor by defining its own scope
			MethodTimer(Test, msg, ctx)
			;
		}
		TraceAny(ctx.GetTmpStore(), "TmpStore");
		assertAnyEqual(expected, ctx.GetTmpStore());
	}
	{
		ctx.GetTmpStore() = Anything(Anything::ArrayMarker());
		Anything setup;
		setup["EnabledValues"]["Method"]["Test"]["SubA"] = true;
		setup["EnabledValues"]["Method"]["Test"]["SubB"] = true;
		setup["SimulatedValues"]["Method"]["Test"]["SubA"] = 10;
		setup["SimulatedValues"]["Method"]["Test"]["SubB"] = 10;
		Context::PushPopEntry<Anything> aEntry(ctx, "setup", setup);
		Anything expected;
		Anything dataA;
		dataA[TimeLogger::eSection] = "Method";
		dataA[TimeLogger::eKey] = "Test.SubA";
		dataA[TimeLogger::eTime] = 10L;
		dataA[TimeLogger::eMsg] = "Test MethodTimer";
		dataA[TimeLogger::eUnit] = "ms";
		dataA[TimeLogger::eNestingLevel] = 0L;
		Anything dataB(dataA.DeepClone());
		dataB[TimeLogger::eKey] = "Test.SubB";
		expected["Log"]["Times"].Append(dataA.DeepClone());
		expected["Log"]["Times"].Append(dataB.DeepClone());

		//call different methods of a class
		{
			// trigger the destructor by defining its own scope
			MethodTimer(Test.SubA, msg, ctx)
			;
		}
		{
			// trigger the destructor by defining its own scope
			MethodTimer(Test.SubB, msg, ctx)
			;
		}
		TraceAny(ctx.GetTmpStore(), "TmpStore");
		assertAnyEqual(expected, ctx.GetTmpStore());
	}
}

// builds up a suite of testcases, add a line for each testmethod
Test *LogTimerTest::suite() {
	StartTrace(LogTimerTest.suite);
	TestSuite *testSuite = new TestSuite;
	ADD_CASE(testSuite, LogTimerTest, MethodTimerTest);
	return testSuite;
}
