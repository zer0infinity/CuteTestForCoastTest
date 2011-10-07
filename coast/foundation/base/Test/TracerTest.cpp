/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#include "TracerTest.h"
#include "Tracer.h"
#include "TestSuite.h"
#include "Anything.h"

void TracerTest::setUp() {
#ifdef COAST_TRACE
	Tracer::ExchangeConfigFile(name());
#endif
}

void TracerTest::tearDown() {
#ifdef COAST_TRACE
	// restore original Tracer.any File to not fail following tests... or wanted output
	Tracer::ExchangeConfigFile("Tracer");
#endif
}

void TracerTest::TracerTestExplicitlyEnabled()
{
#ifdef COAST_TRACE
	// test explicitly enabled switches between lower and upper bound
	assertEqual(true, TriggerEnabled(TracerTest.FirstLevelEnabled));
	assertEqual(false, TriggerEnabled(TracerTest.FirstLevelDisabled));
	assertEqual(false, TriggerEnabled(TracerTest.FirstLevelBelow));
	assertEqual(false, TriggerEnabled(TracerTest.FirstLevelAbove));
	assertEqual(false, TriggerEnabled(TracerTest.FirstLevelNegative));
	assertEqual(false, TriggerEnabled(TracerTest.FirstLevelNotDefined));

	assertEqual(true, TriggerEnabled(TracerTest.Second.SecondLevelEnabled));
	assertEqual(false, TriggerEnabled(TracerTest.Second.SecondLevelDisabled));
	assertEqual(false, TriggerEnabled(TracerTest.Second.SecondLevelBelow));
	assertEqual(false, TriggerEnabled(TracerTest.Second.SecondLevelAbove));
	assertEqual(false, TriggerEnabled(TracerTest.Second.SecondLevelNegative));
	assertEqual(false, TriggerEnabled(TracerTest.Second.SecondLevelNotDefined));

	assertEqual(true, TriggerEnabled(TracerTest.Second.Third.ThirdLevelEnabled));
	assertEqual(false, TriggerEnabled(TracerTest.Second.Third.ThirdLevelDisabled));
	assertEqual(false, TriggerEnabled(TracerTest.Second.Third.ThirdLevelBelow));
	assertEqual(false, TriggerEnabled(TracerTest.Second.Third.ThirdLevelAbove));
	assertEqual(false, TriggerEnabled(TracerTest.Second.Third.ThirdLevelNegative));
	assertEqual(false, TriggerEnabled(TracerTest.Second.Third.ThirdLevelNotDefined));

	assertEqual(false, TriggerEnabled(TracerTest.SlotNotDefined));
	assertEqual(false, TriggerEnabled(SectionNotDefined));
	assertEqual(false, TriggerEnabled(SectionNotDefined.SlotNotDefined));
#endif
}

void TracerTest::TracerTestLowerBoundZero()
{
#ifdef COAST_TRACE
	// test lower bound set to 0
	assertEqual(false, TriggerEnabled(TracerTest.FirstLevelEnabled));
	assertEqual(false, TriggerEnabled(TracerTest.FirstLevelDisabled));
	assertEqual(false, TriggerEnabled(TracerTest.FirstLevelBelow));
	assertEqual(false, TriggerEnabled(TracerTest.FirstLevelAbove));
	assertEqual(false, TriggerEnabled(TracerTest.FirstLevelNegative));
	assertEqual(false, TriggerEnabled(TracerTest.FirstLevelNotDefined));

	assertEqual(false, TriggerEnabled(TracerTest.Second.SecondLevelEnabled));
	assertEqual(false, TriggerEnabled(TracerTest.Second.SecondLevelDisabled));
	assertEqual(false, TriggerEnabled(TracerTest.Second.SecondLevelBelow));
	assertEqual(false, TriggerEnabled(TracerTest.Second.SecondLevelAbove));
	assertEqual(false, TriggerEnabled(TracerTest.Second.SecondLevelNegative));
	assertEqual(false, TriggerEnabled(TracerTest.Second.SecondLevelNotDefined));

	assertEqual(false, TriggerEnabled(TracerTest.Second.Third.ThirdLevelEnabled));
	assertEqual(false, TriggerEnabled(TracerTest.Second.Third.ThirdLevelDisabled));
	assertEqual(false, TriggerEnabled(TracerTest.Second.Third.ThirdLevelBelow));
	assertEqual(false, TriggerEnabled(TracerTest.Second.Third.ThirdLevelAbove));
	assertEqual(false, TriggerEnabled(TracerTest.Second.Third.ThirdLevelNegative));
	assertEqual(false, TriggerEnabled(TracerTest.Second.Third.ThirdLevelNotDefined));

	assertEqual(false, TriggerEnabled(TracerTest.SlotNotDefined));
	assertEqual(false, TriggerEnabled(SectionNotDefined));
	assertEqual(false, TriggerEnabled(SectionNotDefined.SlotNotDefined));
#endif
}

void TracerTest::TracerTestEnableAllFirstLevel()
{
#ifdef COAST_TRACE
	// enable all on the first level
	assertEqual(true, TriggerEnabled(TracerTest.FirstLevelEnabled));
	assertEqual(true, TriggerEnabled(TracerTest.FirstLevelDisabled));
	assertEqual(true, TriggerEnabled(TracerTest.FirstLevelBelow));
	assertEqual(true, TriggerEnabled(TracerTest.FirstLevelAbove));
	assertEqual(false, TriggerEnabled(TracerTest.FirstLevelNegative));
	assertEqual(true, TriggerEnabled(TracerTest.FirstLevelNotDefined));

	assertEqual(true, TriggerEnabled(TracerTest.Second.SecondLevelEnabled));
	assertEqual(true, TriggerEnabled(TracerTest.Second.SecondLevelDisabled));
	assertEqual(true, TriggerEnabled(TracerTest.Second.SecondLevelBelow));
	assertEqual(true, TriggerEnabled(TracerTest.Second.SecondLevelAbove));
	assertEqual(false, TriggerEnabled(TracerTest.Second.SecondLevelNegative));
	assertEqual(true, TriggerEnabled(TracerTest.Second.SecondLevelNotDefined));

	assertEqual(true, TriggerEnabled(TracerTest.Second.Third.ThirdLevelEnabled));
	assertEqual(true, TriggerEnabled(TracerTest.Second.Third.ThirdLevelDisabled));
	assertEqual(true, TriggerEnabled(TracerTest.Second.Third.ThirdLevelBelow));
	assertEqual(true, TriggerEnabled(TracerTest.Second.Third.ThirdLevelAbove));
	assertEqual(false, TriggerEnabled(TracerTest.Second.Third.ThirdLevelNegative));
	assertEqual(true, TriggerEnabled(TracerTest.Second.Third.ThirdLevelNotDefined));

	assertEqual(true, TriggerEnabled(TracerTest.SlotNotDefined));
	assertEqual(false, TriggerEnabled(SectionNotDefined));
	assertEqual(false, TriggerEnabled(SectionNotDefined.SlotNotDefined));
#endif
}

void TracerTest::TracerTestEnableAllSecondLevel()
{
#ifdef COAST_TRACE
	// enable all on the second level
	assertEqual(true, TriggerEnabled(TracerTest.FirstLevelEnabled));
	assertEqual(false, TriggerEnabled(TracerTest.FirstLevelDisabled));
	assertEqual(false, TriggerEnabled(TracerTest.FirstLevelBelow));
	assertEqual(false, TriggerEnabled(TracerTest.FirstLevelAbove));
	assertEqual(false, TriggerEnabled(TracerTest.FirstLevelNegative));
	assertEqual(false, TriggerEnabled(TracerTest.FirstLevelNotDefined));

	assertEqual(true, TriggerEnabled(TracerTest.Second.SecondLevelEnabled));
	assertEqual(true, TriggerEnabled(TracerTest.Second.SecondLevelDisabled));
	assertEqual(true, TriggerEnabled(TracerTest.Second.SecondLevelBelow));
	assertEqual(true, TriggerEnabled(TracerTest.Second.SecondLevelAbove));
	assertEqual(false, TriggerEnabled(TracerTest.Second.SecondLevelNegative));
	assertEqual(true, TriggerEnabled(TracerTest.Second.SecondLevelNotDefined));

	assertEqual(true, TriggerEnabled(TracerTest.Second.Third.ThirdLevelEnabled));
	assertEqual(true, TriggerEnabled(TracerTest.Second.Third.ThirdLevelDisabled));
	assertEqual(true, TriggerEnabled(TracerTest.Second.Third.ThirdLevelBelow));
	assertEqual(true, TriggerEnabled(TracerTest.Second.Third.ThirdLevelAbove));
	assertEqual(false, TriggerEnabled(TracerTest.Second.Third.ThirdLevelNegative));
	assertEqual(true, TriggerEnabled(TracerTest.Second.Third.ThirdLevelNotDefined));

	assertEqual(false, TriggerEnabled(TracerTest.SlotNotDefined));
	assertEqual(false, TriggerEnabled(SectionNotDefined));
	assertEqual(false, TriggerEnabled(SectionNotDefined.SlotNotDefined));
#endif
}

void TracerTest::TracerTestEnableAllThirdLevel()
{
#ifdef COAST_TRACE
	// enable all on the third level
	assertEqual(true, TriggerEnabled(TracerTest.FirstLevelEnabled));
	assertEqual(false, TriggerEnabled(TracerTest.FirstLevelDisabled));
	assertEqual(false, TriggerEnabled(TracerTest.FirstLevelBelow));
	assertEqual(false, TriggerEnabled(TracerTest.FirstLevelAbove));
	assertEqual(false, TriggerEnabled(TracerTest.FirstLevelNegative));
	assertEqual(false, TriggerEnabled(TracerTest.FirstLevelNotDefined));

	assertEqual(true, TriggerEnabled(TracerTest.Second.SecondLevelEnabled));
	assertEqual(false, TriggerEnabled(TracerTest.Second.SecondLevelDisabled));
	assertEqual(false, TriggerEnabled(TracerTest.Second.SecondLevelBelow));
	assertEqual(false, TriggerEnabled(TracerTest.Second.SecondLevelAbove));
	assertEqual(false, TriggerEnabled(TracerTest.Second.SecondLevelNegative));
	assertEqual(false, TriggerEnabled(TracerTest.Second.SecondLevelNotDefined));

	assertEqual(true, TriggerEnabled(TracerTest.Second.Third.ThirdLevelEnabled));
	assertEqual(true, TriggerEnabled(TracerTest.Second.Third.ThirdLevelDisabled));
	assertEqual(true, TriggerEnabled(TracerTest.Second.Third.ThirdLevelBelow));
	assertEqual(true, TriggerEnabled(TracerTest.Second.Third.ThirdLevelAbove));
	assertEqual(false, TriggerEnabled(TracerTest.Second.Third.ThirdLevelNegative));
	assertEqual(true, TriggerEnabled(TracerTest.Second.Third.ThirdLevelNotDefined));

	assertEqual(false, TriggerEnabled(TracerTest.SlotNotDefined));
	assertEqual(false, TriggerEnabled(SectionNotDefined));
	assertEqual(false, TriggerEnabled(SectionNotDefined.SlotNotDefined));
#endif
}

void TracerTest::TracerTestEnableAllAboveUpperBound()
{
#ifdef COAST_TRACE
	// enable all above upper bound switch
	assertEqual(true, TriggerEnabled(TracerTest.FirstLevelEnabled));
	assertEqual(false, TriggerEnabled(TracerTest.FirstLevelDisabled));
	assertEqual(false, TriggerEnabled(TracerTest.FirstLevelBelow));
	assertEqual(false, TriggerEnabled(TracerTest.FirstLevelAbove));
	assertEqual(false, TriggerEnabled(TracerTest.FirstLevelNegative));
	assertEqual(false, TriggerEnabled(TracerTest.FirstLevelNotDefined));

	assertEqual(true, TriggerEnabled(TracerTest.Second.SecondLevelEnabled));
	assertEqual(false, TriggerEnabled(TracerTest.Second.SecondLevelDisabled));
	assertEqual(false, TriggerEnabled(TracerTest.Second.SecondLevelBelow));
	assertEqual(false, TriggerEnabled(TracerTest.Second.SecondLevelAbove));
	assertEqual(false, TriggerEnabled(TracerTest.Second.SecondLevelNegative));
	assertEqual(false, TriggerEnabled(TracerTest.Second.SecondLevelNotDefined));

	assertEqual(true, TriggerEnabled(TracerTest.Second.Third.ThirdLevelEnabled));
	assertEqual(false, TriggerEnabled(TracerTest.Second.Third.ThirdLevelDisabled));
	assertEqual(false, TriggerEnabled(TracerTest.Second.Third.ThirdLevelBelow));
	assertEqual(false, TriggerEnabled(TracerTest.Second.Third.ThirdLevelAbove));
	assertEqual(false, TriggerEnabled(TracerTest.Second.Third.ThirdLevelNegative));
	assertEqual(false, TriggerEnabled(TracerTest.Second.Third.ThirdLevelNotDefined));
#endif
}

void TracerTest::TracerTestEnableAllBelowLowerBound()
{
#ifdef COAST_TRACE
	// enable all below lower bound switch
	assertEqual(true, TriggerEnabled(TracerTest.FirstLevelEnabled));
	assertEqual(false, TriggerEnabled(TracerTest.FirstLevelDisabled));
	assertEqual(false, TriggerEnabled(TracerTest.FirstLevelBelow));
	assertEqual(false, TriggerEnabled(TracerTest.FirstLevelAbove));
	assertEqual(false, TriggerEnabled(TracerTest.FirstLevelNegative));
	assertEqual(false, TriggerEnabled(TracerTest.FirstLevelNotDefined));

	assertEqual(true, TriggerEnabled(TracerTest.Second.SecondLevelEnabled));
	assertEqual(false, TriggerEnabled(TracerTest.Second.SecondLevelDisabled));
	assertEqual(false, TriggerEnabled(TracerTest.Second.SecondLevelBelow));
	assertEqual(false, TriggerEnabled(TracerTest.Second.SecondLevelAbove));
	assertEqual(false, TriggerEnabled(TracerTest.Second.SecondLevelNegative));
	assertEqual(false, TriggerEnabled(TracerTest.Second.SecondLevelNotDefined));

	assertEqual(true, TriggerEnabled(TracerTest.Second.Third.ThirdLevelEnabled));
	assertEqual(false, TriggerEnabled(TracerTest.Second.Third.ThirdLevelDisabled));
	assertEqual(false, TriggerEnabled(TracerTest.Second.Third.ThirdLevelBelow));
	assertEqual(false, TriggerEnabled(TracerTest.Second.Third.ThirdLevelAbove));
	assertEqual(false, TriggerEnabled(TracerTest.Second.Third.ThirdLevelNegative));
	assertEqual(false, TriggerEnabled(TracerTest.Second.Third.ThirdLevelNotDefined));

	assertEqual(false, TriggerEnabled(TracerTest.SlotNotDefined));
	assertEqual(false, TriggerEnabled(SectionNotDefined));
	assertEqual(false, TriggerEnabled(SectionNotDefined.SlotNotDefined));
#endif
}

void TracerTest::TracerTestEnableAllSecondAndBelowDisabled()
{
#ifdef COAST_TRACE
	// check EnableAll with exclusion and disable all below second level
	assertEqual(true, TriggerEnabled(TracerTest.FirstLevelEnabled));
	assertEqual(true, TriggerEnabled(TracerTest.FirstLevelDisabled));
	assertEqual(true, TriggerEnabled(TracerTest.FirstLevelBelow));
	assertEqual(true, TriggerEnabled(TracerTest.FirstLevelAbove));
	assertEqual(false, TriggerEnabled(TracerTest.FirstLevelNegative));
	assertEqual(true, TriggerEnabled(TracerTest.FirstLevelNotDefined));

	assertEqual(false, TriggerEnabled(TracerTest.Second.SecondLevelEnabled));
	assertEqual(false, TriggerEnabled(TracerTest.Second.SecondLevelDisabled));
	assertEqual(false, TriggerEnabled(TracerTest.Second.SecondLevelBelow));
	assertEqual(false, TriggerEnabled(TracerTest.Second.SecondLevelAbove));
	assertEqual(false, TriggerEnabled(TracerTest.Second.SecondLevelNegative));
	assertEqual(false, TriggerEnabled(TracerTest.Second.SecondLevelNotDefined));

	assertEqual(false, TriggerEnabled(TracerTest.Second.Third.ThirdLevelEnabled));
	assertEqual(false, TriggerEnabled(TracerTest.Second.Third.ThirdLevelDisabled));
	assertEqual(false, TriggerEnabled(TracerTest.Second.Third.ThirdLevelBelow));
	assertEqual(false, TriggerEnabled(TracerTest.Second.Third.ThirdLevelAbove));
	assertEqual(false, TriggerEnabled(TracerTest.Second.Third.ThirdLevelNegative));
	assertEqual(false, TriggerEnabled(TracerTest.Second.Third.ThirdLevelNotDefined));

	assertEqual(true, TriggerEnabled(TracerTest.SlotNotDefined));
	assertEqual(false, TriggerEnabled(SectionNotDefined));
	assertEqual(false, TriggerEnabled(SectionNotDefined.SlotNotDefined));
#endif
}

void TracerTest::TracerTestNotAllAboveLowerBound()
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

void TracerTest::TracerTestBug248()
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

void TracerTest::CheckMacrosCompile()
{
	StartTrace(TracerTest.CheckMacrosCompile);
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

Test *TracerTest::suite ()
{
	TestSuite *testSuite = new TestSuite;
	ADD_CASE(testSuite, TracerTest, CheckMacrosCompile);
	ADD_CASE(testSuite, TracerTest, TracerTestExplicitlyEnabled);
	ADD_CASE(testSuite, TracerTest, TracerTestLowerBoundZero);
	ADD_CASE(testSuite, TracerTest, TracerTestEnableAllFirstLevel);
	ADD_CASE(testSuite, TracerTest, TracerTestEnableAllSecondLevel);
	ADD_CASE(testSuite, TracerTest, TracerTestEnableAllThirdLevel);
	ADD_CASE(testSuite, TracerTest, TracerTestEnableAllAboveUpperBound);
	ADD_CASE(testSuite, TracerTest, TracerTestEnableAllBelowLowerBound);
	ADD_CASE(testSuite, TracerTest, TracerTestEnableAllSecondAndBelowDisabled);
	ADD_CASE(testSuite, TracerTest, TracerTestNotAllAboveLowerBound);
	ADD_CASE(testSuite, TracerTest, TracerTestBug248);
	return testSuite;
}
