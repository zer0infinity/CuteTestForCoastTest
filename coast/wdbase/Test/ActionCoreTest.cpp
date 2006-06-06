/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

//--- interface include --------------------------------------------------------
#include "ActionCoreTest.h"

//--- test modules used --------------------------------------------------------
#include "TestSuite.h"

//--- standard modules used ----------------------------------------------------
#include "Context.h"
#include "Action.h"
#include "Dbg.h"

//--- c-library modules used ---------------------------------------------------

//---- ActionCoreTest ----------------------------------------------------------------
ActionCoreTest::ActionCoreTest(TString tname) : TestCaseType(tname)
{
	StartTrace(ActionCoreTest.Ctor);
}

ActionCoreTest::~ActionCoreTest()
{
	StartTrace(ActionCoreTest.Dtor);
}

void ActionCoreTest::ConsistentTransitionHandling()
{
	StartTrace(ActionCoreTest.ConsistentTransitionHandling);

	String transitionToken;
	Context ctx;

	transitionToken = "NoAction";
	t_assert(Action::ExecAction(transitionToken, ctx));
	assertEqual("NoAction", transitionToken);

	Action *testAction = Action::FindAction("TestAction");
	t_assertm(testAction != 0, "TestAction not found");
	if (testAction) {
		testAction->Register("ChangeMeTrue", "Action");
		testAction->Register("ChangeMeFalse", "Action");
		testAction->Register("ReturnFalse", "Action");
		testAction->Register("DontChangeButReturnTrue", "Action");
	}
	transitionToken = "ChangeMeTrue";
	t_assert(Action::ExecAction(transitionToken, ctx));
	assertEqual("Changed", transitionToken);

	transitionToken = "ChangeMeFalse";
	t_assert(!Action::ExecAction(transitionToken, ctx));
	assertEqual("Changed", transitionToken);

	transitionToken = "DontChangeButReturnTrue";
	t_assert(Action::ExecAction(transitionToken, ctx));
	assertEqual("DontChangeButReturnTrue", transitionToken);

	transitionToken = "ReturnFalse";
	t_assert(!Action::ExecAction(transitionToken, ctx));
	assertEqual("ReturnFalse", transitionToken);
}

void ActionCoreTest::EmptyConfigurationTransitionHandling()
{
	StartTrace(ActionCoreTest.EmptyConfigurationTransitionHandling);

	String transitionToken;
	Context ctx;
	Anything wrapper;

	transitionToken = "NoAction";
	t_assert(Action::ExecAction(transitionToken, ctx, wrapper));
	assertEqual("NoAction", transitionToken);

	Action *testAction = Action::FindAction("TestAction");
	t_assertm(testAction != 0, "TestAction not found");
	if (!testAction) {
		return;
	}
	testAction->Register("ChangeMeTrue", "Action");
	testAction->Register("ChangeMeFalse", "Action");
	testAction->Register("ReturnFalse", "Action");
	testAction->Register("DontChangeButReturnTrue", "Action");

	transitionToken = "ChangeMeTrue";
	t_assert(Action::ExecAction(transitionToken, ctx, wrapper));
	assertEqual("Changed", transitionToken);

	transitionToken = "ChangeMeFalse";
	t_assert(!Action::ExecAction(transitionToken, ctx, wrapper));
	assertEqual("Changed", transitionToken);

	transitionToken = "DontChangeButReturnTrue";
	t_assert(Action::ExecAction(transitionToken, ctx, wrapper));
	assertEqual("DontChangeButReturnTrue", transitionToken);

	transitionToken = "ReturnFalse";
	t_assert(!Action::ExecAction(transitionToken, ctx, wrapper));
	assertEqual("ReturnFalse", transitionToken);
}

void ActionCoreTest::ConfiguredTransitionHandling()
{
	StartTrace(ActionCoreTest.ConfiguredTransitionHandling);

	String transitionToken;
	Context ctx;
	Anything wrapper;

	transitionToken = "DoTest";
	Anything config;
	config["RetValue"] = true;
	wrapper["ConfiguredTestAction"] = config;
	t_assert(Action::ExecAction(transitionToken, ctx, wrapper));
	assertEqual("DoTest", transitionToken);

	// Reset
	config = Anything();

	transitionToken = "DoTest";
	config["RetValue"] = true;
	config["ActionToken"] = "Changed";
	wrapper["ConfiguredTestAction"] = config;
	t_assert(Action::ExecAction(transitionToken, ctx, wrapper));
	assertEqual("Changed", transitionToken);

	// Reset
	config = Anything();

	transitionToken = "DoTest";
	config["RetValue"] = false;
	config.Remove("ActionToken");
	wrapper["ConfiguredTestAction"] = config;
	t_assert(!Action::ExecAction(transitionToken, ctx, wrapper));
	assertEqual("DoTest", transitionToken);

	// Reset
	config = Anything();

	transitionToken = "DoTest";
	config["RetValue"] = false;
	config["ActionToken"] = "Changed";
	wrapper["ConfiguredTestAction"] = config;
	t_assert(!Action::ExecAction(transitionToken, ctx, wrapper));
	assertEqual("Changed", transitionToken);
}

void ActionCoreTest::ActionSequence()
{
	StartTrace(ActionCoreTest.ActionSequence);

	String transitionToken;
	Context ctx;

	transitionToken = "ChangeMeTrue";

	Anything config;
	config["TestAction"] = "";
	config["ConfiguredTestAction"]["RetValue"] = false;
	t_assert(!Action::ExecAction(transitionToken, ctx, config));
	assertEqual("Changed", transitionToken);
	// Check if both actions took place
	Anything tmpStore = ctx.GetTmpStore();
	assertEqual("ChangeMeTrue", tmpStore["TestAction"].AsString("x"));
	assertEqual("Changed", tmpStore["ConfiguredTestAction"].AsString("x"));

	// Reset
	tmpStore["TestAction"] = "";
	tmpStore["ConfiguredTestAction"] = "";
	config = Anything();

	transitionToken = "ChangeMeTrue";

	config["TestAction"] = "";
	config["ConfiguredTestAction"]["RetValue"] = true;
	config["ConfiguredTestAction"]["ActionToken"] = "GoHome";
	t_assert(Action::ExecAction(transitionToken, ctx, config));
	assertEqual("GoHome", transitionToken);
	// Check if both actions took place
	assertEqual("ChangeMeTrue", tmpStore["TestAction"].AsString("x"));
	assertEqual("Changed", tmpStore["ConfiguredTestAction"].AsString("x"));

	// Reset
	tmpStore["TestAction"] = "";
	tmpStore["ConfiguredTestAction"] = "";
	config = Anything();

	transitionToken = "ChangeMeTrue";

	config["SlotnameIsNoAction"]["TestAction"] = "";
	config[1]["ConfiguredTestAction"]["RetValue"] = true;
	config[1]["ConfiguredTestAction"]["ActionToken"] = "GoHome";
	config.Append("NoAction");
	t_assert(Action::ExecAction(transitionToken, ctx, config));
	assertEqual("NoAction", transitionToken);
	// Check if both actions took place
	assertEqual("ChangeMeTrue", tmpStore["TestAction"].AsString("x"));
	assertEqual("Changed", tmpStore["ConfiguredTestAction"].AsString("x"));
}

void ActionCoreTest::AbortedActionSequence()
{
	StartTrace(ActionCoreTest.ActionSequence);

	String transitionToken;
	Context ctx;

	transitionToken = "ChangeMeFalse";

	Anything config;
	config["TestAction"] = "";
	config["ConfiguredTestAction"]["RetValue"] = false;
	t_assert(!Action::ExecAction(transitionToken, ctx, config));
	assertEqual("Changed", transitionToken);
	// Check if only the first action took place because the first returned false
	Anything tmpStore = ctx.GetTmpStore();
	assertEqual("ChangeMeFalse", tmpStore["TestAction"].AsString("x"));
	t_assert(!tmpStore.IsDefined("ConfiguredTestAction"));
}

// builds up a suite of testcases, add a line for each testmethod
Test *ActionCoreTest::suite ()
{
	TestSuite *testSuite = new TestSuite;
	ADD_CASE(testSuite, ActionCoreTest, ConsistentTransitionHandling);
	ADD_CASE(testSuite, ActionCoreTest, EmptyConfigurationTransitionHandling);
	ADD_CASE(testSuite, ActionCoreTest, ConfiguredTransitionHandling);
	ADD_CASE(testSuite, ActionCoreTest, ActionSequence);
	ADD_CASE(testSuite, ActionCoreTest, AbortedActionSequence);
	return testSuite;
}
