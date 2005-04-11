/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

//--- c-modules used -----------------------------------------------------------

//--- standard modules used ----------------------------------------------------
#include "Anything.h"
#include "Dbg.h"

//--- test modules used --------------------------------------------------------
#include "TestSuite.h"

//--- module under test --------------------------------------------------------

//--- interface include --------------------------------------------------------
#include "LdapCachePolicyTest.h"
#include "LDAPCachePolicyModule.h"
#include "System.h"
#include "StringStream.h"
#include "Application.h"
#include "Threads.h"

//---- LdapCachePolicyTest ----------------------------------------------------------------
LdapCachePolicyTest::LdapCachePolicyTest(TString name) : TestCase(name)
{
	StartTrace(LdapCachePolicyTest.Ctor);
}

LdapCachePolicyTest::~LdapCachePolicyTest()
{
	StartTrace(LdapCachePolicyTest.Dtor);
}

// setup for this TestCase
void LdapCachePolicyTest::setUp ()
{
	StartTrace(LdapCachePolicyTest.setUp);
	t_assert(System::LoadConfigFile(fGlobalConfig, "Config"));
	t_assert(fGlobalConfig.IsDefined("Modules"));
	Application::InitializeGlobalConfig(fGlobalConfig);
	assertEqualm(0, WDModule::Install(fGlobalConfig), "WDModule::Install should have worked.");
}

void LdapCachePolicyTest::setUp (const String &configName)
{
	StartTrace(LdapCachePolicyTest.setUp);
	t_assert(System::LoadConfigFile(fGlobalConfig, configName));
	TraceAny(fGlobalConfig, "Global Config");
	t_assert(fGlobalConfig.IsDefined("Modules"));
	Application::InitializeGlobalConfig(fGlobalConfig);
	// Will fail because of NoDataDA and LdapCachePolicyModule return
	// code check is mandatory
	assertEqualm(-1, WDModule::Install(fGlobalConfig), "WDModule::Install should have failed.");
}

void LdapCachePolicyTest::tearDown ()
{
	StartTrace(LdapCachePolicyTest.tearDown);
	t_assert(fGlobalConfig.IsDefined("Modules"));
	WDModule::Terminate(fGlobalConfig);
}

void LdapCachePolicyTest::NoDataReadTest()
{
	StartTrace(LdapCachePolicyTest.NoDataReadTest);
	setUp();
	tearDown();
	setUp(String("NoDataQueryConfig"));
	// re-install "good" config
	tearDown();
	setUp();
}

void LdapCachePolicyTest::ReInitTest()
{
	StartTrace(LdapCachePolicyTest.ReInitTest);
	int i;
	for (i = 0; i < 5; i++) {
		ROAnything result;
		t_assert(LdapCacheGetter::Get(result, "TestDA1", ":0.dn"));
		assertEqualm("ou=tkfu,o=10601,c=CH,dc=tkfcd.hsr.ch", result.AsString(), "Reset test failed.");
		t_assert(LdapCacheGetter::Get(result, "TestDA2", ":0.dn"));
		assertEqualm("uid=tkgae,ou=tkfu,o=10601,c=CH,dc=tkfcd.hsr.ch", result.AsString(), "Reset test failed.");
		assertEqualm(0, WDModule::Reset(fGlobalConfig, fGlobalConfig), "WDModule::Reset should have worked");
		break;
	}
	for (i = 0; i < 5; i++) {
		ROAnything result;
		t_assert(LdapCacheGetter::Get(result, "TestDA1Action", ":0.dn"));
		assertEqualm("ou=tkfu,o=10601,c=CH,dc=tkfcd.hsr.ch", result.AsString(), "Reset test failed.");
		t_assert(LdapCacheGetter::Get(result, "TestDA2Action", ":0.dn"));
		assertEqualm("uid=tkgae,ou=tkfu,o=10601,c=CH,dc=tkfcd.hsr.ch", result.AsString(), "Reset test failed.");
		assertEqualm(0, WDModule::Reset(fGlobalConfig, fGlobalConfig), "WDModule::Reset should have worked");
		break;
	}
}

void LdapCachePolicyTest::CallsInARow()
{
	StartTrace(LdapCachePolicyTest.ReInitTest);
	int i;
	for (i = 0; i < 5; i++) {
		ROAnything result;
		t_assert(LdapCacheGetter::Get(result, "TestDA1", ":0.dn"));
		assertEqualm("ou=tkfu,o=10601,c=CH,dc=tkfcd.hsr.ch", result.AsString(), "Reset test failed.");
		t_assert(LdapCacheGetter::Get(result, "TestDA2", ":0.dn"));
		assertEqualm("uid=tkgae,ou=tkfu,o=10601,c=CH,dc=tkfcd.hsr.ch", result.AsString(), "Reset test failed.");
	}
	for (i = 0; i < 5; i++) {
		ROAnything result;
		t_assert(LdapCacheGetter::Get(result, "TestDA1Action", ":0.dn"));
		assertEqualm("ou=tkfu,o=10601,c=CH,dc=tkfcd.hsr.ch", result.AsString(), "Reset test failed.");
		t_assert(LdapCacheGetter::Get(result, "TestDA2Action", ":0.dn"));
		assertEqualm("uid=tkgae,ou=tkfu,o=10601,c=CH,dc=tkfcd.hsr.ch", result.AsString(), "Reset test failed.");
	}
}

void LdapCachePolicyTest::testCase()
{
	StartTrace(LdapCachePolicyTest.testCase);
//	t_assert(false);
}

// builds up a suite of testcases, add a line for each testmethod
Test *LdapCachePolicyTest::suite ()
{
	StartTrace(LdapCachePolicyTest.suite);
	TestSuite *testSuite = new TestSuite;
	testSuite->addTest (NEW_CASE(LdapCachePolicyTest, NoDataReadTest));
	testSuite->addTest (NEW_CASE(LdapCachePolicyTest, CallsInARow));
	testSuite->addTest (NEW_CASE(LdapCachePolicyTest, ReInitTest));
	return testSuite;
}
