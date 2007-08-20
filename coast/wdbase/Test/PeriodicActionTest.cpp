/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

//--- standard modules used ----------------------------------------------------
#include "Anything.h"
#include "DiffTimer.h"
#include "Threads.h"
#include "Context.h"
#include "Action.h"
#include "Dbg.h"

//--- test modules used --------------------------------------------------------
#include "TestSuite.h"

//--- module under test --------------------------------------------------------
#include "PeriodicAction.h"

//--- interface include --------------------------------------------------------
#include "PeriodicActionTest.h"

Condition PeriodicActionTest::fgCalledCond;
Mutex PeriodicActionTest::fgCalledMutex("PeriodicActionTest");

//---- PeriodicAction ----------------------------------------------------------
class PeriodicTestAction : public Action
{
public:
	//--- constructors
	PeriodicTestAction(const char *name);
	~PeriodicTestAction();

	//:cleans the session list from timeouted sessions
	virtual bool DoExecAction(String &transitionToken, Context &ctx, const ROAnything &config);
};

//---- PeriodicActionTest ----------------------------------------------------------------
long PeriodicActionTest::fgCalled = 0;

PeriodicActionTest::PeriodicActionTest(TString tname) : TestCaseType(tname)
{
	StartTrace(PeriodicActionTest.Ctor);
}

PeriodicActionTest::~PeriodicActionTest()
{
	StartTrace(PeriodicActionTest.Dtor);
}

void PeriodicActionTest::PeriodicTest()
{
	StartTrace(PeriodicActionTest.PeriodicTest);

	PeriodicAction pa("PeriodicTestAction", 1L);
	assertEqual("PeriodicTestAction", pa.fAction);
	assertEqual(1L, pa.fWaitTime);
	pa.Start();
	t_assertm(pa.CheckState(Thread::eRunning, 5L), "expected thread to be running");
	{
		LockUnlockEntry me(fgCalledMutex);
		DiffTimer differ;
		long waitTime = 0;
		while ((fgCalled < 2) && (waitTime < (7 * 1000L))) {
			fgCalledCond.TimedWait(fgCalledMutex, 1);
			waitTime = differ.Diff();
			Trace("testPeriodicAction > fgCalled: " << (long)fgCalled << " @ " << waitTime);
		}
	}
	t_assertm(pa.Terminate(7L), "expected thread to be terminated");

	LockUnlockEntry me(fgCalledMutex);
	t_assertm(fgCalled > 1, "expected periodic action to be called several times");
}

void PeriodicActionTest::ActionCalled(Context &ctx)
{
	StartTrace(PeriodicActionTest.ActionCalled);
	LockUnlockEntry me(fgCalledMutex);
	fgCalled++;
	fgCalledCond.Signal();
	Trace("ActionCalled > fgCalled: " << (long)fgCalled << "@" << (long)time(0));
}

// builds up a suite of testcases, add a line for each testmethod
Test *PeriodicActionTest::suite ()
{
	StartTrace(PeriodicActionTest.suite);
	TestSuite *testSuite = new TestSuite;
	ADD_CASE(testSuite, PeriodicActionTest, PeriodicTest);
	return testSuite;
}

RegisterAction(PeriodicTestAction);

PeriodicTestAction::PeriodicTestAction(const char *name) : Action(name) { }

PeriodicTestAction::~PeriodicTestAction() { }

bool PeriodicTestAction::DoExecAction(String &transitionToken, Context &ctx, const ROAnything &config)
{
	StartTrace(PeriodicTestAction.DoExecAction);
	PeriodicActionTest::ActionCalled(ctx);
	return true;
}
