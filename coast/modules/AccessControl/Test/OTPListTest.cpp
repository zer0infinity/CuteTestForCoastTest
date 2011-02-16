/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

//--- interface include --------------------------------------------------------
#include "OTPListTest.h"

//--- module under test --------------------------------------------------------
#include "MockOTPList.h"

//--- test modules used --------------------------------------------------------
#include "TestSuite.h"

//--- standard modules used ----------------------------------------------------
#include "Dbg.h"
#include "WDModule.h"
#include "AccessController.h"

//--- c-modules used -----------------------------------------------------------

//---- TDAC_Adapter ---------------------------------------------------------------

class TDAC_Adapter : public TokenDataAccessController
{
public:
	// constructor
	TDAC_Adapter(MockOTPList *list) : TokenDataAccessController("TDAC_Adapter") {
		fList = list;
	}

	// adapter methods
	unsigned long GetCount(String tokenid) {
		return fList->GetCount(tokenid);
	}
	bool IncCount(String tokenid, long by) {
		unsigned long old = GetCount(tokenid);
		fList->SetCount(tokenid, old + by);
		return (GetCount(tokenid) == old + by);
	}

	// dummy implementation, not used
	String GetSeed(String tokenid) {
		return "";
	} ;

private:
	MockOTPList *fList;
};

//---- OTPListTest ----------------------------------------------------------------
OTPListTest::OTPListTest(TString tstrName)
	: TestCaseType(tstrName)
{
	StartTrace(OTPListTest.OTPListTest);
}

TString OTPListTest::getConfigFileName()
{
	return "OTPListTestConfig";
}

OTPListTest::~OTPListTest()
{
	StartTrace(OTPListTest.Dtor);
}

void OTPListTest::RunOtpTests(String implName, OTPList *impl, TokenDataAccessController *tdac)
{
	StartTrace(OTPListTest.RunOtpTests);

	String tid, code;
	long win, dist;

	Trace("Running tests for " << implName << "...");
	ROAnything tests = GetConfig()["Tests"][implName];
	for (long i = 0; i < tests.GetSize(); i++) {
		Trace("Executing test: " << tests.SlotName(i));
		tid = tests[i]["TokId"].AsString();
		code = tests[i]["Code"].AsString();
		win = tests[i]["Window"].AsLong();
		dist = impl->IsValid(tid, code, win, tdac);

		// validate code
		assertEqual(tests[i]["ValidResult"].AsLong(), dist);

		if (tdac) {
			if (dist >= 0) {
				// if code was valid, then do another check, this time updating the count
				impl->Verify(tid, code, win, tdac);
			}
			assertEqual(tests[i]["CountResult"].AsLong(), tdac->GetCount(tid));
		}
	}

}

void OTPListTest::AlwaysTrueMockOTPListTest()
{
	StartTrace(OTPListTest.AlwaysTrueMockOTPListTest);

	String implName = "AlwaysTrueOtp";
	OTPList *impl = OTPList::FindOTPList(implName);
	if (t_assertm(impl, "couldn't find MockOTPList-implementation 'AlwaysTrueOtp'.")) {
		// run tests with null-tdac
		RunOtpTests(implName, impl, 0);
	}
}

void OTPListTest::MockOTPListTest()
{
	StartTrace(OTPListTest.MockOTPListTest);

	String implName = "MockOtp";
	MockOTPList *impl = static_cast<MockOTPList *>(OTPList::FindOTPList(implName));
	if (t_assertm(impl, "couldn't find MockOTPList-implementation 'MockOtp'.")) {
		// set up a TDAC adapter
		TDAC_Adapter tdacAdapter(impl);
		// run tests
		RunOtpTests(implName, impl, &tdacAdapter);
	}
}

// builds up a suite of tests, add a line for each testmethod
Test *OTPListTest::suite ()
{
	StartTrace(OTPListTest.suite);
	TestSuite *testSuite = new TestSuite;
	ADD_CASE(testSuite, OTPListTest, AlwaysTrueMockOTPListTest);
	ADD_CASE(testSuite, OTPListTest, MockOTPListTest);
	return testSuite;
}
