/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

//--- interface include --------------------------------------------------------
#include "LDAPConnectionManagerTest.h"

//--- module under test --------------------------------------------------------
#include "PersistentLDAPConnection.h"
#include "LDAPConnectionManager.h"

//--- test modules used --------------------------------------------------------
#include "TestSuite.h"

//--- standard modules used ----------------------------------------------------
#include "Dbg.h"
#include "LDAPErrorHandler.h"
#include "Context.h"
#include "Mapper.h"
#include "Anything.h"
#include "System.h"

//--- c-modules used -----------------------------------------------------------

//---- LDAPConnectionManagerTest ----------------------------------------------------------------
LDAPConnectionManagerTest::LDAPConnectionManagerTest(TString tstrName)
	: ConfiguredActionTest(tstrName, "LDAPConnectionManagerTestConfig")
{
	StartTrace(LDAPConnectionManagerTest.Ctor);
}

LDAPConnectionManagerTest::~LDAPConnectionManagerTest()
{
	StartTrace(LDAPConnectionManagerTest.Dtor);
}

// setup for this ConfiguredTestCase
void LDAPConnectionManagerTest::setUp ()
{
	StartTrace(LDAPConnectionManagerTest.setUp);
	ConfiguredActionTest::setUp();
}

void LDAPConnectionManagerTest::tearDown ()
{
	StartTrace(LDAPConnectionManagerTest.tearDown);
	ConfiguredActionTest::tearDown();
}

void LDAPConnectionManagerTest::testLDAPConnectionManager()
{
	StartTrace(LDAPConnectionManagerTest.testLDAPConnectionManager);
	FOREACH_ENTRY("TestLDAPConnectionManager", caseConfig, caseName) {
		Trace("At entry " << i);
		long maxConnections = 0;
		String poolId;

		for ( long l = 0; l < caseConfig["NumberOfRuns"].AsLong(1); l++ ) {
			for ( long ll = 0; ll < caseConfig["Suite"].GetSize(); ll++ ) {
				if ( caseConfig["Suite"][ll].IsDefined("Set") ) {
					Anything set = caseConfig["Suite"][ll]["Set"].DeepClone();
					String server = set["Data"]["LDAPServer"].AsString();
					long port = set["Data"]["LDAPServer"].AsLong(389);
					LDAP *handle;
					if ( set["Data"]["NullHandle"].AsLong(0L) ) {
						Trace("Setting NULL handle");
						handle = (LDAP *) NULL;
						t_assert(handle == (LDAP *) NULL);
					} else {
						handle = ::ldap_init( server, port);
						t_assert(handle != (LDAP *) NULL);
					}
					poolId = set["Data"]["PoolId"].AsString();
					maxConnections = set["Data"]["LDAPMaxConnections"].AsLong(0);

					bool ret = LDAPConnectionManager::LDAPCONNMGR()->SetLdapConnection(maxConnections, poolId, handle);
					assertEqual(set["Expected"]["Return"].AsBool(true), ret);
				}
				if ( caseConfig["Suite"][ll].IsDefined("Get") ) {
					Anything get = caseConfig["Suite"][ll]["Get"].DeepClone();
					poolId = get["Data"]["PoolId"].AsString();
					long rebindTimeout	= get["Data"]["LDAPRebindTimeout"].AsLong(0L);
					maxConnections = get["Data"]["LDAPMaxConnections"].AsLong(0);
					long sleepTime 		= get["Data"]["SleepTime"].AsLong(0L);
					System::MicroSleep(sleepTime * 1000000L);
					Anything returned;
					returned = LDAPConnectionManager::LDAPCONNMGR()->GetLdapConnection(false, maxConnections, poolId, rebindTimeout);
					TraceAny(returned, "returned");
					TraceAny(get, "get");
					assertEqual(get["Expected"]["MustRebind"].AsBool(true), returned["MustRebind"].AsBool(false));
				}
			}
			LDAPConnectionManager::LDAPCONNMGR()->ReleaseHandleInfo(maxConnections, poolId);
		}
	}
}

LDAP *LDAPConnectionManagerTest::CreateBadConnectionHandle(const String &name, String &badConnectionPoolId, long &maxBadConnections)
{
	StartTrace(LDAPConnectionManagerTest.CreateBadConnectionHandle);
	Anything params;
	params["Server"] 			= fConfig[name]["LDAPServer"].AsString();
	params["Port"] 				= fConfig[name]["LDAPPort"].AsLong();
	params["BindName"] 			= fConfig[name]["LDAPBindName"].AsString();
	params["BindPW"] 			= fConfig[name]["LDAPBindPW"].AsString();
	params["PooledConnections"]	= fConfig[name]["LDAPPooledConnections"].AsLong(0L);
	params["ConnectionTimeout"]	= fConfig[name]["LDAPConnectionTimeout"].AsLong();
	badConnectionPoolId = PersistentLDAPConnection::GetLdapPoolId(fConfig[name]["LDAPServer"].AsString(),
						  fConfig[name]["LDAPPort"].AsLong(0),
						  fConfig[name]["LDAPBindName"].AsString(),
						  fConfig[name]["LDAPBindPW"].AsString(),
						  fConfig[name]["LDAPConnectionTimeout"].AsLong() * 1000L);

	maxBadConnections = fConfig[name]["LDAPMaxConnections"].AsLong(5L);
	Context ctx;
	ParameterMapper pm("ConnectionTestParameterMapper");
	ResultMapper rm("ConnectionTestResultMapper");
	rm.CheckConfig("ResultMapper");
	String da("DataAccess_TestAutoRebindBadConnection");
	LDAPErrorHandler eh(ctx, &pm, &rm, da);
	eh.PutConnectionParams(params);
	// connect
	LDAPConnection lc(params);
	LDAPConnection::EConnectState eConnectState = lc.DoConnect(params, eh);
	assertEqual("eBindNok", LDAPConnection::ConnectRetToString(eConnectState));
	return lc.GetConnection();
}

void LDAPConnectionManagerTest::testNoAutoRebind()
{
	StartTrace(LDAPConnectionManagerTest.testAutoRebind);

	FOREACH_ENTRY("TestNoAutoRebind", caseConfig, caseName) {
		Trace("At entry " << i);
		TraceAny(caseConfig["ConfiguredActionTestAction"], "ConfiguredActionTestAction");
		String poolId = PersistentLDAPConnection::GetLdapPoolId(fConfig["LDAPConnectionDataNoAutoRetry"]["LDAPServer"].AsString(),
						fConfig["LDAPConnectionDataNoAutoRetry"]["LDAPPort"].AsLong(0),
						fConfig["LDAPConnectionDataNoAutoRetry"]["LDAPBindName"].AsString(),
						fConfig["LDAPConnectionDataNoAutoRetry"]["LDAPBindPW"].AsString(),
						fConfig["LDAPConnectionDataNoAutoRetry"]["LDAPConnectionTimeout"].AsLong() * 1000L);
		Trace("poolId: " << poolId);
		DoTest(PrepareConfig(fConfig["TestNoAutoRetryOkQuery"][0L]["ConfiguredActionTestAction"][0L].DeepClone()),
			   fConfig["TestNoAutoRetryOkQuery"][0L]["ConfiguredActionTestAction"].SlotName(0));
		String badConnectionPoolId;
		long maxBadConnections;
		LDAP *badConnectionHandle = CreateBadConnectionHandle("TestAutoRebindBadConnection", badConnectionPoolId, maxBadConnections);
		assertEqual(true, LDAPConnectionManager::LDAPCONNMGR()->ReplaceHandlesForConnectionPool(poolId, badConnectionHandle));
		String testCaseName = caseConfig["ConfiguredActionTestAction"].SlotName(0);
		DoTest(PrepareConfig(caseConfig["ConfiguredActionTestAction"][0L].DeepClone()), testCaseName);
		assertEqual(true, LDAPConnectionManager::LDAPCONNMGR()->ReplaceHandlesForConnectionPool(poolId, ( LDAP * ) NULL ));
	}
}

void LDAPConnectionManagerTest::testAutoRebind()
{
	StartTrace(LDAPConnectionManagerTest.testAutoRebind);

	FOREACH_ENTRY("TestAutoRebind", caseConfig, caseName) {
		Trace("At entry " << i);
		TraceAny(caseConfig["ConfiguredActionTestAction"], "ConfiguredActionTestAction");
		String poolId = PersistentLDAPConnection::GetLdapPoolId(fConfig["LDAPConnectionDataAutoRetry"]["LDAPServer"].AsString(),
						fConfig["LDAPConnectionDataAutoRetry"]["LDAPPort"].AsLong(0),
						fConfig["LDAPConnectionDataAutoRetry"]["LDAPBindName"].AsString(),
						fConfig["LDAPConnectionDataAutoRetry"]["LDAPBindPW"].AsString(),
						fConfig["LDAPConnectionDataAutoRetry"]["LDAPConnectionTimeout"].AsLong() * 1000L);
		Trace("poolId: " << poolId);
		DoTest(PrepareConfig(fConfig["TestAutoRebind"][0L]["ConfiguredActionTestAction"][0L].DeepClone()),
			   fConfig["TestAutoRebind"][0L]["ConfiguredActionTestAction"].SlotName(0));
		String badConnectionPoolId;
		long maxBadConnections;
		LDAP *badConnectionHandle = CreateBadConnectionHandle("TestAutoRebindBadConnection", badConnectionPoolId, maxBadConnections);
		assertEqual(true, LDAPConnectionManager::LDAPCONNMGR()->ReplaceHandlesForConnectionPool(poolId, badConnectionHandle));
		String testCaseName = caseConfig["ConfiguredActionTestAction"].SlotName(0);
		DoTest(PrepareConfig(caseConfig["ConfiguredActionTestAction"][0L].DeepClone()), testCaseName);
	}
}

// builds up a suite of ConfiguredTestCases, add a line for each testmethod
Test *LDAPConnectionManagerTest::suite ()
{
	StartTrace(LDAPConnectionManagerTest.suite);
	TestSuite *testSuite = new TestSuite;

	ADD_CASE(testSuite, LDAPConnectionManagerTest, testLDAPConnectionManager);
	ADD_CASE(testSuite, LDAPConnectionManagerTest, testAutoRebind);
	ADD_CASE(testSuite, LDAPConnectionManagerTest, testNoAutoRebind);

	return testSuite;
}
