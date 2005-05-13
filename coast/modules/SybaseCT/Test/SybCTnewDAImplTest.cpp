/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

//--- interface include --------------------------------------------------------
#include "SybCTnewDAImplTest.h"

//--- module under test --------------------------------------------------------
#include "SybCTnewDAImpl.h"

//--- test modules used --------------------------------------------------------
#include "TestSuite.h"

//--- standard modules used ----------------------------------------------------
#include "System.h"
#include "TimeStamp.h"
#include "Dbg.h"

//---- SybCTnewDAImplTest ----------------------------------------------------------------
SybCTnewDAImplTest::SybCTnewDAImplTest(TString name)
	: TestCase(name)
	, fbWasInitialized(false)
{
	StartTrace(SybCTnewDAImplTest.Ctor);
}

SybCTnewDAImplTest::~SybCTnewDAImplTest()
{
	StartTrace(SybCTnewDAImplTest.Dtor);
}

// setup for this TestCase
void SybCTnewDAImplTest::setUp()
{
	StartTrace(SybCTnewDAImplTest.setUp);
	fbWasInitialized = SybCTnewDAImpl::fgInitialized;
	if ( fbWasInitialized ) {
		SybCTnewDAImpl::Finis();
	}
}

void SybCTnewDAImplTest::tearDown()
{
	StartTrace(SybCTnewDAImplTest.tearDown);
	// set initialized state back here
	if ( fbWasInitialized ) {
		Anything anyConfig;
		if ( t_assert(System::LoadConfigFile(anyConfig, "Config")) ) {
			t_assert(SybCTnewDAImpl::Init(anyConfig));
		}
	}
}

void SybCTnewDAImplTest::UninitializedExecTest()
{
	StartTrace(SybCTnewDAImplTest.UninitializedExecTest);

	Anything params;
	params["SybDBUser"] = "wdtester";
	params["SybDBPW"] = "all2test";
	params["SybDBHost"] = "HIKU_INT2";
	params["SybDBApp"] = "SybCTnewDAImplTest";
	params["SQL"] = "select au_fname, au_lname from authors where au_lname='Bennet'";

	TraceAny(params, "Input params:");
	Context ctx;
	ctx.PushStore("Params", params);

	SybCTnewDAImpl da("SybSearchTestCoded");
	da.CheckConfig("SybCTnewDAImpl");
	ParameterMapper	inpMapper("SybSearchTestCoded");
	inpMapper.CheckConfig("ParameterMapper");
	ResultMapper outMapper("SybSearchTestCoded");
	outMapper.CheckConfig("ResultMapper");

	ctx.Push("DataAccess", &da);
	t_assertm( da.Exec(ctx, &inpMapper, &outMapper) == false, "DataAccess should have failed due to uninitialized SybCTnewDAImpl!");
	ctx.Remove("DataAccess");

	TraceAny(ctx.GetTmpStore(), "resulting tmpstore: ");
}

void SybCTnewDAImplTest::InitTest()
{
	StartTrace(SybCTnewDAImplTest.InitTest);
	Anything anyCfg, anyTmp;
	anyCfg["SybaseModule"] = MetaThing();
	anyTmp = anyCfg["SybaseModule"];
	if ( t_assert(SybCTnewDAImpl::Init(anyCfg)) ) {
		assertEqualm(5L, SybCTnewDAImpl::fgListOfSybCT["Size"].AsLong(-1L), "default coded size should be 5");
		t_assertm(SybCTnewDAImpl::fgpPeriodicAction != NULL, "expected PeriodicAction to be initialized");
		t_assert(SybCTnewDAImpl::Finis());
		t_assertm(SybCTnewDAImpl::fgpPeriodicAction == NULL, "expected PeriodicAction to be terminated");
	}
	anyTmp["SybCTnewDAImpl"] = MetaThing();
	anyTmp = anyTmp["SybCTnewDAImpl"];
	anyTmp["ParallelQueries"] = 2L;
	if ( t_assert(SybCTnewDAImpl::Init(anyCfg)) ) {
		assertEqualm(2L, SybCTnewDAImpl::fgListOfSybCT["Size"].AsLong(-1L), "size should now be 2");
		t_assertm(SybCTnewDAImpl::fgpPeriodicAction != NULL, "expected PeriodicAction to be initialized");
		t_assert(SybCTnewDAImpl::Finis());
		t_assertm(SybCTnewDAImpl::fgpPeriodicAction == NULL, "expected PeriodicAction to be terminated");
	}
}

void SybCTnewDAImplTest::DoGetConnectionTest()
{
	StartTrace(SybCTnewDAImplTest.DoGetConnectionTest);
	Anything anyCfg, anyTmp;
	anyTmp["ParallelQueries"] = 2L;
	anyCfg["SybaseModule"]["SybCTnewDAImpl"] = anyTmp;
	if ( t_assert(SybCTnewDAImpl::Init(anyCfg)) ) {
		assertEqualm(2L, SybCTnewDAImpl::fgListOfSybCT["Size"].AsLong(-1L), "size should now be 2");
		SybCTnewDA *pSyb[3L] = { 0 };
		bool bIsOpen;
		String server, user;
		assertEqual(2L, SybCTnewDAImpl::fgListOfSybCT["Unused"].GetSize());
		t_assertm(SybCTnewDAImpl::DoGetConnection(pSyb[0L], bIsOpen, server, user), "expected valid connection");
		t_assertm( !bIsOpen, "expected unopen connection");
		t_assertm( pSyb[0L] != NULL, "expected valid SybCTnewDA *");
		assertEqual(1L, SybCTnewDAImpl::fgListOfSybCT["Unused"].GetSize());
		t_assertm(SybCTnewDAImpl::DoGetConnection(pSyb[1L], bIsOpen, server, user), "expected valid connection");
		t_assertm( !bIsOpen, "expected unopen connection");
		t_assertm( pSyb[1L] != NULL, "expected valid SybCTnewDA *");
		assertEqual(0L, SybCTnewDAImpl::fgListOfSybCT["Unused"].GetSize());
		// the following call would block so we can not test it easy
//		t_assertm( !SybCTnewDAImpl::DoGetConnection(pSyb[2L], bIsOpen, server, user), "expected invalid connection");
//		t_assertm( pSyb[2L] == NULL, "expected invalid SybCTnewDA *");
		t_assert(SybCTnewDAImpl::Finis());
	}
	if ( t_assert(SybCTnewDAImpl::Init(anyCfg)) ) {
		assertEqualm(2L, SybCTnewDAImpl::fgListOfSybCT["Size"].AsLong(-1L), "size should now be 2");
		assertEqual(2L, SybCTnewDAImpl::fgListOfSybCT["Unused"].GetSize());
		SybCTnewDA *pSyb[3L] = { 0 };
		bool bIsOpen;
		String server("SRV1"), user("USR1");
		t_assertm(SybCTnewDAImpl::DoGetConnection(pSyb[0L], bIsOpen, server, user), "expected valid connection");
		t_assertm( !bIsOpen, "expected unopen connection");
		t_assertm( pSyb[0L] != NULL, "expected valid SybCTnewDA *");
		SybCTnewDAImpl::DoPutbackConnection(pSyb[0L], true, server, user);
		assertEqual(1L, SybCTnewDAImpl::fgListOfSybCT["Unused"].GetSize());
		assertEqual(1L, SybCTnewDAImpl::fgListOfSybCT["Open"].GetSize());
		t_assertm(SybCTnewDAImpl::DoGetConnection(pSyb[0L], bIsOpen, server, user), "expected valid connection");
		t_assertm( bIsOpen, "expected open connection");
		t_assertm( pSyb[0L] != NULL, "expected valid SybCTnewDA *");
		SybCTnewDAImpl::DoPutbackConnection(pSyb[0L], true, server, user);
		user = "USR2";
		t_assertm(SybCTnewDAImpl::DoGetConnection(pSyb[0L], bIsOpen, server, user), "expected valid connection");
		t_assertm( !bIsOpen, "expected unopen connection");
		t_assertm( pSyb[0L] != NULL, "expected valid SybCTnewDA *");

		t_assert(SybCTnewDAImpl::Finis());
	}
}

void SybCTnewDAImplTest::DoPutbackConnectionTest()
{
	StartTrace(SybCTnewDAImplTest.DoPutbackConnectionTest);
	Anything anyCfg, anyTmp;
	anyTmp["ParallelQueries"] = 2L;
	anyCfg["SybaseModule"]["SybCTnewDAImpl"] = anyTmp;
	if ( t_assert(SybCTnewDAImpl::Init(anyCfg)) ) {
		assertEqualm(2L, SybCTnewDAImpl::fgListOfSybCT["Size"].AsLong(-1L), "size should now be 2");
		assertEqual(2L, SybCTnewDAImpl::fgListOfSybCT["Unused"].GetSize());
		SybCTnewDA *pSyb[3L] = { 0 };
		bool bIsOpen;
		String server("SRV1"), user("USR1");
		t_assertm(SybCTnewDAImpl::DoGetConnection(pSyb[0L], bIsOpen, server, user), "expected valid connection");
		t_assertm( !bIsOpen, "expected unopen connection");
		t_assertm( pSyb[0L] != NULL, "expected valid SybCTnewDA *");
		t_assertm(SybCTnewDAImpl::DoGetConnection(pSyb[1L], bIsOpen, server, user), "expected valid connection");
		t_assertm( !bIsOpen, "expected unopen connection");
		t_assertm( pSyb[1L] != NULL, "expected valid SybCTnewDA *");

		SybCTnewDAImpl::DoPutbackConnection(pSyb[0L], true, server, user);
		assertEqual(1L, SybCTnewDAImpl::fgListOfSybCT["Open"].GetSize());
		Thread::Wait(1L);
		user = "USR2";
		SybCTnewDAImpl::DoPutbackConnection(pSyb[1L], true, server, user);
		assertEqual(2L, SybCTnewDAImpl::fgListOfSybCT["Open"].GetSize());

		assertEqualm("SRV1.USR1", SybCTnewDAImpl::fgListOfSybCT["Open"][0L][0L][1L].AsString(), "expected SRV1.USR1");
		assertEqualm("SRV1.USR2", SybCTnewDAImpl::fgListOfSybCT["Open"][1L][0L][1L].AsString(), "expected SRV1.USR2");

		t_assertm( TimeStamp(SybCTnewDAImpl::fgListOfSybCT["Open"].SlotName(0L)) < TimeStamp(SybCTnewDAImpl::fgListOfSybCT["Open"].SlotName(1L)), "expected timestamps to be different");
		t_assert(SybCTnewDAImpl::Finis());
	}
}

void SybCTnewDAImplTest::testCase()
{
	StartTrace(SybCTnewDAImplTest.testCase);

	Anything params;
	params["SybDBUser"] = "wdtester";
	params["SybDBPW"] = "all2test";
	params["SybDBHost"] = "HIKU_INT2";
	params["SybDBApp"] = "SybCTnewDAImplTest";
	params["SQL"] = "select au_fname, au_lname from authors where au_lname='Bennet'";

	TraceAny(params, "Input params:");
	Context ctx(params);

	Anything result;
	Anything expected;
	expected["Mapper"]["Query"] = params["SQL"];
	expected["Mapper"]["QueryCount"] = 1;
	expected["Mapper"]["QueryResult"]["0"]["au_fname"] = "Abraham";
	expected["Mapper"]["QueryResult"]["0"]["au_lname"] = "Bennet";
	expected["Mapper"]["QuerySource"] = "HIKU_INT2";
	TraceAny(expected, "expected tmpstore: ");
	// force load of configuration
	SybCTnewDAImpl da("SybSearchTestCoded");
	da.CheckConfig("SybCTnewDAImpl");
	ParameterMapper	inpMapper("SybSearchTestCoded");
	inpMapper.CheckConfig("ParameterMapper");
	ResultMapper	outMapper("SybSearchTestCoded");
	outMapper.CheckConfig("ResultMapper");

	ctx.Push("DataAccess", &da);
	t_assert(da.Exec(ctx, &inpMapper, &outMapper));
	ctx.Remove("DataAccess");

	result = ctx.GetTmpStore();
	Anything cfg;
	cfg["Slot"] = "Mapper.Messages";
	SlotCleaner::Operate(result, cfg);
	TraceAny(result, "resulting tmpstore: ");

	assertAnyEqual(expected, result);
}

// builds up a suite of testcases, add a line for each testmethod
Test *SybCTnewDAImplTest::suite ()
{
	StartTrace(SybCTnewDAImplTest.suite);
	TestSuite *testSuite = new TestSuite;

	ADD_CASE(testSuite, SybCTnewDAImplTest, UninitializedExecTest);
	ADD_CASE(testSuite, SybCTnewDAImplTest, InitTest);
	ADD_CASE(testSuite, SybCTnewDAImplTest, DoGetConnectionTest);
	ADD_CASE(testSuite, SybCTnewDAImplTest, DoPutbackConnectionTest);
///	ADD_CASE(testSuite, SybCTnewDAImplTest, testCase);

	return testSuite;

} // suite
