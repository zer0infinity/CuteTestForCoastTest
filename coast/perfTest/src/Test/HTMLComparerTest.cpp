/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#include "HTMLComparerTest.h"
#include "TestSuite.h"
#include "HTMLComparer.h"

TString HTMLComparerTest::getConfigFileName() {
	return "HTMLComparerTestConfig";
}

void HTMLComparerTest::NoDifferenceComparison() {
	StartTrace(HTMLComparerTest.NoDifferenceComparison);

	long sz = GetTestCaseConfig().GetSize();
	for (long i = 0; i < sz; i++) {
		Anything thisCase = GetTestCaseConfig()[i].DeepClone();
		String thisCaseName = GetTestCaseConfig().SlotName(i);
		Anything t1 = thisCase["Master"];
		Anything t2 = thisCase["Slave"];

		HTMLComparer comparer(t1, t2);
		String report;
		if (!t_assertm(comparer.Compare(report), (const char *)thisCaseName)) {
			assertEqualm("", report, (const char *)thisCaseName);
		}
	}

}

void HTMLComparerTest::DifferenceComparison() {
	StartTrace(HTMLComparerTest.DifferenceComparison);

	long sz = GetTestCaseConfig().GetSize();
	for (long i = 0; i < sz; i++) {
		Anything thisCase = GetTestCaseConfig()[i].DeepClone();
		String thisCaseName = GetTestCaseConfig().SlotName(i);

		Anything t1 = thisCase["Master"];
		Anything t2 = thisCase["Slave"];

		HTMLComparer comparer(t1, t2);
		String report;
		t_assertm(!comparer.Compare(report), (const char *)thisCaseName);
		assertEqualm(thisCase["ExpectedReport"].AsCharPtr("XXX"), report, (const char *)thisCaseName);
	}
}

// builds up a suite of testcases, add a line for each testmethod
Test *HTMLComparerTest::suite() {
	StartTrace(HTMLComparerTest.suite);
	TestSuite *testSuite = new TestSuite;
	ADD_CASE(testSuite, HTMLComparerTest, NoDifferenceComparison);
	ADD_CASE(testSuite, HTMLComparerTest, DifferenceComparison);
	return testSuite;
}
