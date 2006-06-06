/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

//--- interface include -----------------------------------------------------
#include "AnyUtilsTest.h"

//--- test modules used --------------------------------------------------------
#include "TestSuite.h"

//--- module under test --------------------------------------------------------
#include "AnyUtils.h"

//--- standard modules used ----------------------------------------------------
#include "System.h"
#include "SysLog.h"
#include "StringStream.h"
#include "Dbg.h"

//--- c-library modules used ---------------------------------------------------

AnyUtilsTest::AnyUtilsTest (TString tname) : TestCaseType(tname) {};
AnyUtilsTest::~AnyUtilsTest() {};

void AnyUtilsTest::CompareTest()
{
	Anything testConfig2;
	const char *myFilename = "config/anyutilstest1";
	iostream *ifp = System::OpenStream( myFilename, "any");

	if (ifp == 0) {
		String eMsg = "Can't open config file ";
		eMsg << myFilename << ".any";
		SYSERROR(eMsg);
		return;
	} else {
		testConfig2.Import(*ifp);
	}
	// close file
	delete ifp;

	DoCheck(testConfig2["OkTests"], true, "OKTests");
	DoCheck(testConfig2["NokTests"], false, "NotOkTests");
}
bool AnyUtilsTest::DoXMLTest(const char *expect, ROAnything foroutput)
{
	StringStream ss;
	AnyUtils::PrintSimpleXML(ss, foroutput);
	return assertEqual(expect, ss.str());
}

void AnyUtilsTest::printEmptyXmlTest()
{
	Anything empty;
	t_assert(DoXMLTest("", empty));
}
void AnyUtilsTest::printSimpleXmlTest()
{
	Anything simple("foo");
	t_assert(DoXMLTest("<any:elt>foo</any:elt>", simple));
}
void AnyUtilsTest::printSequenceXmlTest()
{
	Anything sequence;
	sequence.Append("foo");
	sequence.Append("bar");
	sequence.Append("end");
	t_assert(DoXMLTest("<any:seq><any:elt>foo</any:elt><any:elt>bar</any:elt><any:elt>end</any:elt></any:seq>", sequence));
}

void AnyUtilsTest::printHashXmlTest()
{
	Anything hash;
	hash["foo"] = "bar";
	hash["end"] = "finish";
	t_assert(DoXMLTest("<any:seq><foo>bar</foo><end>finish</end></any:seq>", hash));
}
void AnyUtilsTest::printMixedXmlTest()
{
	Anything mixed;
	mixed["foo"] = "bar";
	mixed.Append("anonymous");
	mixed["number"] = 5L;
	mixed["end"] = "finish";
	t_assert(DoXMLTest("<any:seq><foo>bar</foo><any:elt>anonymous</any:elt><number>5</number><end>finish</end></any:seq>", mixed));
}

void AnyUtilsTest::DoCheck(Anything testCases, bool expectedResult, String description)
{
	StartTrace(AnyUtilsTest.DoCheck);

	long sz = testCases.GetSize();

	for (long i = 0; i < sz; i++) {
		Trace("Test " << testCases.SlotName(i));
		ROAnything testee = testCases[i];
		String msg = description;
		msg << "." << testCases.SlotName(i) << "\n";
		OStringStream resultStream;
		bool res = AnyUtils::AnyCompareEqual(testee["In"], testee["Master"], testCases.SlotName(i), &resultStream, testee["Delim"].AsCharPtr(".")[0L], testee["IndexDelim"].AsCharPtr(":")[0L] );
		assertEqualm( expectedResult, res, (const char *)(msg << resultStream.str()) );
	}
}

void AnyUtilsTest::MergeTest()
{
	StartTrace(AnyUtilsTest.MergeTest);
	Anything testConfig;
	const char *myFilename = "AnyMergeTest";
	iostream *ifp = System::OpenStream( myFilename, "any");
	if (ifp == 0) {
		String eMsg = "Can't open config file ";
		eMsg << myFilename << ".any";
		SYSERROR(eMsg);
		return;
	} else {
		testConfig.Import(*ifp);
	}
	// close file
	delete ifp;

	for (long i = 0; i < testConfig.GetSize(); i++) {
		Trace("Executing " << testConfig.SlotName(i));
		ROAnything testee = testConfig[i];
		Anything anyMaster = testee["Master"].DeepClone();
		AnyUtils::AnyMerge(anyMaster, testee["ToMerge"], testee["OverwriteSlots"].AsBool(0L), testee["Delim"].AsCharPtr(".")[0L], testee["IndexDelim"].AsCharPtr(":")[0L]);
		assertAnyCompareEqual(testee["Expected"], anyMaster, testConfig.SlotName(i), testee["Delim"].AsCharPtr(".")[0L], testee["IndexDelim"].AsCharPtr(":")[0L]);
	}
}

Test *AnyUtilsTest::suite ()
{
	TestSuite *testSuite = new TestSuite;
	ADD_CASE(testSuite, AnyUtilsTest, CompareTest);
	ADD_CASE(testSuite, AnyUtilsTest, printEmptyXmlTest);
	ADD_CASE(testSuite, AnyUtilsTest, printSimpleXmlTest);
	ADD_CASE(testSuite, AnyUtilsTest, printSequenceXmlTest);
	ADD_CASE(testSuite, AnyUtilsTest, printHashXmlTest);
	ADD_CASE(testSuite, AnyUtilsTest, printMixedXmlTest);
	ADD_CASE(testSuite, AnyUtilsTest, CompareTest);
	ADD_CASE(testSuite, AnyUtilsTest, MergeTest);

	return testSuite;
}
