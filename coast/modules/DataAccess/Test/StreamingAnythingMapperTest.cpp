/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

//--- interface include --------------------------------------------------------
#include "StreamingAnythingMapperTest.h"

//--- module under test --------------------------------------------------------
#include "StreamingAnythingMapper.h"

//--- test modules used --------------------------------------------------------
#include "TestSuite.h"

//--- standard modules used ----------------------------------------------------
#include "StringStream.h"
#include "Dbg.h"

//---- StreamingAnythingMapperTest ----------------------------------------------------------------
StreamingAnythingMapperTest::StreamingAnythingMapperTest(TString tname) : TestCase(tname)
{
}

StreamingAnythingMapperTest::~StreamingAnythingMapperTest()
{
}

void StreamingAnythingMapperTest::setUp ()
// setup connector for this TestCase
{

} // setUp

void StreamingAnythingMapperTest::tearDown ()
{

} // tearDown

void StreamingAnythingMapperTest::GetTest()
{
	StartTrace(StreamingAnythingMapperTest.GetTest);
	Anything clientData;
	clientData["Input"]["Slot1"] = "Something";
	clientData["Input"]["Slot2"] = 42;

	Context ctx(clientData, Anything(), 0, 0, 0, 0);
	AnythingToStreamMapper mapper("NoName");

	String streamedAny;
	OStringStream out(&streamedAny);
	mapper.Get("Input", out, ctx);
	assertEqual( _QUOTE_({/Slot1 "Something" /Slot2 42}) , streamedAny);

	String streamedAny2;
	OStringStream out2(&streamedAny2);
	mapper.Get("Input.Slot1", out2, ctx);
	assertEqual( _QUOTE_("Something") , streamedAny2);
}

void StreamingAnythingMapperTest::PutTest()
{
	StartTrace(StreamingAnythingMapperTest.PutTest);

	Anything dummy;
	Context ctx(dummy, Anything(), 0, 0, 0, 0);
	StreamToAnythingMapper sam("NoName");
	String streamedAny("{ /Slot1 Something/Slot2 42}");
	IStringStream in(&streamedAny);
	sam.Put("Output", in, ctx);

	Anything tmpStore = ctx.GetTmpStore();
	Anything result = tmpStore["Mapper"]["Output"];

	assertEqual("Something", result["Slot1"].AsCharPtr("x"));
	assertEqual(42, result["Slot2"].AsLong(0));
}

Test *StreamingAnythingMapperTest::suite ()
// collect all test cases for the SocketStream
{
	TestSuite *testSuite = new TestSuite;

	testSuite->addTest (NEW_CASE(StreamingAnythingMapperTest, PutTest));
	testSuite->addTest (NEW_CASE(StreamingAnythingMapperTest, GetTest));

	return testSuite;

} // suite
