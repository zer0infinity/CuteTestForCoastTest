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
#include "LDAPConnection.h"
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
	StartTrace(LDAPConnectionManagerTest.ConnectionTest);
	FOREACH_ENTRY("testLDAPConnectionManager", caseConfig, caseName) {
		Trace("At entry " << i);

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
					String uniqueConnectionId = set["Data"]["UniqueConnectionId"].AsString();
					bool ret = LDAPConnectionManager::LDAPCONNMGR()->SetLdapConnection(uniqueConnectionId, handle);
					assertEqual(set["Expected"]["Return"].AsBool(true), ret);
				}
				if ( caseConfig["Suite"][ll].IsDefined("Get") ) {
					Anything get = caseConfig["Suite"][ll]["Get"].DeepClone();
					String uniqueConnectionId = get["Data"]["UniqueConnectionId"].AsString();
					long rebindTimeout	= get["Data"]["LDAPRebindTimeout"].AsLong(0L);
					long sleepTime 		= get["Data"]["SleepTime"].AsLong(0L);
					System::MicroSleep(sleepTime * 1000000L);
					Anything returned;
					returned = LDAPConnectionManager::LDAPCONNMGR()->GetLdapConnection(uniqueConnectionId, rebindTimeout);
					assertEqual(get["Expected"]["MustRebind"].AsBool(true), returned["MustRebind"].AsBool(false));
				}
			}
		}
	}
}

void LDAPConnectionManagerTest::testAutoRebind()
{
	StartTrace(LDAPConnectionManagerTest.testAutoRebind);

	FOREACH_ENTRY("TestAutoRebind", caseConfig, caseName) {
		Trace("At entry " << i);
		TraceAny(caseConfig["ConfiguredActionTestAction"], "ConfiguredActionTestAction");
		String uniqueConnectionId = GetLdapConnectionManagerId(fConfig["LDAPConnectionData"]["LDAPServer"].AsString(),
									fConfig["LDAPConnectionData"]["LDAPPort"].AsLong(0),
									fConfig["LDAPConnectionData"]["LDAPBindName"].AsString(),
									fConfig["LDAPConnectionData"]["LDAPBindPW"].AsString(),
									fConfig["LDAPConnectionData"]["LDAPConnectionTimeout"].AsLong() * 1000L);
		Trace("uniqueConnectionId: " << uniqueConnectionId);
		String testCaseName = caseConfig["ConfiguredActionTestAction"].SlotName(0);
		DoTest(PrepareConfig(caseConfig["ConfiguredActionTestAction"][0L].DeepClone()), testCaseName);
//		System::MicroSleep(15 * 1000000L);
	}
}

String LDAPConnectionManagerTest::GetLdapConnectionManagerId(const String &server, long port, const String &bindName,
		const String &bindPW, long connectionTimeout)
{
	StartTrace(LDAPConnectionManagerTest.GetLdapConnectionManagerId);
	return	String("ThreadId[") << Thread::MyId() << "] Host[" << server << "] Port[" << port << "] DN[" <<
			bindName << "] BindPW[" << bindPW << "] ConnTimeout[" << connectionTimeout << "]";
}

// builds up a suite of ConfiguredTestCases, add a line for each testmethod
Test *LDAPConnectionManagerTest::suite ()
{
	StartTrace(LDAPConnectionManagerTest.suite);
	TestSuite *testSuite = new TestSuite;

	ADD_CASE(testSuite, LDAPConnectionManagerTest, testLDAPConnectionManager);
	ADD_CASE(testSuite, LDAPConnectionManagerTest, testAutoRebind);

	return testSuite;
}
