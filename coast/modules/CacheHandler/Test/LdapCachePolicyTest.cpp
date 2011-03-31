/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#include "LdapCachePolicyTest.h"
#include "LDAPCachePolicyModule.h"
#include "TestSuite.h"

//---- LdapCachePolicyTest ----------------------------------------------------------------
LdapCachePolicyTest::LdapCachePolicyTest(TString tstrName)
	: TestCaseType(tstrName)
{
	StartTrace(LdapCachePolicyTest.Ctor);
}

LdapCachePolicyTest::~LdapCachePolicyTest()
{
	StartTrace(LdapCachePolicyTest.Dtor);
}

void LdapCachePolicyTest::NoDataReadTest()
{
	StartTrace(LdapCachePolicyTest.NoDataReadTest);
	WDModule *pModule = WDModule::FindWDModule("LdapCachePolicyModule");
	if ( t_assertm(pModule != NULL, "expected LdapCachePolicyModule to be registered") ) {
		t_assertm( !pModule->Init(GetTestCaseConfig()), "LdapCachePolicyModule init should have failed because a relevant dataaccess failed");
	}
}

void LdapCachePolicyTest::ReInitTest()
{
	StartTrace(LdapCachePolicyTest.ReInitTest);
	WDModule *pModule = WDModule::FindWDModule("LdapCachePolicyModule");
	if ( t_assertm(pModule != NULL, "expected LdapCachePolicyModule to be registered") ) {
		ROAnything result;
		t_assert(LdapCacheGetter::Get(result, "TestDA1", ":0.dn"));
		assertEqualm("ou=tkfu,o=10601,c=CH,dc=tkfcd.hsr.ch", result.AsString(), "Reset test failed.");
		t_assert(LdapCacheGetter::Get(result, "TestDA2", ":0.dn"));
		assertEqualm("uid=tkgae,ou=tkfu,o=10601,c=CH,dc=tkfcd.hsr.ch", result.AsString(), "Reset test failed.");

		// simulate a WDModule::Reset()
		if ( t_assertm( pModule->ResetFinis(GetTestCaseConfig()), "ResetFinis should have worked") ) {
			t_assertm( pModule->ResetInit(GetTestCaseConfig()), "ResetFinis should have worked");
		}

		t_assert(LdapCacheGetter::Get(result, "TestDA1Action", ":0.dn"));
		assertEqualm("ou=tkfu,o=10601,c=CH,dc=tkfcd.hsr.ch", result.AsString(), "Reset test failed.");
		t_assert(LdapCacheGetter::Get(result, "TestDA2Action", ":0.dn"));
		assertEqualm("uid=tkgae,ou=tkfu,o=10601,c=CH,dc=tkfcd.hsr.ch", result.AsString(), "Reset test failed.");
	}
}

void LdapCachePolicyTest::CallsInARow()
{
	StartTrace(LdapCachePolicyTest.CallsInARow);
	ROAnything result;
	t_assert(LdapCacheGetter::Get(result, "TestDA1", ":0.dn"));
	assertEqualm("ou=tkfu,o=10601,c=CH,dc=tkfcd.hsr.ch", result.AsString(), "Reset test failed.");
	t_assert(LdapCacheGetter::Get(result, "TestDA2", ":0.dn"));
	assertEqualm("uid=tkgae,ou=tkfu,o=10601,c=CH,dc=tkfcd.hsr.ch", result.AsString(), "Reset test failed.");

	t_assert(LdapCacheGetter::Get(result, "TestDA1Action", ":0.dn"));
	assertEqualm("ou=tkfu,o=10601,c=CH,dc=tkfcd.hsr.ch", result.AsString(), "Reset test failed.");
	t_assert(LdapCacheGetter::Get(result, "TestDA2Action", ":0.dn"));
	assertEqualm("uid=tkgae,ou=tkfu,o=10601,c=CH,dc=tkfcd.hsr.ch", result.AsString(), "Reset test failed.");
}

// builds up a suite of testcases, add a line for each testmethod
Test *LdapCachePolicyTest::suite ()
{
	StartTrace(LdapCachePolicyTest.suite);
	TestSuite *testSuite = new TestSuite;
	ADD_CASE(testSuite, LdapCachePolicyTest, NoDataReadTest);
	ADD_CASE(testSuite, LdapCachePolicyTest, ReInitTest);
	ADD_CASE(testSuite, LdapCachePolicyTest, CallsInARow);
	return testSuite;
}
