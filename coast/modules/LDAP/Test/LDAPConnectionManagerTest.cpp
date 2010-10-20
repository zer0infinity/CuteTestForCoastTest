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
#include "LDAPConnectionManager.h"

//--- test modules used --------------------------------------------------------
#include "TestSuite.h"

//--- standard modules used ----------------------------------------------------
#include "PersistentLDAPConnection.h"
#include "Mapper.h"
#include "AnyIterators.h"
#include "SystemBase.h"

//--- c-modules used -----------------------------------------------------------

//---- LDAPConnectionManagerTest ----------------------------------------------------------------
LDAPConnectionManagerTest::LDAPConnectionManagerTest(TString tstrName)
	: ConfiguredActionTest(tstrName)
{
	StartTrace(LDAPConnectionManagerTest.LDAPConnectionManagerTest);
}

TString LDAPConnectionManagerTest::getConfigFileName()
{
	return "LDAPConnectionManagerTestConfig";
}

LDAPConnectionManagerTest::~LDAPConnectionManagerTest()
{
	StartTrace(LDAPConnectionManagerTest.Dtor);
}

void LDAPConnectionManagerTest::ConnectionManagerTest()
{
	StartTrace(LDAPConnectionManagerTest.ConnectionManagerTest);
	ROAnything caseConfig;
	AnyExtensions::Iterator<ROAnything> aEntryIterator(GetTestCaseConfig());
	while ( aEntryIterator.Next(caseConfig) ) {
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
					Coast::System::MicroSleep(sleepTime * 1000000L);
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

LDAP *LDAPConnectionManagerTest::CreateBadConnectionHandle(const String &tname, String &badConnectionPoolId, long &maxBadConnections)
{
	StartTrace(LDAPConnectionManagerTest.CreateBadConnectionHandle);
	ParameterMapper pm("ConnectionTestParameterMapper");
	ResultMapper rm("ConnectionTestResultMapper");
	t_assert(pm.Initialize("ParameterMapper"));
	t_assert(rm.Initialize("ResultMapper"));
	Anything params = getConnectionParams(&pm);

	badConnectionPoolId = PersistentLDAPConnection::GetLdapPoolId(params["Server"].AsString(),
							params["Port"].AsLong(0),
							params["BindName"].AsString(),
							params["BindPW"].AsString(),
							params["ConnectionTimeout"].AsLong() * 1000L);

	maxBadConnections = GetConfig()[tname]["LDAPMaxConnections"].AsLong(5L);

	String da("DataAccess_TestAutoRebindBadConnection");
	Context ctx;
	LDAPErrorHandler eh(ctx, &pm, &rm, da);
	eh.PutConnectionParams(params);
	// connect
	LDAPConnection lc(params);
	LDAPConnection::EConnectState eConnectState = lc.DoConnect(params, eh);
	assertEqual("eBindNok", LDAPConnection::ConnectRetToString(eConnectState));
	return lc.GetConnection();
}

void LDAPConnectionManagerTest::NoAutoRebindTest()
{
	StartTrace(LDAPConnectionManagerTest.NoAutoRebindTest);

	ROAnything caseConfig;
	AnyExtensions::Iterator<ROAnything> aEntryIterator(GetTestCaseConfig());

	while ( aEntryIterator.Next(caseConfig) ) {
		TraceAny(caseConfig["ConfiguredActionTestAction"], "ConfiguredActionTestAction");
		String testCaseName = caseConfig["ConfiguredActionTestAction"].SlotName(0);
		ParameterMapper *pGetter = ParameterMapper::FindParameterMapper(testCaseName);

		if ( t_assertm(pGetter!=0, TString("could not find ParameterMapper [").Append(testCaseName).Append(']')) ) {
			Anything params = getConnectionParams(pGetter);
			String poolId = PersistentLDAPConnection::GetLdapPoolId(params["Server"].AsString(),
					params["Port"].AsLong(0),
					params["BindName"].AsString(),
					params["BindPW"].AsString(),
					params["ConnectionTimeout"].AsLong() * 1000L);
			Trace("poolId: " << poolId);
			DoTest(PrepareConfig(GetConfig()["NoAutoRetryOkQueryTest"][0L]["ConfiguredActionTestAction"][0L].DeepClone()),
				   GetConfig()["NoAutoRetryOkQueryTest"][0L]["ConfiguredActionTestAction"].SlotName(0));
			String badConnectionPoolId;
			long maxBadConnections;
			LDAP *badConnectionHandle = CreateBadConnectionHandle("TestAutoRebindBadConnection", badConnectionPoolId, maxBadConnections);
			assertEqual(true, LDAPConnectionManager::LDAPCONNMGR()->ReplaceHandlesForConnectionPool(poolId, badConnectionHandle));
			DoTest(PrepareConfig(caseConfig["ConfiguredActionTestAction"][0L].DeepClone()), testCaseName);
			assertEqual(true, LDAPConnectionManager::LDAPCONNMGR()->ReplaceHandlesForConnectionPool(poolId, ( LDAP * ) NULL ));
		}
	}
}

void LDAPConnectionManagerTest::AutoRebindTest()
{
	StartTrace(LDAPConnectionManagerTest.AutoRebindTest);

	ROAnything caseConfig;
	AnyExtensions::Iterator<ROAnything> aEntryIterator(GetTestCaseConfig());
	while ( aEntryIterator.Next(caseConfig) ) {
		String testCaseName = caseConfig["ConfiguredActionTestAction"].SlotName(0);
		ParameterMapper *pGetter = ParameterMapper::FindParameterMapper(testCaseName);

		if ( t_assertm(pGetter!=0, TString("could not find ParameterMapper [").Append(testCaseName).Append(']')) ) {
			Anything params = getConnectionParams(pGetter);
			String poolId = PersistentLDAPConnection::GetLdapPoolId(params["Server"].AsString(),
					params["Port"].AsLong(0),
					params["BindName"].AsString(),
					params["BindPW"].AsString(),
					params["ConnectionTimeout"].AsLong() * 1000L);
			Trace("poolId: " << poolId);
			TraceAny(caseConfig["ConfiguredActionTestAction"], "ConfiguredActionTestAction");
			DoTest(PrepareConfig(GetConfig()["TestAutoRebind"][0L]["ConfiguredActionTestAction"][0L].DeepClone()),
				   GetConfig()["TestAutoRebind"][0L]["ConfiguredActionTestAction"].SlotName(0));
			String badConnectionPoolId;
			long maxBadConnections;
			LDAP *badConnectionHandle = CreateBadConnectionHandle("TestAutoRebindBadConnection", badConnectionPoolId, maxBadConnections);
			assertEqual(true, LDAPConnectionManager::LDAPCONNMGR()->ReplaceHandlesForConnectionPool(poolId, badConnectionHandle));
			DoTest(PrepareConfig(caseConfig["ConfiguredActionTestAction"][0L].DeepClone()), testCaseName);
		}
	}
}

Anything LDAPConnectionManagerTest::getConnectionParams(ParameterMapper* pGetter) {
	Context ctx;
	Anything params;
	params["Server"] = "No server specified!";
	pGetter->Get("LDAPServer", params["Server"], ctx);
	params["BindName"] = "No bind name specified!";
	pGetter->Get("LDAPBindName", params["BindName"], ctx);
	params["BindPW"] = "No bind password specified!";
	pGetter->Get("LDAPBindPW", params["BindPW"], ctx);
	params["Port"] = 0L;
	pGetter->Get("LDAPPort", params["Port"], ctx);
	params["ConnectionTimeout"] = 0L;
	pGetter->Get("LDAPConnectionTimeout", params["ConnectionTimeout"], ctx);

	return params;
}

// builds up a suite of tests, add a line for each testmethod
Test *LDAPConnectionManagerTest::suite ()
{
	StartTrace(LDAPConnectionManagerTest.suite);
	TestSuite *testSuite = new TestSuite;
	ADD_CASE(testSuite, LDAPConnectionManagerTest, ConnectionManagerTest);
	ADD_CASE(testSuite, LDAPConnectionManagerTest, AutoRebindTest);
	ADD_CASE(testSuite, LDAPConnectionManagerTest, NoAutoRebindTest);

	return testSuite;
}
