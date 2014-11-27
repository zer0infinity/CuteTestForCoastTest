/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#include "MIMEHeaderTest.h"
#include "MIMEHeader.h"
#include "TestSuite.h"
#include "AnyUtils.h"

void MIMEHeaderTest::SimpleHeaderTest() {
	StartTrace(MIMEHeaderTest.SimpleHeaderTest);

	Anything result, result1;

	// some bad data tests
	String testinput = "nonsense";
	{
		MIMEHeader mh;
		StringStream is(testinput);

		// basic checks of success
		try {
			t_assertm(!mh.ParseHeaders(is), "expected header parsing to fail");
		} catch (MIMEHeader::InvalidLineException &e) {
			t_assertm(true, "expected invalid line exception to happen");
		} catch (...) {
			t_assertm(false,"did not expect other exceptions to be thrown");
		}

		// sanity checks
		t_assertm(mh.IsMultiPart() == false, "expected no multipart");
		t_assertm(mh.GetBoundary().Length() == 0, "expected no multipart seperator");
		t_assertm(mh.GetContentLength() == -1, "expected -1, since field is not set");
		assertEqualm("", mh.Lookup("NotThere", ""), "expected 'NotThere' to be emtpy");
		assertAnyEqual(result, mh.GetInfo());
		TraceAny(mh.GetInfo(), "Header: ");
	}
}

void MIMEHeaderTest::ConfiguredTests() {
	StartTrace(MIMEHeaderTest.ConfiguredTests);
	ROAnything caseConfig;
	AnyExtensions::Iterator<ROAnything, ROAnything, TString> aEntryIterator(GetTestCaseConfig());
	while (aEntryIterator.Next(caseConfig)) {
		TString caseName;
		if (!aEntryIterator.SlotName(caseName)) {
			caseName << "At index: " << aEntryIterator.Index();
		}
		coast::urlutils::NormalizeTag normalizeKey = (coast::urlutils::NormalizeTag) (caseConfig["NormalizeTag"].AsLong(
				(long) coast::urlutils::eUpshift));
		MIMEHeader mh(normalizeKey);

		String testinput = caseConfig["Input"].AsString();
		Trace("TestInput: <" << testinput << ">");
		StringStream is(testinput);
		// basic checks of success
		t_assertm(mh.ParseHeaders(is), TString("expected header parsing to succeed at ") << caseName);
		assertEqualm(caseConfig["HeaderLength"].AsLong(777L), mh.GetParsedHeaderLength(), caseName);

		// sanity checks
		assertEqualm(caseConfig["IsMultiPart"].AsBool(false), mh.IsMultiPart(), caseName);
		assertComparem(caseConfig["BoundaryLength"].AsLong(0), equal_to, mh.GetBoundary().Length(), caseName);
		assertCharPtrEqualm(caseConfig["Boundary"].AsString(), mh.GetBoundary(), caseName);
		assertComparem(caseConfig["ContentLength"].AsLong(-1), equal_to, mh.GetContentLength(), caseName);
		assertEqualm("", mh.Lookup("NotThere", ""), caseName);
		assertAnyCompareEqual(caseConfig["Expected"], mh.GetInfo(), caseName, '.', ':');
	}
}

// builds up a suite of testcases, add a line for each testmethod
Test *MIMEHeaderTest::suite() {
	StartTrace(MIMEHeaderTest.suite);
	TestSuite *testSuite = new TestSuite;
	ADD_CASE(testSuite, MIMEHeaderTest, SimpleHeaderTest);
	ADD_CASE(testSuite, MIMEHeaderTest, ConfiguredTests);
	return testSuite;
}

