/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#include "DbgTest.h"
#include "Dbg.h"
#include "TestSuite.h"
#include "Anything.h"

//---- DbgTest ----------------------------------------------------------------
void DbgTest::setUp() {
#ifdef COAST_TRACE
	Tracer::ExchangeConfigFile(name());
#endif
}

void DbgTest::tearDown() {
#ifdef COAST_TRACE
	// restore original Dbg.any File to not fail following tests... or wanted output
	Tracer::ExchangeConfigFile("Dbg");
#endif
}

void DbgTest::DbgTestExplicitlyEnabled()
{
#ifdef COAST_TRACE
	// test explicitly enabled switches between lower and upper bound
	assertEqual(true, TriggerEnabled(DbgTest.FirstLevelEnabled));
	assertEqual(false, TriggerEnabled(DbgTest.FirstLevelDisabled));
	assertEqual(false, TriggerEnabled(DbgTest.FirstLevelBelow));
	assertEqual(false, TriggerEnabled(DbgTest.FirstLevelAbove));
	assertEqual(false, TriggerEnabled(DbgTest.FirstLevelNegative));
	assertEqual(false, TriggerEnabled(DbgTest.FirstLevelNotDefined));

	assertEqual(true, TriggerEnabled(DbgTest.Second.SecondLevelEnabled));
	assertEqual(false, TriggerEnabled(DbgTest.Second.SecondLevelDisabled));
	assertEqual(false, TriggerEnabled(DbgTest.Second.SecondLevelBelow));
	assertEqual(false, TriggerEnabled(DbgTest.Second.SecondLevelAbove));
	assertEqual(false, TriggerEnabled(DbgTest.Second.SecondLevelNegative));
	assertEqual(false, TriggerEnabled(DbgTest.Second.SecondLevelNotDefined));

	assertEqual(true, TriggerEnabled(DbgTest.Second.Third.ThirdLevelEnabled));
	assertEqual(false, TriggerEnabled(DbgTest.Second.Third.ThirdLevelDisabled));
	assertEqual(false, TriggerEnabled(DbgTest.Second.Third.ThirdLevelBelow));
	assertEqual(false, TriggerEnabled(DbgTest.Second.Third.ThirdLevelAbove));
	assertEqual(false, TriggerEnabled(DbgTest.Second.Third.ThirdLevelNegative));
	assertEqual(false, TriggerEnabled(DbgTest.Second.Third.ThirdLevelNotDefined));

	assertEqual(false, TriggerEnabled(DbgTest.SlotNotDefined));
	assertEqual(false, TriggerEnabled(SectionNotDefined));
	assertEqual(false, TriggerEnabled(SectionNotDefined.SlotNotDefined));
#endif
}

void DbgTest::DbgTestLowerBoundZero()
{
#ifdef COAST_TRACE
	// test lower bound set to 0
	assertEqual(false, TriggerEnabled(DbgTest.FirstLevelEnabled));
	assertEqual(false, TriggerEnabled(DbgTest.FirstLevelDisabled));
	assertEqual(false, TriggerEnabled(DbgTest.FirstLevelBelow));
	assertEqual(false, TriggerEnabled(DbgTest.FirstLevelAbove));
	assertEqual(false, TriggerEnabled(DbgTest.FirstLevelNegative));
	assertEqual(false, TriggerEnabled(DbgTest.FirstLevelNotDefined));

	assertEqual(false, TriggerEnabled(DbgTest.Second.SecondLevelEnabled));
	assertEqual(false, TriggerEnabled(DbgTest.Second.SecondLevelDisabled));
	assertEqual(false, TriggerEnabled(DbgTest.Second.SecondLevelBelow));
	assertEqual(false, TriggerEnabled(DbgTest.Second.SecondLevelAbove));
	assertEqual(false, TriggerEnabled(DbgTest.Second.SecondLevelNegative));
	assertEqual(false, TriggerEnabled(DbgTest.Second.SecondLevelNotDefined));

	assertEqual(false, TriggerEnabled(DbgTest.Second.Third.ThirdLevelEnabled));
	assertEqual(false, TriggerEnabled(DbgTest.Second.Third.ThirdLevelDisabled));
	assertEqual(false, TriggerEnabled(DbgTest.Second.Third.ThirdLevelBelow));
	assertEqual(false, TriggerEnabled(DbgTest.Second.Third.ThirdLevelAbove));
	assertEqual(false, TriggerEnabled(DbgTest.Second.Third.ThirdLevelNegative));
	assertEqual(false, TriggerEnabled(DbgTest.Second.Third.ThirdLevelNotDefined));

	assertEqual(false, TriggerEnabled(DbgTest.SlotNotDefined));
	assertEqual(false, TriggerEnabled(SectionNotDefined));
	assertEqual(false, TriggerEnabled(SectionNotDefined.SlotNotDefined));
#endif
}

void DbgTest::DbgTestEnableAllFirstLevel()
{
#ifdef COAST_TRACE
	// enable all on the first level
	assertEqual(true, TriggerEnabled(DbgTest.FirstLevelEnabled));
	assertEqual(true, TriggerEnabled(DbgTest.FirstLevelDisabled));
	assertEqual(true, TriggerEnabled(DbgTest.FirstLevelBelow));
	assertEqual(true, TriggerEnabled(DbgTest.FirstLevelAbove));
	assertEqual(false, TriggerEnabled(DbgTest.FirstLevelNegative));
	assertEqual(true, TriggerEnabled(DbgTest.FirstLevelNotDefined));

	assertEqual(true, TriggerEnabled(DbgTest.Second.SecondLevelEnabled));
	assertEqual(true, TriggerEnabled(DbgTest.Second.SecondLevelDisabled));
	assertEqual(true, TriggerEnabled(DbgTest.Second.SecondLevelBelow));
	assertEqual(true, TriggerEnabled(DbgTest.Second.SecondLevelAbove));
	assertEqual(false, TriggerEnabled(DbgTest.Second.SecondLevelNegative));
	assertEqual(true, TriggerEnabled(DbgTest.Second.SecondLevelNotDefined));

	assertEqual(true, TriggerEnabled(DbgTest.Second.Third.ThirdLevelEnabled));
	assertEqual(true, TriggerEnabled(DbgTest.Second.Third.ThirdLevelDisabled));
	assertEqual(true, TriggerEnabled(DbgTest.Second.Third.ThirdLevelBelow));
	assertEqual(true, TriggerEnabled(DbgTest.Second.Third.ThirdLevelAbove));
	assertEqual(false, TriggerEnabled(DbgTest.Second.Third.ThirdLevelNegative));
	assertEqual(true, TriggerEnabled(DbgTest.Second.Third.ThirdLevelNotDefined));

	assertEqual(true, TriggerEnabled(DbgTest.SlotNotDefined));
	assertEqual(false, TriggerEnabled(SectionNotDefined));
	assertEqual(false, TriggerEnabled(SectionNotDefined.SlotNotDefined));
#endif
}

void DbgTest::DbgTestEnableAllSecondLevel()
{
#ifdef COAST_TRACE
	// enable all on the second level
	assertEqual(true, TriggerEnabled(DbgTest.FirstLevelEnabled));
	assertEqual(false, TriggerEnabled(DbgTest.FirstLevelDisabled));
	assertEqual(false, TriggerEnabled(DbgTest.FirstLevelBelow));
	assertEqual(false, TriggerEnabled(DbgTest.FirstLevelAbove));
	assertEqual(false, TriggerEnabled(DbgTest.FirstLevelNegative));
	assertEqual(false, TriggerEnabled(DbgTest.FirstLevelNotDefined));

	assertEqual(true, TriggerEnabled(DbgTest.Second.SecondLevelEnabled));
	assertEqual(true, TriggerEnabled(DbgTest.Second.SecondLevelDisabled));
	assertEqual(true, TriggerEnabled(DbgTest.Second.SecondLevelBelow));
	assertEqual(true, TriggerEnabled(DbgTest.Second.SecondLevelAbove));
	assertEqual(false, TriggerEnabled(DbgTest.Second.SecondLevelNegative));
	assertEqual(true, TriggerEnabled(DbgTest.Second.SecondLevelNotDefined));

	assertEqual(true, TriggerEnabled(DbgTest.Second.Third.ThirdLevelEnabled));
	assertEqual(true, TriggerEnabled(DbgTest.Second.Third.ThirdLevelDisabled));
	assertEqual(true, TriggerEnabled(DbgTest.Second.Third.ThirdLevelBelow));
	assertEqual(true, TriggerEnabled(DbgTest.Second.Third.ThirdLevelAbove));
	assertEqual(false, TriggerEnabled(DbgTest.Second.Third.ThirdLevelNegative));
	assertEqual(true, TriggerEnabled(DbgTest.Second.Third.ThirdLevelNotDefined));

	assertEqual(false, TriggerEnabled(DbgTest.SlotNotDefined));
	assertEqual(false, TriggerEnabled(SectionNotDefined));
	assertEqual(false, TriggerEnabled(SectionNotDefined.SlotNotDefined));
#endif
}

void DbgTest::DbgTestEnableAllThirdLevel()
{
#ifdef COAST_TRACE
	// enable all on the third level
	assertEqual(true, TriggerEnabled(DbgTest.FirstLevelEnabled));
	assertEqual(false, TriggerEnabled(DbgTest.FirstLevelDisabled));
	assertEqual(false, TriggerEnabled(DbgTest.FirstLevelBelow));
	assertEqual(false, TriggerEnabled(DbgTest.FirstLevelAbove));
	assertEqual(false, TriggerEnabled(DbgTest.FirstLevelNegative));
	assertEqual(false, TriggerEnabled(DbgTest.FirstLevelNotDefined));

	assertEqual(true, TriggerEnabled(DbgTest.Second.SecondLevelEnabled));
	assertEqual(false, TriggerEnabled(DbgTest.Second.SecondLevelDisabled));
	assertEqual(false, TriggerEnabled(DbgTest.Second.SecondLevelBelow));
	assertEqual(false, TriggerEnabled(DbgTest.Second.SecondLevelAbove));
	assertEqual(false, TriggerEnabled(DbgTest.Second.SecondLevelNegative));
	assertEqual(false, TriggerEnabled(DbgTest.Second.SecondLevelNotDefined));

	assertEqual(true, TriggerEnabled(DbgTest.Second.Third.ThirdLevelEnabled));
	assertEqual(true, TriggerEnabled(DbgTest.Second.Third.ThirdLevelDisabled));
	assertEqual(true, TriggerEnabled(DbgTest.Second.Third.ThirdLevelBelow));
	assertEqual(true, TriggerEnabled(DbgTest.Second.Third.ThirdLevelAbove));
	assertEqual(false, TriggerEnabled(DbgTest.Second.Third.ThirdLevelNegative));
	assertEqual(true, TriggerEnabled(DbgTest.Second.Third.ThirdLevelNotDefined));

	assertEqual(false, TriggerEnabled(DbgTest.SlotNotDefined));
	assertEqual(false, TriggerEnabled(SectionNotDefined));
	assertEqual(false, TriggerEnabled(SectionNotDefined.SlotNotDefined));
#endif
}

void DbgTest::DbgTestEnableAllAboveUpperBound()
{
#ifdef COAST_TRACE
	// enable all above upper bound switch
	assertEqual(true, TriggerEnabled(DbgTest.FirstLevelEnabled));
	assertEqual(false, TriggerEnabled(DbgTest.FirstLevelDisabled));
	assertEqual(false, TriggerEnabled(DbgTest.FirstLevelBelow));
	assertEqual(false, TriggerEnabled(DbgTest.FirstLevelAbove));
	assertEqual(false, TriggerEnabled(DbgTest.FirstLevelNegative));
	assertEqual(false, TriggerEnabled(DbgTest.FirstLevelNotDefined));

	assertEqual(true, TriggerEnabled(DbgTest.Second.SecondLevelEnabled));
	assertEqual(false, TriggerEnabled(DbgTest.Second.SecondLevelDisabled));
	assertEqual(false, TriggerEnabled(DbgTest.Second.SecondLevelBelow));
	assertEqual(false, TriggerEnabled(DbgTest.Second.SecondLevelAbove));
	assertEqual(false, TriggerEnabled(DbgTest.Second.SecondLevelNegative));
	assertEqual(false, TriggerEnabled(DbgTest.Second.SecondLevelNotDefined));

	assertEqual(true, TriggerEnabled(DbgTest.Second.Third.ThirdLevelEnabled));
	assertEqual(false, TriggerEnabled(DbgTest.Second.Third.ThirdLevelDisabled));
	assertEqual(false, TriggerEnabled(DbgTest.Second.Third.ThirdLevelBelow));
	assertEqual(false, TriggerEnabled(DbgTest.Second.Third.ThirdLevelAbove));
	assertEqual(false, TriggerEnabled(DbgTest.Second.Third.ThirdLevelNegative));
	assertEqual(false, TriggerEnabled(DbgTest.Second.Third.ThirdLevelNotDefined));
#endif
}

void DbgTest::DbgTestEnableAllBelowLowerBound()
{
#ifdef COAST_TRACE
	// enable all below lower bound switch
	assertEqual(true, TriggerEnabled(DbgTest.FirstLevelEnabled));
	assertEqual(false, TriggerEnabled(DbgTest.FirstLevelDisabled));
	assertEqual(false, TriggerEnabled(DbgTest.FirstLevelBelow));
	assertEqual(false, TriggerEnabled(DbgTest.FirstLevelAbove));
	assertEqual(false, TriggerEnabled(DbgTest.FirstLevelNegative));
	assertEqual(false, TriggerEnabled(DbgTest.FirstLevelNotDefined));

	assertEqual(true, TriggerEnabled(DbgTest.Second.SecondLevelEnabled));
	assertEqual(false, TriggerEnabled(DbgTest.Second.SecondLevelDisabled));
	assertEqual(false, TriggerEnabled(DbgTest.Second.SecondLevelBelow));
	assertEqual(false, TriggerEnabled(DbgTest.Second.SecondLevelAbove));
	assertEqual(false, TriggerEnabled(DbgTest.Second.SecondLevelNegative));
	assertEqual(false, TriggerEnabled(DbgTest.Second.SecondLevelNotDefined));

	assertEqual(true, TriggerEnabled(DbgTest.Second.Third.ThirdLevelEnabled));
	assertEqual(false, TriggerEnabled(DbgTest.Second.Third.ThirdLevelDisabled));
	assertEqual(false, TriggerEnabled(DbgTest.Second.Third.ThirdLevelBelow));
	assertEqual(false, TriggerEnabled(DbgTest.Second.Third.ThirdLevelAbove));
	assertEqual(false, TriggerEnabled(DbgTest.Second.Third.ThirdLevelNegative));
	assertEqual(false, TriggerEnabled(DbgTest.Second.Third.ThirdLevelNotDefined));

	assertEqual(false, TriggerEnabled(DbgTest.SlotNotDefined));
	assertEqual(false, TriggerEnabled(SectionNotDefined));
	assertEqual(false, TriggerEnabled(SectionNotDefined.SlotNotDefined));
#endif
}

void DbgTest::DbgTestEnableAllSecondAndBelowDisabled()
{
#ifdef COAST_TRACE
	// check EnableAll with exclusion and disable all below second level
	assertEqual(true, TriggerEnabled(DbgTest.FirstLevelEnabled));
	assertEqual(true, TriggerEnabled(DbgTest.FirstLevelDisabled));
	assertEqual(true, TriggerEnabled(DbgTest.FirstLevelBelow));
	assertEqual(true, TriggerEnabled(DbgTest.FirstLevelAbove));
	assertEqual(false, TriggerEnabled(DbgTest.FirstLevelNegative));
	assertEqual(true, TriggerEnabled(DbgTest.FirstLevelNotDefined));

	assertEqual(false, TriggerEnabled(DbgTest.Second.SecondLevelEnabled));
	assertEqual(false, TriggerEnabled(DbgTest.Second.SecondLevelDisabled));
	assertEqual(false, TriggerEnabled(DbgTest.Second.SecondLevelBelow));
	assertEqual(false, TriggerEnabled(DbgTest.Second.SecondLevelAbove));
	assertEqual(false, TriggerEnabled(DbgTest.Second.SecondLevelNegative));
	assertEqual(false, TriggerEnabled(DbgTest.Second.SecondLevelNotDefined));

	assertEqual(false, TriggerEnabled(DbgTest.Second.Third.ThirdLevelEnabled));
	assertEqual(false, TriggerEnabled(DbgTest.Second.Third.ThirdLevelDisabled));
	assertEqual(false, TriggerEnabled(DbgTest.Second.Third.ThirdLevelBelow));
	assertEqual(false, TriggerEnabled(DbgTest.Second.Third.ThirdLevelAbove));
	assertEqual(false, TriggerEnabled(DbgTest.Second.Third.ThirdLevelNegative));
	assertEqual(false, TriggerEnabled(DbgTest.Second.Third.ThirdLevelNotDefined));

	assertEqual(true, TriggerEnabled(DbgTest.SlotNotDefined));
	assertEqual(false, TriggerEnabled(SectionNotDefined));
	assertEqual(false, TriggerEnabled(SectionNotDefined.SlotNotDefined));
#endif
}

void DbgTest::DbgTestNotAllAboveLowerBound()
{
#ifdef COAST_TRACE
	// check EnableAll with exclusion and disable all below second level
	assertEqual(false, TriggerEnabled(BelowLowerBound.SlotNotDefined));
	assertEqual(true, TriggerEnabled(EnabledInRange.SlotNotDefined));
	assertEqual(true, TriggerEnabled(AtUpperBound.SlotNotDefined));
	assertEqual(false, TriggerEnabled(AboveUpperBound.SlotNotDefined));
	assertEqual(true, TriggerEnabled(NotAllEnabled.IamActive));
	assertEqual(false, TriggerEnabled(NotAllEnabled.SlotNotDefined));
	assertEqual(false, TriggerEnabled(Context.SlotNotDefined));
	assertEqual(true, TriggerEnabled(Context.HTMLWDDebug));
	assertEqual(true, TriggerEnabled(Context.HTMLWDDebug.TmpStore));
	assertEqual(true, TriggerEnabled(Context.HTMLWDDebug.EnvStore));
#endif
}

void DbgTest::DbgTestBug248()
{
#ifdef COAST_TRACE
	// check EnableAll with exclusion and disable all below second level
	assertEqual(false, TriggerEnabled(MyHTMLWriter.SlotNotDefined));
	assertEqual(false, TriggerEnabled(MyHTMLWriter.Tag));
	assertEqual(false, TriggerEnabled(MyHTMLWriter.Put));
	assertEqual(false, TriggerEnabled(MyHTMLWriter.Tag.NodeStack));
	assertEqual(false, TriggerEnabled(MyHTMLWriter.Tag.SlotNotDefined));
#endif
}

void DbgTest::CheckMacrosCompile()
{
	StartTrace(DbgTest.CheckMacrosCompile);
	Anything a;
	Trace("i was here");
	TraceBuf("buftrace so long", 10);
	TraceAny(a, "an any");
	SubTrace("test", "a sub trace message");
	SubTraceAny("test", a, "a sub trace any");
	SubTraceBuf("test", "a subtrace buffer", 10);
	StatTrace("test", "a stat trace", Coast::Storage::Current());
	StatTraceBuf("bli.bla", "0123456789012345", 10, Coast::Storage::Current());
	StatTraceAny("test.x", a, "an any", Coast::Storage::Current());
	TriggerEnabled(SectionNotDefined.SlotNotDefined);//lint !e522
}

Test *DbgTest::suite ()
{
	TestSuite *testSuite = new TestSuite;
	ADD_CASE(testSuite, DbgTest, CheckMacrosCompile);
	ADD_CASE(testSuite, DbgTest, DbgTestExplicitlyEnabled);
	ADD_CASE(testSuite, DbgTest, DbgTestLowerBoundZero);
	ADD_CASE(testSuite, DbgTest, DbgTestEnableAllFirstLevel);
	ADD_CASE(testSuite, DbgTest, DbgTestEnableAllSecondLevel);
	ADD_CASE(testSuite, DbgTest, DbgTestEnableAllThirdLevel);
	ADD_CASE(testSuite, DbgTest, DbgTestEnableAllAboveUpperBound);
	ADD_CASE(testSuite, DbgTest, DbgTestEnableAllBelowLowerBound);
	ADD_CASE(testSuite, DbgTest, DbgTestEnableAllSecondAndBelowDisabled);
	ADD_CASE(testSuite, DbgTest, DbgTestNotAllAboveLowerBound);
	ADD_CASE(testSuite, DbgTest, DbgTestBug248);
	return testSuite;
}
