/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

//--- interface include --------------------------------------------------------
#include "LDAPDAICachePolicyTest.h"
//--- module under test --------------------------------------------------------
#include "LDAPDAICachePolicyModule.h"

//--- test modules used --------------------------------------------------------
#include "TestSuite.h"

//--- standard modules used ----------------------------------------------------
//--- c-modules used -----------------------------------------------------------

//---- LDAPDAICachePolicyTest ----------------------------------------------------------------
LDAPDAICachePolicyTest::LDAPDAICachePolicyTest(TString tstrName)
	: TestCaseType(tstrName)
{
	StartTrace(LDAPDAICachePolicyTest.Ctor);
}

LDAPDAICachePolicyTest::~LDAPDAICachePolicyTest()
{
	StartTrace(LDAPDAICachePolicyTest.Dtor);
}

void LDAPDAICachePolicyTest::NoDataReadTest()
{
	StartTrace(LDAPDAICachePolicyTest.NoDataReadTest);
	WDModule *pModule = WDModule::FindWDModule("LDAPDAICachePolicyModule");
	if ( t_assertm(pModule != NULL, "expected LDAPDAICachePolicyModule to be registered") ) {
		t_assertm( !pModule->Init(GetTestCaseConfig()), "LDAPDAICachePolicyModule init should have failed because a relevant dataaccess failed");
	}
}

void LDAPDAICachePolicyTest::ReInitTest()
{
	StartTrace(LDAPDAICachePolicyTest.ReInitTest);
	WDModule *pModule = WDModule::FindWDModule("LDAPDAICachePolicyModule");
	if ( t_assertm(pModule != NULL, "expected LDAPDAICachePolicyModule to be registered") ) {
		ROAnything result;
		t_assert(LDAPDAICacheGetter::Get(result, "TestDA1", ":0.name"));
		assertEqualm("ifs APPL. User Directory", result.AsString(), "Reset test failed.");
		t_assert(LDAPDAICacheGetter::Get(result, "TestDA2", ":0.pd-dn"));
		assertEqualm("cn=CH10601-tkgae,dc=tkfpd.hsr.ch", result.AsString(), "Reset test failed.");

		// simulate a WDModule::Reset()
		if ( t_assertm( pModule->ResetFinis(GetTestCaseConfig()), "ResetFinis should have worked") ) {
			t_assertm( pModule->ResetInit(GetTestCaseConfig()), "ResetFinis should have worked");
		}

		t_assert(LDAPDAICacheGetter::Get(result, "TestDA1Action", ":0.name"));
		assertEqualm("ifs APPL. User Directory", result.AsString(), "Reset test failed.");
		t_assert(LDAPDAICacheGetter::Get(result, "TestDA2Action", ":0.pd-dn"));
		assertEqualm("cn=CH10601-tkgae,dc=tkfpd.hsr.ch", result.AsString(), "Reset test failed.");
	}
}

void LDAPDAICachePolicyTest::CallsInARow()
{
	StartTrace(LDAPDAICachePolicyTest.CallsInARow);
	ROAnything result;
	t_assert(LDAPDAICacheGetter::Get(result, "TestDA1", ":0.name"));
	assertEqualm("ifs APPL. User Directory", result.AsString(), "Reset test failed.");
	t_assert(LDAPDAICacheGetter::Get(result, "TestDA2", ":0.pd-dn"));
	assertEqualm("cn=CH10601-tkgae,dc=tkfpd.hsr.ch", result.AsString(), "Reset test failed.");

	t_assert(LDAPDAICacheGetter::Get(result, "TestDA1Action", ":0.name"));
	assertEqualm("ifs APPL. User Directory", result.AsString(), "Reset test failed.");
	t_assert(LDAPDAICacheGetter::Get(result, "TestDA2Action", ":0.pd-dn"));
	assertEqualm("cn=CH10601-tkgae,dc=tkfpd.hsr.ch", result.AsString(), "Reset test failed.");
}

// builds up a suite of testcases, add a line for each testmethod
Test *LDAPDAICachePolicyTest::suite ()
{
	StartTrace(LDAPDAICachePolicyTest.suite);
	TestSuite *testSuite = new TestSuite;
	ADD_CASE(testSuite, LDAPDAICachePolicyTest, NoDataReadTest);
	ADD_CASE(testSuite, LDAPDAICachePolicyTest, CallsInARow);
	ADD_CASE(testSuite, LDAPDAICachePolicyTest, ReInitTest);
	return testSuite;
}
