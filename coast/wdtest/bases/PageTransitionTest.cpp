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
#include "Context.h"
#include "Role.h"
#include "AnyUtils.h"
#include "Server.h"
#include "Session.h"

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
	: ConfiguredActionTest(tname)
{
	StartTrace(PageTransitionTest.PageTransitionTest);
}

TString PageTransitionTest::getConfigFileName()
{
	return "PageTransitionTestConfig";
}

PageTransitionTest::~PageTransitionTest()
{
	StartTrace(PageTransitionTest.Dtor);
}

void PageTransitionTest::setUp ()
{
	StartTrace1(PageTransitionTest.setUp, " : >" << name() << "<");
	ConfiguredActionTest::setUp();
	fServer = Server::FindServer("Server");
	t_assert ( fServer != 0);
	fServer->CheckConfig("Server");
}

void PageTransitionTest::TestCases()
{
	StartTrace(PageTransitionTest.TestCases);

	Anything testCases;
	long runOnlySz = GetConfig()["RunOnly"].GetSize();
	if (runOnlySz > 0) {
		cout << "PageTransitionTest not complete : Running only " << runOnlySz << " Testcases" << endl;
		for (long i = 0; i < runOnlySz; i++) {
			String testCaseName = GetConfig()["RunOnly"][i].AsString("Unknown");
			testCases[testCaseName] = GetConfig()["TestCases"][testCaseName].DeepClone();
		}
		TraceAny(testCases, "TestCases");
	} else {
		testCases = GetConfig()["TestCases"].DeepClone();
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
	Anything result = PrepareConfig(GetConfig()["TestCases"][useConfigName].DeepClone());

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
	PTTestSession theSession("PTTestSession", GetConfig()["Database"].AsString("test"), theContext);
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
	Anything removeFromTempStoreList = GetConfig()["RemoveLists"]["TempStore"].DeepClone();
	Anything tempStore = theContext.GetTmpStore();
	long sz = removeFromTempStoreList.GetSize(), i;
	for (i = 0; i < sz; i++) {
		Anything toRemove = removeFromTempStoreList[i];
		Anything parent;
		if (tempStore.LookupPath(parent, toRemove["Parent"].AsString("-"))) {
			parent.Remove(toRemove["Slot"].AsString("-"));
		}
	}

	Anything removeFromSessionStoreList = GetConfig()["RemoveLists"]["SessionStore"].DeepClone();
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

	ADD_CASE(testSuite, PageTransitionTest, TestCases);

	return testSuite;

}
