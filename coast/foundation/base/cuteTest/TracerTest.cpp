/*
 * Copyright (c) 2006, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * Copyright (c) 2015, David Tran, Faculty of Computer Science,
 * University of Applied Sciences Rapperswil (HSR),
 * 8640 Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#include "TracerTest.h"
#include "Tracer.h"
#include "Anything.h"

TracerTest::TracerTest() {
#ifdef COAST_TRACE
	Tracer::ExchangeConfigFile(name());
#endif
}

TracerTest::~TracerTest() {
#ifdef COAST_TRACE
	// restore original Tracer.any File to not fail following tests... or wanted output
	Tracer::ExchangeConfigFile("Tracer");
#endif
}

void TracerTest::TracerTestExplicitlyEnabled()
{
#ifdef COAST_TRACE
	// test explicitly enabled switches between lower and upper bound
	ASSERT_EQUAL(true, TriggerEnabled(TracerTest.FirstLevelEnabled));
	ASSERT_EQUAL(false, TriggerEnabled(TracerTest.FirstLevelDisabled));
	ASSERT_EQUAL(false, TriggerEnabled(TracerTest.FirstLevelBelow));
	ASSERT_EQUAL(false, TriggerEnabled(TracerTest.FirstLevelAbove));
	ASSERT_EQUAL(false, TriggerEnabled(TracerTest.FirstLevelNegative));
	ASSERT_EQUAL(false, TriggerEnabled(TracerTest.FirstLevelNotDefined));

	ASSERT_EQUAL(true, TriggerEnabled(TracerTest.Second.SecondLevelEnabled));
	ASSERT_EQUAL(false, TriggerEnabled(TracerTest.Second.SecondLevelDisabled));
	ASSERT_EQUAL(false, TriggerEnabled(TracerTest.Second.SecondLevelBelow));
	ASSERT_EQUAL(false, TriggerEnabled(TracerTest.Second.SecondLevelAbove));
	ASSERT_EQUAL(false, TriggerEnabled(TracerTest.Second.SecondLevelNegative));
	ASSERT_EQUAL(false, TriggerEnabled(TracerTest.Second.SecondLevelNotDefined));

	ASSERT_EQUAL(true, TriggerEnabled(TracerTest.Second.Third.ThirdLevelEnabled));
	ASSERT_EQUAL(false, TriggerEnabled(TracerTest.Second.Third.ThirdLevelDisabled));
	ASSERT_EQUAL(false, TriggerEnabled(TracerTest.Second.Third.ThirdLevelBelow));
	ASSERT_EQUAL(false, TriggerEnabled(TracerTest.Second.Third.ThirdLevelAbove));
	ASSERT_EQUAL(false, TriggerEnabled(TracerTest.Second.Third.ThirdLevelNegative));
	ASSERT_EQUAL(false, TriggerEnabled(TracerTest.Second.Third.ThirdLevelNotDefined));

	ASSERT_EQUAL(false, TriggerEnabled(TracerTest.SlotNotDefined));
	ASSERT_EQUAL(false, TriggerEnabled(SectionNotDefined));
	ASSERT_EQUAL(false, TriggerEnabled(SectionNotDefined.SlotNotDefined));
#endif
}

void TracerTest::TracerTestLowerBoundZero()
{
#ifdef COAST_TRACE
	// test lower bound set to 0
	ASSERT_EQUAL(false, TriggerEnabled(TracerTest.FirstLevelEnabled));
	ASSERT_EQUAL(false, TriggerEnabled(TracerTest.FirstLevelDisabled));
	ASSERT_EQUAL(false, TriggerEnabled(TracerTest.FirstLevelBelow));
	ASSERT_EQUAL(false, TriggerEnabled(TracerTest.FirstLevelAbove));
	ASSERT_EQUAL(false, TriggerEnabled(TracerTest.FirstLevelNegative));
	ASSERT_EQUAL(false, TriggerEnabled(TracerTest.FirstLevelNotDefined));

	ASSERT_EQUAL(false, TriggerEnabled(TracerTest.Second.SecondLevelEnabled));
	ASSERT_EQUAL(false, TriggerEnabled(TracerTest.Second.SecondLevelDisabled));
	ASSERT_EQUAL(false, TriggerEnabled(TracerTest.Second.SecondLevelBelow));
	ASSERT_EQUAL(false, TriggerEnabled(TracerTest.Second.SecondLevelAbove));
	ASSERT_EQUAL(false, TriggerEnabled(TracerTest.Second.SecondLevelNegative));
	ASSERT_EQUAL(false, TriggerEnabled(TracerTest.Second.SecondLevelNotDefined));

	ASSERT_EQUAL(false, TriggerEnabled(TracerTest.Second.Third.ThirdLevelEnabled));
	ASSERT_EQUAL(false, TriggerEnabled(TracerTest.Second.Third.ThirdLevelDisabled));
	ASSERT_EQUAL(false, TriggerEnabled(TracerTest.Second.Third.ThirdLevelBelow));
	ASSERT_EQUAL(false, TriggerEnabled(TracerTest.Second.Third.ThirdLevelAbove));
	ASSERT_EQUAL(false, TriggerEnabled(TracerTest.Second.Third.ThirdLevelNegative));
	ASSERT_EQUAL(false, TriggerEnabled(TracerTest.Second.Third.ThirdLevelNotDefined));

	ASSERT_EQUAL(false, TriggerEnabled(TracerTest.SlotNotDefined));
	ASSERT_EQUAL(false, TriggerEnabled(SectionNotDefined));
	ASSERT_EQUAL(false, TriggerEnabled(SectionNotDefined.SlotNotDefined));
#endif
}

void TracerTest::TracerTestEnableAllFirstLevel()
{
#ifdef COAST_TRACE
	// enable all on the first level
	ASSERT_EQUAL(true, TriggerEnabled(TracerTest.FirstLevelEnabled));
	ASSERT_EQUAL(true, TriggerEnabled(TracerTest.FirstLevelDisabled));
	ASSERT_EQUAL(true, TriggerEnabled(TracerTest.FirstLevelBelow));
	ASSERT_EQUAL(true, TriggerEnabled(TracerTest.FirstLevelAbove));
	ASSERT_EQUAL(false, TriggerEnabled(TracerTest.FirstLevelNegative));
	ASSERT_EQUAL(true, TriggerEnabled(TracerTest.FirstLevelNotDefined));

	ASSERT_EQUAL(true, TriggerEnabled(TracerTest.Second.SecondLevelEnabled));
	ASSERT_EQUAL(true, TriggerEnabled(TracerTest.Second.SecondLevelDisabled));
	ASSERT_EQUAL(true, TriggerEnabled(TracerTest.Second.SecondLevelBelow));
	ASSERT_EQUAL(true, TriggerEnabled(TracerTest.Second.SecondLevelAbove));
	ASSERT_EQUAL(false, TriggerEnabled(TracerTest.Second.SecondLevelNegative));
	ASSERT_EQUAL(true, TriggerEnabled(TracerTest.Second.SecondLevelNotDefined));

	ASSERT_EQUAL(true, TriggerEnabled(TracerTest.Second.Third.ThirdLevelEnabled));
	ASSERT_EQUAL(true, TriggerEnabled(TracerTest.Second.Third.ThirdLevelDisabled));
	ASSERT_EQUAL(true, TriggerEnabled(TracerTest.Second.Third.ThirdLevelBelow));
	ASSERT_EQUAL(true, TriggerEnabled(TracerTest.Second.Third.ThirdLevelAbove));
	ASSERT_EQUAL(false, TriggerEnabled(TracerTest.Second.Third.ThirdLevelNegative));
	ASSERT_EQUAL(true, TriggerEnabled(TracerTest.Second.Third.ThirdLevelNotDefined));

	ASSERT_EQUAL(true, TriggerEnabled(TracerTest.SlotNotDefined));
	ASSERT_EQUAL(false, TriggerEnabled(SectionNotDefined));
	ASSERT_EQUAL(false, TriggerEnabled(SectionNotDefined.SlotNotDefined));
#endif
}

void TracerTest::TracerTestEnableAllSecondLevel()
{
#ifdef COAST_TRACE
	// enable all on the second level
	ASSERT_EQUAL(true, TriggerEnabled(TracerTest.FirstLevelEnabled));
	ASSERT_EQUAL(false, TriggerEnabled(TracerTest.FirstLevelDisabled));
	ASSERT_EQUAL(false, TriggerEnabled(TracerTest.FirstLevelBelow));
	ASSERT_EQUAL(false, TriggerEnabled(TracerTest.FirstLevelAbove));
	ASSERT_EQUAL(false, TriggerEnabled(TracerTest.FirstLevelNegative));
	ASSERT_EQUAL(false, TriggerEnabled(TracerTest.FirstLevelNotDefined));

	ASSERT_EQUAL(true, TriggerEnabled(TracerTest.Second.SecondLevelEnabled));
	ASSERT_EQUAL(true, TriggerEnabled(TracerTest.Second.SecondLevelDisabled));
	ASSERT_EQUAL(true, TriggerEnabled(TracerTest.Second.SecondLevelBelow));
	ASSERT_EQUAL(true, TriggerEnabled(TracerTest.Second.SecondLevelAbove));
	ASSERT_EQUAL(false, TriggerEnabled(TracerTest.Second.SecondLevelNegative));
	ASSERT_EQUAL(true, TriggerEnabled(TracerTest.Second.SecondLevelNotDefined));

	ASSERT_EQUAL(true, TriggerEnabled(TracerTest.Second.Third.ThirdLevelEnabled));
	ASSERT_EQUAL(true, TriggerEnabled(TracerTest.Second.Third.ThirdLevelDisabled));
	ASSERT_EQUAL(true, TriggerEnabled(TracerTest.Second.Third.ThirdLevelBelow));
	ASSERT_EQUAL(true, TriggerEnabled(TracerTest.Second.Third.ThirdLevelAbove));
	ASSERT_EQUAL(false, TriggerEnabled(TracerTest.Second.Third.ThirdLevelNegative));
	ASSERT_EQUAL(true, TriggerEnabled(TracerTest.Second.Third.ThirdLevelNotDefined));

	ASSERT_EQUAL(false, TriggerEnabled(TracerTest.SlotNotDefined));
	ASSERT_EQUAL(false, TriggerEnabled(SectionNotDefined));
	ASSERT_EQUAL(false, TriggerEnabled(SectionNotDefined.SlotNotDefined));
#endif
}

void TracerTest::TracerTestEnableAllThirdLevel()
{
#ifdef COAST_TRACE
	// enable all on the third level
	ASSERT_EQUAL(true, TriggerEnabled(TracerTest.FirstLevelEnabled));
	ASSERT_EQUAL(false, TriggerEnabled(TracerTest.FirstLevelDisabled));
	ASSERT_EQUAL(false, TriggerEnabled(TracerTest.FirstLevelBelow));
	ASSERT_EQUAL(false, TriggerEnabled(TracerTest.FirstLevelAbove));
	ASSERT_EQUAL(false, TriggerEnabled(TracerTest.FirstLevelNegative));
	ASSERT_EQUAL(false, TriggerEnabled(TracerTest.FirstLevelNotDefined));

	ASSERT_EQUAL(true, TriggerEnabled(TracerTest.Second.SecondLevelEnabled));
	ASSERT_EQUAL(false, TriggerEnabled(TracerTest.Second.SecondLevelDisabled));
	ASSERT_EQUAL(false, TriggerEnabled(TracerTest.Second.SecondLevelBelow));
	ASSERT_EQUAL(false, TriggerEnabled(TracerTest.Second.SecondLevelAbove));
	ASSERT_EQUAL(false, TriggerEnabled(TracerTest.Second.SecondLevelNegative));
	ASSERT_EQUAL(false, TriggerEnabled(TracerTest.Second.SecondLevelNotDefined));

	ASSERT_EQUAL(true, TriggerEnabled(TracerTest.Second.Third.ThirdLevelEnabled));
	ASSERT_EQUAL(true, TriggerEnabled(TracerTest.Second.Third.ThirdLevelDisabled));
	ASSERT_EQUAL(true, TriggerEnabled(TracerTest.Second.Third.ThirdLevelBelow));
	ASSERT_EQUAL(true, TriggerEnabled(TracerTest.Second.Third.ThirdLevelAbove));
	ASSERT_EQUAL(false, TriggerEnabled(TracerTest.Second.Third.ThirdLevelNegative));
	ASSERT_EQUAL(true, TriggerEnabled(TracerTest.Second.Third.ThirdLevelNotDefined));

	ASSERT_EQUAL(false, TriggerEnabled(TracerTest.SlotNotDefined));
	ASSERT_EQUAL(false, TriggerEnabled(SectionNotDefined));
	ASSERT_EQUAL(false, TriggerEnabled(SectionNotDefined.SlotNotDefined));
#endif
}

void TracerTest::TracerTestEnableAllAboveUpperBound()
{
#ifdef COAST_TRACE
	// enable all above upper bound switch
	ASSERT_EQUAL(true, TriggerEnabled(TracerTest.FirstLevelEnabled));
	ASSERT_EQUAL(false, TriggerEnabled(TracerTest.FirstLevelDisabled));
	ASSERT_EQUAL(false, TriggerEnabled(TracerTest.FirstLevelBelow));
	ASSERT_EQUAL(false, TriggerEnabled(TracerTest.FirstLevelAbove));
	ASSERT_EQUAL(false, TriggerEnabled(TracerTest.FirstLevelNegative));
	ASSERT_EQUAL(false, TriggerEnabled(TracerTest.FirstLevelNotDefined));

	ASSERT_EQUAL(true, TriggerEnabled(TracerTest.Second.SecondLevelEnabled));
	ASSERT_EQUAL(false, TriggerEnabled(TracerTest.Second.SecondLevelDisabled));
	ASSERT_EQUAL(false, TriggerEnabled(TracerTest.Second.SecondLevelBelow));
	ASSERT_EQUAL(false, TriggerEnabled(TracerTest.Second.SecondLevelAbove));
	ASSERT_EQUAL(false, TriggerEnabled(TracerTest.Second.SecondLevelNegative));
	ASSERT_EQUAL(false, TriggerEnabled(TracerTest.Second.SecondLevelNotDefined));

	ASSERT_EQUAL(true, TriggerEnabled(TracerTest.Second.Third.ThirdLevelEnabled));
	ASSERT_EQUAL(false, TriggerEnabled(TracerTest.Second.Third.ThirdLevelDisabled));
	ASSERT_EQUAL(false, TriggerEnabled(TracerTest.Second.Third.ThirdLevelBelow));
	ASSERT_EQUAL(false, TriggerEnabled(TracerTest.Second.Third.ThirdLevelAbove));
	ASSERT_EQUAL(false, TriggerEnabled(TracerTest.Second.Third.ThirdLevelNegative));
	ASSERT_EQUAL(false, TriggerEnabled(TracerTest.Second.Third.ThirdLevelNotDefined));
#endif
}

void TracerTest::TracerTestEnableAllBelowLowerBound()
{
#ifdef COAST_TRACE
	// enable all below lower bound switch
	ASSERT_EQUAL(true, TriggerEnabled(TracerTest.FirstLevelEnabled));
	ASSERT_EQUAL(false, TriggerEnabled(TracerTest.FirstLevelDisabled));
	ASSERT_EQUAL(false, TriggerEnabled(TracerTest.FirstLevelBelow));
	ASSERT_EQUAL(false, TriggerEnabled(TracerTest.FirstLevelAbove));
	ASSERT_EQUAL(false, TriggerEnabled(TracerTest.FirstLevelNegative));
	ASSERT_EQUAL(false, TriggerEnabled(TracerTest.FirstLevelNotDefined));

	ASSERT_EQUAL(true, TriggerEnabled(TracerTest.Second.SecondLevelEnabled));
	ASSERT_EQUAL(false, TriggerEnabled(TracerTest.Second.SecondLevelDisabled));
	ASSERT_EQUAL(false, TriggerEnabled(TracerTest.Second.SecondLevelBelow));
	ASSERT_EQUAL(false, TriggerEnabled(TracerTest.Second.SecondLevelAbove));
	ASSERT_EQUAL(false, TriggerEnabled(TracerTest.Second.SecondLevelNegative));
	ASSERT_EQUAL(false, TriggerEnabled(TracerTest.Second.SecondLevelNotDefined));

	ASSERT_EQUAL(true, TriggerEnabled(TracerTest.Second.Third.ThirdLevelEnabled));
	ASSERT_EQUAL(false, TriggerEnabled(TracerTest.Second.Third.ThirdLevelDisabled));
	ASSERT_EQUAL(false, TriggerEnabled(TracerTest.Second.Third.ThirdLevelBelow));
	ASSERT_EQUAL(false, TriggerEnabled(TracerTest.Second.Third.ThirdLevelAbove));
	ASSERT_EQUAL(false, TriggerEnabled(TracerTest.Second.Third.ThirdLevelNegative));
	ASSERT_EQUAL(false, TriggerEnabled(TracerTest.Second.Third.ThirdLevelNotDefined));

	ASSERT_EQUAL(false, TriggerEnabled(TracerTest.SlotNotDefined));
	ASSERT_EQUAL(false, TriggerEnabled(SectionNotDefined));
	ASSERT_EQUAL(false, TriggerEnabled(SectionNotDefined.SlotNotDefined));
#endif
}

void TracerTest::TracerTestEnableAllSecondAndBelowDisabled()
{
#ifdef COAST_TRACE
	// check EnableAll with exclusion and disable all below second level
	ASSERT_EQUAL(true, TriggerEnabled(TracerTest.FirstLevelEnabled));
	ASSERT_EQUAL(true, TriggerEnabled(TracerTest.FirstLevelDisabled));
	ASSERT_EQUAL(true, TriggerEnabled(TracerTest.FirstLevelBelow));
	ASSERT_EQUAL(true, TriggerEnabled(TracerTest.FirstLevelAbove));
	ASSERT_EQUAL(false, TriggerEnabled(TracerTest.FirstLevelNegative));
	ASSERT_EQUAL(true, TriggerEnabled(TracerTest.FirstLevelNotDefined));

	ASSERT_EQUAL(false, TriggerEnabled(TracerTest.Second.SecondLevelEnabled));
	ASSERT_EQUAL(false, TriggerEnabled(TracerTest.Second.SecondLevelDisabled));
	ASSERT_EQUAL(false, TriggerEnabled(TracerTest.Second.SecondLevelBelow));
	ASSERT_EQUAL(false, TriggerEnabled(TracerTest.Second.SecondLevelAbove));
	ASSERT_EQUAL(false, TriggerEnabled(TracerTest.Second.SecondLevelNegative));
	ASSERT_EQUAL(false, TriggerEnabled(TracerTest.Second.SecondLevelNotDefined));

	ASSERT_EQUAL(false, TriggerEnabled(TracerTest.Second.Third.ThirdLevelEnabled));
	ASSERT_EQUAL(false, TriggerEnabled(TracerTest.Second.Third.ThirdLevelDisabled));
	ASSERT_EQUAL(false, TriggerEnabled(TracerTest.Second.Third.ThirdLevelBelow));
	ASSERT_EQUAL(false, TriggerEnabled(TracerTest.Second.Third.ThirdLevelAbove));
	ASSERT_EQUAL(false, TriggerEnabled(TracerTest.Second.Third.ThirdLevelNegative));
	ASSERT_EQUAL(false, TriggerEnabled(TracerTest.Second.Third.ThirdLevelNotDefined));

	ASSERT_EQUAL(true, TriggerEnabled(TracerTest.SlotNotDefined));
	ASSERT_EQUAL(false, TriggerEnabled(SectionNotDefined));
	ASSERT_EQUAL(false, TriggerEnabled(SectionNotDefined.SlotNotDefined));
#endif
}

void TracerTest::TracerTestNotAllAboveLowerBound()
{
#ifdef COAST_TRACE
	// check EnableAll with exclusion and disable all below second level
	ASSERT_EQUAL(false, TriggerEnabled(BelowLowerBound.SlotNotDefined));
	ASSERT_EQUAL(true, TriggerEnabled(EnabledInRange.SlotNotDefined));
	ASSERT_EQUAL(true, TriggerEnabled(AtUpperBound.SlotNotDefined));
	ASSERT_EQUAL(false, TriggerEnabled(AboveUpperBound.SlotNotDefined));
	ASSERT_EQUAL(true, TriggerEnabled(NotAllEnabled.IamActive));
	ASSERT_EQUAL(false, TriggerEnabled(NotAllEnabled.SlotNotDefined));
	ASSERT_EQUAL(false, TriggerEnabled(Context.SlotNotDefined));
	ASSERT_EQUAL(true, TriggerEnabled(Context.HTMLWDDebug));
	ASSERT_EQUAL(true, TriggerEnabled(Context.HTMLWDDebug.TmpStore));
	ASSERT_EQUAL(true, TriggerEnabled(Context.HTMLWDDebug.EnvStore));
#endif
}

void TracerTest::TracerTestBug248()
{
#ifdef COAST_TRACE
	// check EnableAll with exclusion and disable all below second level
	ASSERT_EQUAL(false, TriggerEnabled(MyHTMLWriter.SlotNotDefined));
	ASSERT_EQUAL(false, TriggerEnabled(MyHTMLWriter.Tag));
	ASSERT_EQUAL(false, TriggerEnabled(MyHTMLWriter.Put));
	ASSERT_EQUAL(false, TriggerEnabled(MyHTMLWriter.Tag.NodeStack));
	ASSERT_EQUAL(false, TriggerEnabled(MyHTMLWriter.Tag.SlotNotDefined));
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
	StatTrace("test", "a stat trace", coast::storage::Current());
	StatTraceBuf("bli.bla", "0123456789012345", 10, coast::storage::Current());
	StatTraceAny("test.x", a, "an any", coast::storage::Current());
	TriggerEnabled(SectionNotDefined.SlotNotDefined);//lint !e522
}

void TracerTest::runAllTests(cute::suite &s) {
	s.push_back(CUTE_SMEMFUN(TracerTest, CheckMacrosCompile));
	s.push_back(CUTE_SMEMFUN(TracerTest, TracerTestExplicitlyEnabled));
	s.push_back(CUTE_SMEMFUN(TracerTest, TracerTestLowerBoundZero));
	s.push_back(CUTE_SMEMFUN(TracerTest, TracerTestEnableAllFirstLevel));
	s.push_back(CUTE_SMEMFUN(TracerTest, TracerTestEnableAllSecondLevel));
	s.push_back(CUTE_SMEMFUN(TracerTest, TracerTestEnableAllThirdLevel));
	s.push_back(CUTE_SMEMFUN(TracerTest, TracerTestEnableAllAboveUpperBound));
	s.push_back(CUTE_SMEMFUN(TracerTest, TracerTestEnableAllBelowLowerBound));
	s.push_back(CUTE_SMEMFUN(TracerTest, TracerTestEnableAllSecondAndBelowDisabled));
	s.push_back(CUTE_SMEMFUN(TracerTest, TracerTestNotAllAboveLowerBound));
	s.push_back(CUTE_SMEMFUN(TracerTest, TracerTestBug248));
}
