/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

//--- standard modules used ----------------------------------------------------
#include "Anything.h"
#include "Context.h"
#include "Dbg.h"

//--- test modules used --------------------------------------------------------
#include "TestSuite.h"

//--- module under test --------------------------------------------------------
#include "Action.h"

//--- interface include --------------------------------------------------------
#include "CopyActionsTest.h"

CopyActionsTest::CopyActionsTest(TString tname) : ConfiguredTestCase(tname, "CopyActionsTestConfig")
{
	StartTrace(CopyActionsTest.Ctor);
}

CopyActionsTest::~CopyActionsTest()
{
	StartTrace(CopyActionsTest.Dtor);
}

void CopyActionsTest::CopyActionTest()
{
	StartTrace(CopyActionsTest.CopyActionTest);
	// Set up
	Context c;
	Anything rStore = fConfig["RoleStore"];
	PutInStore(rStore, c.GetRoleStoreGlobal());
	Anything query = fConfig["Query"];
	PutInStore(query, c.GetQuery());
	Anything tStore = fConfig["TempStore"];
	PutInStore(tStore, c.GetTmpStore());

	// Process
	String ret1 = ExecAction("Copy1", c, true);
	String ret2 = ExecAction("Copy2", c, true);
	String ret3 = ExecAction("Copy3", c, true);
	String ret4 = ExecAction("Copy4", c, true);
	String ret5 = ExecAction("Copy5", c, true);
	String ret6 = ExecAction("Copy6", c, true);
	String ret7 = ExecAction("Copy7", c, true);
	String ret8 = ExecAction("Copy8", c, true);
	String ret9 = ExecAction("Copy9", c, true);
	String retw1 = ExecAction("CopyWrongConfig1", c, false);
	String retw2 = ExecAction("CopyWrongConfig2", c, false);

	TraceAny(c.GetTmpStore(), "TempStore after");
	TraceAny(c.GetRoleStoreGlobal(), "RoleStore after");
	TraceAny(c.GetQuery(), "Query after");

	// Verify
	assertEqual("Copy1", ret1);
	assertEqual("Copy2", ret2);
	assertEqual("Copy3", ret3);
	assertEqual("Copy4", ret4);
	assertEqual("Copy5", ret5);
	assertEqual("Copy6", ret6);
	assertEqual("Copy7", ret7);
	assertEqual("Copy8", ret8);
	assertEqual("Copy9", ret9);
	assertEqual("CopyWrongConfig1", retw1);
	assertEqual("CopyWrongConfig2", retw2);

	Anything sessionStore = c.GetSessionStore();
	Anything expectedSessionStore(fConfig["Results"]["SessionStore"]);
	assertAnyEqual(expectedSessionStore, sessionStore);

	Anything tmpStore = c.GetTmpStore();
	Anything expectedTempStore(fConfig["Results"]["TempStoreCopyResult"]);
	assertAnyEqual(expectedTempStore, tmpStore["CopyResult"]);
	expectedTempStore = fConfig["Results"]["Copy6Result"];
	assertAnyEqual(expectedTempStore, tmpStore["Copy6Fields"]);
}

String CopyActionsTest::ExecAction(String token, Context &c, bool expectedResult)
{
	StartTrace(CopyActionsTest.ExecAction);

	ROAnything config = c.Lookup(token);

	t_assert(Action::ExecAction(token, c, config) == expectedResult);

	return token;
}

Test *CopyActionsTest::suite ()
/* what: return the whole suite of tests for CopyActionsTest, add all top level
		 test functions here.
*/
{
	TestSuite *testSuite = new TestSuite;
	ADD_CASE(testSuite, CopyActionsTest, CopyActionTest);
	return testSuite;
} // suite
