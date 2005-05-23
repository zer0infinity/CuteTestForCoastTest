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
#include "LDAPDAICachePolicyTest.h"
#include "LDAPDAICachePolicyModule.h"
#include "System.h"
#include "StringStream.h"
#include "Application.h"
#include "Threads.h"

//---- LDAPDAICachePolicyTest ----------------------------------------------------------------
LDAPDAICachePolicyTest::LDAPDAICachePolicyTest(TString tstrName) : TestCase(tstrName)
{
	StartTrace(LDAPDAICachePolicyTest.Ctor);
}

LDAPDAICachePolicyTest::~LDAPDAICachePolicyTest()
{
	StartTrace(LDAPDAICachePolicyTest.Dtor);
}

// setup for this TestCase
void LDAPDAICachePolicyTest::setUp ()
{
	StartTrace(LDAPDAICachePolicyTest.setUp);
	t_assert(System::LoadConfigFile(fGlobalConfig, "Config"));
	t_assert(fGlobalConfig.IsDefined("Modules"));
	Application::InitializeGlobalConfig(fGlobalConfig);
	assertEqualm(0, WDModule::Install(fGlobalConfig), "WDModule::Install should have worked.");
}

void LDAPDAICachePolicyTest::setUp (const String &configName)
{
	StartTrace(LDAPDAICachePolicyTest.setUp);
	t_assert(System::LoadConfigFile(fGlobalConfig, configName));
	TraceAny(fGlobalConfig, "Global Config");
	t_assert(fGlobalConfig.IsDefined("Modules"));
	Application::InitializeGlobalConfig(fGlobalConfig);
	// Will fail because of NoDataDA and LdapCachePolicyModule return
	// code check is mandatory
	assertEqualm(-1, WDModule::Install(fGlobalConfig), "WDModule::Install should have failed.");
}

void LDAPDAICachePolicyTest::tearDown ()
{
	StartTrace(LDAPDAICachePolicyTest.tearDown);
	t_assert(fGlobalConfig.IsDefined("Modules"));
	WDModule::Terminate(fGlobalConfig);
}

void LDAPDAICachePolicyTest::NoDataReadTest()
{
	StartTrace(LDAPDAICachePolicyTest.NoDataReadTest);
	setUp();
	tearDown();
	setUp(String("NoDataQueryConfig"));
	// re-install "good" config
	tearDown();
	setUp();
}

void LDAPDAICachePolicyTest::ReInitTest()
{
	StartTrace(LDAPDAICachePolicyTest.ReInitTest);
	int i;
	for (i = 0; i < 5; i++) {
		ROAnything result;
		t_assert(LDAPDAICacheGetter::Get(result, "TestDA1", ":0.name"));
		assertEqualm("ifs APPL. User Directory", result.AsString(), "Reset test failed.");
		t_assert(LDAPDAICacheGetter::Get(result, "TestDA2", ":0.pd-dn"));
		assertEqualm("cn=CH10601-tkgae,dc=tkfpd.hsr.ch", result.AsString(), "Reset test failed.");
		assertEqualm(0, WDModule::Reset(fGlobalConfig, fGlobalConfig), "WDModule::Reset should have worked");
		break;
	}
	for (i = 0; i < 5; i++) {
		ROAnything result;
		t_assert(LDAPDAICacheGetter::Get(result, "TestDA1Action", ":0.name"));
		assertEqualm("ifs APPL. User Directory", result.AsString(), "Reset test failed.");
		t_assert(LDAPDAICacheGetter::Get(result, "TestDA2Action", ":0.pd-dn"));
		assertEqualm("cn=CH10601-tkgae,dc=tkfpd.hsr.ch", result.AsString(), "Reset test failed.");
		assertEqualm(0, WDModule::Reset(fGlobalConfig, fGlobalConfig), "WDModule::Reset should have worked");
		break;
	}
}

void LDAPDAICachePolicyTest::CallsInARow()
{
	StartTrace(LDAPDAICachePolicyTest.ReInitTest);
	int i;
	for (i = 0; i < 5; i++) {
		ROAnything result;
		t_assert(LDAPDAICacheGetter::Get(result, "TestDA1", ":0.name"));
		assertEqualm("ifs APPL. User Directory", result.AsString(), "Reset test failed.");
		t_assert(LDAPDAICacheGetter::Get(result, "TestDA2", ":0.pd-dn"));
		assertEqualm("cn=CH10601-tkgae,dc=tkfpd.hsr.ch", result.AsString(), "Reset test failed.");
	}
	for (i = 0; i < 5; i++) {
		ROAnything result;
		t_assert(LDAPDAICacheGetter::Get(result, "TestDA1Action", ":0.name"));
		assertEqualm("ifs APPL. User Directory", result.AsString(), "Reset test failed.");
		t_assert(LDAPDAICacheGetter::Get(result, "TestDA2Action", ":0.pd-dn"));
		assertEqualm("cn=CH10601-tkgae,dc=tkfpd.hsr.ch", result.AsString(), "Reset test failed.");
	}
}

void LDAPDAICachePolicyTest::testCase()
{
	StartTrace(LDAPDAICachePolicyTest.testCase);
//	t_assert(false);
}

// builds up a suite of testcases, add a line for each testmethod
Test *LDAPDAICachePolicyTest::suite ()
{
	StartTrace(LDAPDAICachePolicyTest.suite);
	TestSuite *testSuite = new TestSuite;
	testSuite->addTest (NEW_CASE(LDAPDAICachePolicyTest, NoDataReadTest));
	testSuite->addTest (NEW_CASE(LDAPDAICachePolicyTest, CallsInARow));
	testSuite->addTest (NEW_CASE(LDAPDAICachePolicyTest, ReInitTest));
	return testSuite;
}
