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
LDAPConnectionManagerTest::LDAPConnectionManagerTest(TString name)
	: ConfiguredTestCase(name, "LDAPConnectionManagerTestConfig")
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
	ConfiguredTestCase::setUp();
}

void LDAPConnectionManagerTest::tearDown ()
{
	StartTrace(LDAPConnectionManagerTest.tearDown);
	ConfiguredTestCase::tearDown();
}

void LDAPConnectionManagerTest::testLDAPConnectionManager()
{
	StartTrace(LDAPConnectionManagerTest.ConnectionTest);
	FOREACH_ENTRY("testLDAPConnectionManager", caseConfig, caseName) {
		Trace("Running " << caseName << " Test");

		for ( long l = 0; l < caseConfig["NumberOfRuns"].AsLong(1); l++ ) {
			if ( caseConfig.IsDefined("Set") ) {
				Anything set = caseConfig["Set"].DeepClone();
				String server = caseConfig["Data"]["LDAPServer"].AsString();
				long port = caseConfig["Data"]["LDAPServer"].AsLong(389);
				LDAP *handle = ::ldap_init( server, port);
				t_assert(handle != (LDAP *) NULL);
				String uniqueConnectionId = set["Data"]["UniqueConnectionId"].AsString();
				bool ret = LDAPConnectionManager::LDAPCONNMGR()->SetLdapConnection(uniqueConnectionId, handle);
				assertEqual(set["Expected"]["Return"].AsBool(true), ret);
			}
			if ( caseConfig.IsDefined("Get") ) {
				Anything get = caseConfig["Get"].DeepClone();
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

// builds up a suite of ConfiguredTestCases, add a line for each testmethod
Test *LDAPConnectionManagerTest::suite ()
{
	StartTrace(LDAPConnectionManagerTest.suite);
	TestSuite *testSuite = new TestSuite;

	ADD_CASE(testSuite, LDAPConnectionManagerTest, testLDAPConnectionManager);

	return testSuite;
}
