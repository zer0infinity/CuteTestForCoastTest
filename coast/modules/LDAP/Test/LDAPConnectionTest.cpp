/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

//--- interface include --------------------------------------------------------
#include "LDAPConnectionTest.h"

//--- module under test --------------------------------------------------------
#include "LDAPConnection.h"
#include "LDAPErrorHandler.h"
#include "Context.h"
#include "Mapper.h"

//--- test modules used --------------------------------------------------------
#include "TestSuite.h"

//--- standard modules used ----------------------------------------------------
#include "Dbg.h"

//--- c-modules used -----------------------------------------------------------

//---- LDAPConnectionTest ----------------------------------------------------------------
LDAPConnectionTest::LDAPConnectionTest(TString tstrName)
	: ConfiguredTestCase(tstrName, "LDAPConnectionTestConfig")
{
	StartTrace(LDAPConnectionTest.Ctor);
}

LDAPConnectionTest::~LDAPConnectionTest()
{
	StartTrace(LDAPConnectionTest.Dtor);
}

// setup for this ConfiguredTestCase
void LDAPConnectionTest::setUp ()
{
	StartTrace(LDAPConnectionTest.setUp);
	ConfiguredTestCase::setUp();
}

void LDAPConnectionTest::tearDown ()
{
	StartTrace(LDAPConnectionTest.tearDown);
	ConfiguredTestCase::tearDown();
}

void LDAPConnectionTest::ConnectionTest()
{
	StartTrace(LDAPConnectionTest.ConnectionTest);
	FOREACH_ENTRY("ConnectionTest", cConfig, caseName) {
		Trace("At test entry: " << i);

		for ( long l = 0; l < cConfig["NumberOfConnects"].AsLong(1); l++ ) {
			Anything params;
			params["Server"] 			= cConfig["LDAPServer"].AsString();
			params["Port"] 				= cConfig["LDAPPort"].AsLong();
			params["Timeout"] 			= cConfig["LDAPTimeout"].AsLong();
			params["ConnectionTimeout"] = cConfig["LDAPConnectionTimeout"].AsLong(0);
			params["BindName"] 			= cConfig["LDAPBindName"].AsString();
			params["BindPW"] 			= cConfig["LDAPBindPW"].AsString();
			params["PooledConnections"]	= cConfig["LDAPPooledConnections"].AsLong(0L);
			params["RebindTimeout"]		= cConfig["LDAPRebindTimeout"].AsLong(3600L);
			params["TryAutoRebind"]		= cConfig["LDAPTryAutoRebind"].AsLong(0L);
			params["MaxConnections"]	= cConfig["LDAPMaxConnections"].AsLong(2L);

			Context ctx;
			ParameterMapper pm("ConnectionTestParameterMapper");
			ResultMapper rm("ConnectionTestResultMapper");
			rm.CheckConfig("ResultMapper");
			String da("DataAccess_");
			da << i;

			LDAPErrorHandler eh(ctx, &pm, &rm, da);
			eh.PutConnectionParams(params);

			// connect
			LDAPConnection lc(params);
			LDAPConnection::EConnectState eConnectState = lc.DoConnect(params, eh);
			String result(LDAPConnection::ConnectRetToString(eConnectState));
			Trace("Connect result: " << result);
			// check for errors
			Anything error;
			if ( !eh.GetError(error) ) {
				Trace("No error reported.");
			} else {
				TraceAny(error, "Error description:");
			}

			// compare result and expected error
			assertEqual(cConfig["ConnectRet"].AsString(), result);
			bool ret = LDAPConnection::IsConnectOk(eConnectState);
			assertEqual(cConfig["ConnectIsOk"].AsBool(1), ret);
			if (!ret) {
				assertAnyEqual(cConfig["Error"], error);
			}
			// now release sema and lock
			lc.ReleaseHandleInfo();
		}
	}
}

// builds up a suite of ConfiguredTestCases, add a line for each testmethod
Test *LDAPConnectionTest::suite ()
{
	StartTrace(LDAPConnectionTest.suite);
	TestSuite *testSuite = new TestSuite;

	ADD_CASE(testSuite, LDAPConnectionTest, ConnectionTest);

	return testSuite;
}
