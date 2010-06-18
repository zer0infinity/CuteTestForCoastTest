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
	params["Server"] 			= GetConfig()[name]["LDAPServer"].AsString();
	params["Port"] 				= GetConfig()[name]["LDAPPort"].AsLong();
	params["BindName"] 			= GetConfig()[name]["LDAPBindName"].AsString();
	params["BindPW"] 			= GetConfig()[name]["LDAPBindPW"].AsString();
	params["PooledConnections"]	= GetConfig()[name]["LDAPPooledConnections"].AsLong(0L);
	params["ConnectionTimeout"]	= GetConfig()[name]["LDAPConnectionTimeout"].AsLong();

	badConnectionPoolId = PersistentLDAPConnection::GetLdapPoolId(GetConfig()[name]["LDAPServer"].AsString(),
						  GetConfig()[name]["LDAPPort"].AsLong(0),
						  GetConfig()[name]["LDAPBindName"].AsString(),
						  GetConfig()[name]["LDAPBindPW"].AsString(),
						  GetConfig()[name]["LDAPConnectionTimeout"].AsLong() * 1000L);

	maxBadConnections = GetConfig()[name]["LDAPMaxConnections"].AsLong(5L);
	Context ctx;
	ParameterMapper pm("ConnectionTestParameterMapper");
	ResultMapper rm("ConnectionTestResultMapper");
	t_assert(pm.Initialize("ParameterMapper"));
	t_assert(rm.Initialize("ResultMapper"));
	String da("DataAccess_TestAutoRebindBadConnection");
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
			String poolId = fetchPoolId(pGetter);
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
			String poolId = fetchPoolId(pGetter);
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

String LDAPConnectionManagerTest::fetchPoolId(ParameterMapper *pGetter) {
	Context ctx;
	String server("No server specified!");
	pGetter->Get("LDAPServer", server, ctx);
	String bindName("No bind name specified!");
	pGetter->Get("LDAPBindName", bindName, ctx);
	String bindPw("No bind password specified!");
	pGetter->Get("LDAPBindPW", bindPw, ctx);
	long port = 0L;
	pGetter->Get("LDAPPort", port, ctx);
	long connectionTimeout = 0L;
	pGetter->Get("LDAPConnectionTimeout", connectionTimeout, ctx);

	return PersistentLDAPConnection::GetLdapPoolId(server, port, bindName, bindPw, connectionTimeout * 1000L);
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
