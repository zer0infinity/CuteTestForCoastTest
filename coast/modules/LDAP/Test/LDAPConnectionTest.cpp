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
LDAPConnectionTest::LDAPConnectionTest(TString name)
	: ConfiguredTestCase(name, "LDAPConnectionTestConfig")
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
	FOREACH_ENTRY("ConnectionTest", caseConfig, caseName) {
		Trace("Running " << caseName << " Test");

		Anything params;
		params["Server"] = caseConfig["LDAPServer"].AsString();
		params["Port"] = caseConfig["LDAPPort"].AsLong();
		params["Timeout"] = caseConfig["LDAPTimeout"].AsLong();
		params["ConnectionTimeout"] = caseConfig["LDAPConnectionTimeout"].AsLong();
		params["BindName"] = caseConfig["LDAPBindName"].AsString();
		params["BindPW"] = caseConfig["LDAPBindPW"].AsString();

		Context ctx;
		ParameterMapper pm("ConnectionTestParameterMapper");
		ResultMapper rm("ConnectionTestResultMapper");
		rm.CheckConfig("OutputMapper");
		String da(caseName);

		LDAPErrorHandler eh(ctx, &pm, &rm, da);
		eh.PutConnectionParams(params);

		// connect
		LDAPConnection lc(params);
		bool result = lc.Connect(params, eh);
		Trace("Connect success: " << result);

		// check for errors
		Anything error;
		if ( !eh.GetError(error) ) {
			Trace("No error reported.");
		} else {
			TraceAny(error, "Error description:");
		}

		// compare result and expected error
		assertEqual(caseConfig["Expected"].AsBool(), result);
		if (!result) {
			assertAnyEqual(caseConfig["Error"], error);
		}

		// NOTE: unbind happens automatically at destruction of lc
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
