/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

//--- standard modules used ----------------------------------------------------
#include "Anything.h"
#include "Threads.h"
#include "Dbg.h"

//--- test modules used --------------------------------------------------------
#include "TestSuite.h"

//--- interface include --------------------------------------------------------
#include "RequestBlockerTest.h"

//--- modules under test -----------------------------------------------------
#include "RequestBlocker.h"

#include "System.h"

class RBRunner: public Thread
{
public:
	RBRunner(RequestBlockerTest *env, long iterations, long checkEvery, bool compare, String name);
	virtual ~RBRunner() {};
	virtual void Run();
	virtual void StartRunning() {
		SetWorking();
	}

private:
	const long cRunSz;
	RequestBlockerTest *fTest;
	const bool fCompare;
	const long cCheckEvery;
};

RBRunner::RBRunner(RequestBlockerTest *env, long iterations, long checkEvery, bool compare , String name)
	: Thread(name)
	, cRunSz(iterations)
	, fTest(env)
	, fCompare(compare)
	, cCheckEvery(checkEvery)
{
	StartTrace(RBRunner.RBRunner);
}

void RBRunner::Run()
{
	StartTrace(RBRunner.Run);
	Trace(GetName() << " Id is: " << (long)GetId());
	bool checkIt;
	CheckRunningState(eWorking);
	for (long i = 1; i <= cRunSz; i++) { // avoid 0
		checkIt = (!(i % cCheckEvery));
		fTest->QueryRB(fCompare, checkIt, GetName());
	}
}

//---- RequestBlockerTest ----------------------------------------------------------------
RequestBlockerTest::RequestBlockerTest(TString name) : TestCase(name)
{
	StartTrace(RequestBlockerTest.Ctor);
}

RequestBlockerTest::~RequestBlockerTest()
{
	StartTrace(RequestBlockerTest.Dtor);
}

// setup for this TestCase
void RequestBlockerTest::setUp ()
{
	StartTrace(RequestBlockerTest.setUp);
} // setUp

void RequestBlockerTest::tearDown ()
{
	StartTrace(RequestBlockerTest.tearDown);
} // tearDown

void RequestBlockerTest::QueryRB(bool compare, bool checkIt, TString tname)
{
	StartTrace(RequestBlockerTest.QueryRB);
	// Exercise RB under any conditions but only verify result
	// if wanted
	if (checkIt) {
		t_assertm(RequestBlocker::RB()->IsBlocked() == compare, tname << (compare ? " RB should have been blocked" : " RB should have NOT been blocked" ));
	} else {
		RequestBlocker::RB()->IsBlocked();
	}
	System::MicroSleep(1); // slow down runner should ensure no false positives for 5,6
}

void RequestBlockerTest::RBTest()
{
	StartTrace(RequestBlockerTest.RBTest);
	bool isblocked = RequestBlocker::RB()->IsBlocked();
	// Set initial state of RB, must be false
	RequestBlocker::RB()->UnBlock();
	t_assert(!RequestBlocker::RB()->IsBlocked());
	Trace("MyId is: " << Thread::MyId());
	RBRunner t1(this, 4, 1, false, "Runner1"), t2(this, 4, 1, false, "Runner2");
	t1.Start();
	t1.CheckState(Thread::eRunning);
	t2.Start();
	t2.CheckState(Thread::eRunning);
	t_assertm(t1.CheckRunningState(Thread::eReady), "State should be eReady");
	t_assertm(t2.CheckRunningState(Thread::eReady), "State should be eReady");
	// start em up
	t1.StartRunning();
	t2.StartRunning();
	t1.CheckState(Thread::eTerminated);
	t2.CheckState(Thread::eTerminated);

	// Test changed state of RB
	RequestBlocker::RB()->Block();
	RBRunner t3(this, 4, 1, true, "Runner3"), t4(this, 4, 1, true, "Runner4");
	t3.Start();
	t3.CheckState(Thread::eRunning);
	t4.Start();
	t4.CheckState(Thread::eRunning);
	t_assertm(t3.CheckRunningState(Thread::eReady), "State should be eReady");
	t_assertm(t4.CheckRunningState(Thread::eReady), "State should be eReady");
	// start em up
	t3.StartRunning();
	t4.StartRunning();
	t3.CheckState(Thread::eTerminated);
	t4.CheckState(Thread::eTerminated);

	t_assert(RequestBlocker::RB()->IsBlocked() == true);
	// Let 2 threads run and query the RW lock 2 * 100 times. In the meantime, set the
	// RW-Lock to unblocking and let the last iteration of each of the 2 threads
	// verify the changed state.
	RBRunner t5(this, 100, 100, false, "Runner5"), t6(this, 100, 100, false, "Runner6");
	t5.Start();
	t5.CheckState(Thread::eRunning);
	t6.Start();
	t6.CheckState(Thread::eRunning);
	t_assertm(t5.CheckRunningState(Thread::eReady), "State should be eReady");
	t_assertm(t6.CheckRunningState(Thread::eReady), "State should be eReady");
	// start em up
	t5.StartRunning();
	t6.StartRunning();
	RequestBlocker::RB()->UnBlock();
	t5.CheckState(Thread::eTerminated);
	t6.CheckState(Thread::eTerminated);
	t_assert(!RequestBlocker::RB()->IsBlocked());
	if (isblocked) {
		RequestBlocker::RB()->Block();
	}
	t_assert(isblocked == RequestBlocker::RB()->IsBlocked());
}

// builds up a suite of testcases, add a line for each testmethod
Test *RequestBlockerTest::suite ()
{
	StartTrace(RequestBlockerTest.suite);
	TestSuite *testSuite = new TestSuite;

	testSuite->addTest (NEW_CASE(RequestBlockerTest, RBTest));

	return testSuite;

} // suite
