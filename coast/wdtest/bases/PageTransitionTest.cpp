/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

//--- interface include --------------------------------------------------------
#include "PageTransitionTest.h"

//--- test modules used --------------------------------------------------------
#include "TestSuite.h"

//--- standard modules used ----------------------------------------------------
#include "StringStream.h"
#include "Context.h"
#include "Role.h"
#include "WDModule.h"
#include "AnyUtils.h"
#include "Server.h"
#include "Session.h"
#include "Dbg.h"

class PTTestSession: public Session
{
private:
	Anything	fSpecial;

public:
	PTTestSession(const char *name, String dbName, Context &ctx): Session(name, ctx) {
		fSpecial["Database"] = dbName;
	};

	bool Lookup(const char *key, ROAnything &result, char delim, char indexdelim) const {
		ROAnything store(fSpecial);
		if (store.LookupPath(result, key, delim, indexdelim)) {
			return true;
		}
		return Session::Lookup(key, result, delim, indexdelim);
	}
};

//---- PageTransitionTest ----------------------------------------------------------------
PageTransitionTest::PageTransitionTest(TString tname)
	: ConfiguredActionTest(tname, "PageTransitionTestConfig")
{
	StartTrace(PageTransitionTest.Ctor);
}

PageTransitionTest::~PageTransitionTest()
{
	StartTrace(PageTransitionTest.Dtor);

}

// setup for this TestCase
void PageTransitionTest::setUp ()
{
	StartTrace1(PageTransitionTest.setUp, " : >" << name() << "<");

	ConfiguredActionTest::setUp();

	fServer = Server::FindServer("Server");
	t_assert ( fServer != 0);
	fServer->CheckConfig("Server");

} // setUp

void PageTransitionTest::RunTestCases()
{
	StartTrace(PageTransitionTest.RunTestCases);

	Anything testCases;
	long runOnlySz = fConfig["RunOnly"].GetSize();
	if (runOnlySz > 0) {
		cout << "PageTransitionTest not complete : Running only " << runOnlySz << " Testcases" << endl;
		for (long i = 0; i < runOnlySz; i++) {
			String testCaseName = fConfig["RunOnly"][i].AsString("Unknown");
			testCases[testCaseName] = fConfig["TestCases"][testCaseName];
		}
		TraceAny(testCases, "TestCases");
	} else {
		testCases = fConfig["TestCases"];
	}

	long sz = testCases.GetSize();
	for (long i = 0; i < sz; i++) {
		String testCaseName = testCases.SlotName(i);
		DoTest(PrepareConfig(testCases[i]), testCaseName);
	}
}

Anything PageTransitionTest::PrepareConfig(Anything originalConfig)
{
	StartTrace(PageTransitionTest.PrepareConfig);

	if (!originalConfig.IsDefined("UseConfig")) {
		return originalConfig;
	}

	String useConfigName = originalConfig["UseConfig"].AsString();
	Anything result = PrepareConfig(fConfig["TestCases"][useConfigName].DeepClone());

	Anything replaceList = originalConfig["Replace"];
	Anything slotPutConfig;
	long sz = replaceList.GetSize();
	for (long i = 0; i < sz; i++) {
		String destSlot = replaceList.SlotName(i);
		slotPutConfig["Slot"] = destSlot;
		SlotPutter::Operate(replaceList[i], result, slotPutConfig);
	}

	TraceAny(result, "Patched Config");
	return result;
}

void PageTransitionTest::DoTest(Anything config, const char *testCaseName)
{
	StartTrace1(PageTransitionTest.DoTest, "<" << NotNull(testCaseName) << ">");

	// SetUp  -------------------------------------------------
	Context theContext(config);
	theContext.SetServer(fServer);
	PTTestSession theSession("PTTestSession", fConfig["Database"].AsString("test"), theContext);
	theContext.Push(&theSession);
	theContext.SetRole(theSession.GetRole(theContext));

	theSession.Init(testCaseName, theContext);

	Role *theRole = Role::FindRole(config["StartingRole"].AsString("Role"));
	theSession.SetRole(theRole, theContext);

	TraceAny(theContext.GetSessionStore(), "SessionStore Before 1");
	PutInStore(config["LoadIntoSessionStore"], theContext.GetSessionStore());
	PutInStore(config["LoadIntoTempStore"], theContext.GetTmpStore());

	String transitionToken, newPage;
	theSession.SetupContext(theContext, transitionToken, newPage);

	TraceAny(theContext.GetSessionStore(), "SessionStore Before 2");
	// Process -------------------------------------------------

	theSession.DoFindNextPage(theContext, transitionToken, newPage);

	// Verify --------------------------------------------------
	Anything expected = config["Results"];

	assertEqualm(expected["NewPage"].AsString("x"), newPage, testCaseName);
	String roleName;
	t_assertm(theContext.GetRole()->GetName(roleName), testCaseName);
	assertEqualm(expected["NewRole"].AsString("x"), roleName, testCaseName);

	// Remove Timestamps etc from Stores
	Anything removeFromTempStoreList = fConfig["RemoveLists"]["TempStore"];
	Anything tempStore = theContext.GetTmpStore();
	long sz = removeFromTempStoreList.GetSize(), i;
	for (i = 0; i < sz; i++) {
		Anything toRemove = removeFromTempStoreList[i];
		Anything parent;
		if (tempStore.LookupPath(parent, toRemove["Parent"].AsString("-"))) {
			parent.Remove(toRemove["Slot"].AsString("-"));
		}
	}

	Anything removeFromSessionStoreList = fConfig["RemoveLists"]["SessionStore"];
	Anything sessionStore = theContext.GetSessionStore();
	long sz2 = removeFromSessionStoreList.GetSize();
	for (i = 0; i < sz2; i++) {
		Anything toRemove = removeFromSessionStoreList[i];
		Anything parent;
		if (sessionStore.LookupPath(parent, toRemove["Parent"].AsString("-"))) {
			parent.Remove(toRemove["Slot"].AsString("-"));
		}
	}

	DoCheckStores(expected, theContext, testCaseName);
	Context checkContext(config["env"], config["query"], fServer, &theSession, theSession.GetRole(theContext));
}

// builds up a suite of testcases, add a line for each testmethod
Test *PageTransitionTest::suite ()
{
	StartTrace(PageTransitionTest.suite);
	TestSuite *testSuite = new TestSuite;

	testSuite->addTest (NEW_CASE(PageTransitionTest, RunTestCases));

	return testSuite;

} // suite
