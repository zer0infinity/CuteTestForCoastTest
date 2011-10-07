/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#include "TestSuite.h"
#include "Page.h"
#include "PageTest.h"
#include "TestAction.h"
#include <iostream>

PageTest::PageTest(TString tname) : TestCaseType(tname)
{
	StartTrace(PageTest.Ctor);
}

PageTest::~PageTest()
{
	StartTrace(PageTest.Dtor);
}

void PageTest::setUp ()
{
	StartTrace(PageTest.setUp);
	Action *testAction = Action::FindAction("TestAction");
	if (testAction != 0) {
		testAction->Register("ChangeMeTrue", "Action");
		testAction->Register("ReturnFalse", "Action");

		fActionConfig["TestAction"] = "";
		fActionConfig["ConfiguredTestAction"]["RetValue"] = false;
		fActionConfig["ConfiguredTestAction"]["ActionToken"] = "ConfigChanged";

		Anything tmpStore(fCtx.GetTmpStore());
		tmpStore["DoTest"] = fActionConfig;
	} else {
		std::cerr << "couldn't find TestAction" << std::endl;
	}
}

void PageTest::FinishTest()
{
	StartTrace(PageTest.FinishTest);

	Page p("test");
	p.Initialize("Page");
	String transitionToken;

	transitionToken = "NoAction";
	t_assert(p.Finish(transitionToken, fCtx));
	assertEqual("NoAction", transitionToken);

	Anything tmpStore = fCtx.GetTmpStore();
	tmpStore["TestAction"] = "";
	tmpStore["ConfiguredTestAction"] = "";

	transitionToken = "ChangeMeTrue";
	t_assert(p.Finish(transitionToken, fCtx));
	assertEqual("Changed", transitionToken);
	assertEqual("ChangeMeTrue", tmpStore["TestAction"].AsString("x"));

	tmpStore["TestAction"] = "";
	tmpStore["ConfiguredTestAction"] = "";

	transitionToken = "ReturnFalse";
	t_assert(!p.Finish(transitionToken, fCtx));
	assertEqual("ReturnFalse", transitionToken);
	assertEqual("ReturnFalse", tmpStore["TestAction"].AsString("x"));

	tmpStore["TestAction"] = "";
	tmpStore["ConfiguredTestAction"] = "";

	transitionToken = "DoTest";
	t_assert(!p.Finish(transitionToken, fCtx));
	assertEqual("ConfigChanged", transitionToken);
	// Check if both actions took place
	assertEqual("DoTest", tmpStore["TestAction"].AsString("x"));
	assertEqual("DoTest", tmpStore["ConfiguredTestAction"].AsString("x"));
}

void PageTest::PrepareTest()
{
	StartTrace(PageTest.PrepareTest);

	Page p("test");
	p.Initialize("Page");
	String transitionToken("");

	t_assert(p.Prepare(transitionToken, fCtx));
	assertEqual("", transitionToken);

	transitionToken = "NoAction";
	t_assert(p.Prepare(transitionToken, fCtx));
	assertEqual("NoAction", transitionToken);

	Anything tmpStore = fCtx.GetTmpStore();
	tmpStore["TestAction"] = "";
	tmpStore["ConfiguredTestAction"] = "";

	transitionToken = "ChangeMeTrue";
	t_assert(p.Prepare(transitionToken, fCtx));
	assertEqual("Changed", transitionToken);

	tmpStore["TestAction"] = "";
	tmpStore["ConfiguredTestAction"] = "";

	transitionToken = "ReturnFalse";
	t_assert(!p.Prepare(transitionToken, fCtx));
	assertEqual("ReturnFalse", transitionToken);

	tmpStore["TestAction"] = "";
	tmpStore["ConfiguredTestAction"] = "";

	transitionToken = "DoTest";
	t_assert(!p.Finish(transitionToken, fCtx));
	assertEqual("ConfigChanged", transitionToken);
	// Check if both actions took place
	assertEqual("DoTest", tmpStore["TestAction"].AsString("x"));
	assertEqual("DoTest", tmpStore["ConfiguredTestAction"].AsString("x"));

}

// builds up a suite of testcases, add a line for each testmethod
Test *PageTest::suite ()
{
	StartTrace(PageTest.suite);
	TestSuite *testSuite = new TestSuite;

	ADD_CASE(testSuite, PageTest, FinishTest);
	ADD_CASE(testSuite, PageTest, PrepareTest);

	return testSuite;

}
