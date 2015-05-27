/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#include "AnyUtilsTest.h"
#include "AnyUtils.h"
#include "SystemLog.h"

using namespace coast;

void AnyUtilsTest::CompareTest() {
	Anything testConfig2;
	const char *myFilename = "config/anyutilstest1";
	std::iostream *ifp = system::OpenStream(myFilename, "any");

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

bool AnyUtilsTest::DoXMLTest(const char *expect, ROAnything foroutput) {
	std::stringstream ss;
	AnyUtils::PrintSimpleXML(ss, foroutput);
	ASSERT_EQUAL(expect, ss.str());
	return expect == ss;
}

void AnyUtilsTest::printEmptyXmlTest() {
	Anything empty;
	DoXMLTest("", empty);
}
void AnyUtilsTest::printSimpleXmlTest() {
	Anything simple("foo");
	DoXMLTest("<any:elt>foo</any:elt>", simple);
}
void AnyUtilsTest::printSequenceXmlTest() {
	Anything sequence;
	sequence.Append("foo");
	sequence.Append("bar");
	sequence.Append("end");
	DoXMLTest("<any:seq><any:elt>foo</any:elt><any:elt>bar</any:elt><any:elt>end</any:elt></any:seq>", sequence);
}

void AnyUtilsTest::printHashXmlTest() {
	Anything hash;
	hash["foo"] = "bar";
	hash["end"] = "finish";
	DoXMLTest("<any:seq><foo>bar</foo><end>finish</end></any:seq>", hash);
}
void AnyUtilsTest::printMixedXmlTest() {
	Anything mixed;
	mixed["foo"] = "bar";
	mixed.Append("anonymous");
	mixed["number"] = 5L;
	mixed["end"] = "finish";
	DoXMLTest("<any:seq><foo>bar</foo><any:elt>anonymous</any:elt><number>5</number><end>finish</end></any:seq>", mixed);
}

void AnyUtilsTest::DoCheck(Anything testCases, bool expectedResult, String description) {
	StartTrace(AnyUtilsTest.DoCheck);

	long sz = testCases.GetSize();

	for (long i = 0; i < sz; i++) {
		Trace("Test " << testCases.SlotName(i));
		ROAnything testee = testCases[i];
		String msg = description;
		msg << "." << testCases.SlotName(i) << "\n";
		OStringStream resultStream;
		bool res = AnyUtils::AnyCompareEqual(testee["In"], testee["Master"], testCases.SlotName(i), &resultStream,
				testee["Delim"].AsCharPtr(".")[0L], testee["IndexDelim"].AsCharPtr(":")[0L]);
		ASSERT_EQUALM((const char *)(msg << resultStream.str()) , expectedResult, res);
	}
}

void AnyUtilsTest::MergeTest() {
	StartTrace(AnyUtilsTest.MergeTest);
	Anything testConfig;
	const char *myFilename = "AnyMergeTest";
	std::iostream *ifp = system::OpenStream(myFilename, "any");
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
		AnyUtils::AnyMerge(anyMaster, testee["ToMerge"], testee["OverwriteSlots"].AsBool(0L), testee["Delim"].AsCharPtr(".")[0L],
				testee["IndexDelim"].AsCharPtr(":")[0L]);
		ASSERT_ANY_COMPARE(testee["Expected"], anyMaster, testConfig.SlotName(i), testee["Delim"].AsCharPtr(".")[0L], testee["IndexDelim"].AsCharPtr(":")[0L]);
	}
}

void AnyUtilsTest::runAllTests(cute::suite &s) {
	s.push_back(CUTE_SMEMFUN(AnyUtilsTest, CompareTest));
	s.push_back(CUTE_SMEMFUN(AnyUtilsTest, printEmptyXmlTest));
	s.push_back(CUTE_SMEMFUN(AnyUtilsTest, printSimpleXmlTest));
	s.push_back(CUTE_SMEMFUN(AnyUtilsTest, printSequenceXmlTest));
	s.push_back(CUTE_SMEMFUN(AnyUtilsTest, printHashXmlTest));
	s.push_back(CUTE_SMEMFUN(AnyUtilsTest, printMixedXmlTest));
	s.push_back(CUTE_SMEMFUN(AnyUtilsTest, MergeTest));


}
