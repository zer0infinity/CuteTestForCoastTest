/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

//--- interface include --------------------------------------------------------
#include "ThreadsTest.h"

//--- module under test --------------------------------------------------------
#include "TestThread.h"

//--- test modules used --------------------------------------------------------
#include "TestSuite.h"

//--- standard modules used ----------------------------------------------------
#include "StringStream.h"
#include "Dbg.h"

//--- c-library modules used ---------------------------------------------------

#define MILISEC 1000000 /* 1 million nanoseconds */
#define MILISECGRANULARITY 50  //100  /* check and measure for 0.1 or 0.05 seconds */
// SOP: 10 miliseconds seem to be too fine grained on my linux box.
class ExecCountThread : public Thread
{
public:
	ExecCountThread();

	long fCount;
protected:
	virtual void Run();
};

ExecCountThread::ExecCountThread()
	: Thread("ExecCountThread")
{
	fCount = 0;
}

void ExecCountThread::Run()
{
	fCount++;
}

//---- ThreadsTest ----------------------------------------------------------------
ThreadsTest::ThreadsTest(TString tname) : TestCaseType(tname), fMutex(tname)
{
}

ThreadsTest::~ThreadsTest()
{
}

Test *ThreadsTest::suite ()
{
	TestSuite *testSuite = new TestSuite;
	ADD_CASE(testSuite, ThreadsTest, SimpleSemaphoreTest);
	ADD_CASE(testSuite, ThreadsTest, MultiSemaphoreTest);
	ADD_CASE(testSuite, ThreadsTest, SimpleMutexTest);
	ADD_CASE(testSuite, ThreadsTest, TimedRunTest);
	ADD_CASE(testSuite, ThreadsTest, RecursiveMutexTest);
	ADD_CASE(testSuite, ThreadsTest, MutexLockIterationTest);
	ADD_CASE(testSuite, ThreadsTest, SimpleConditionsTest);
	ADD_CASE(testSuite, ThreadsTest, ThreadHookArgsTest);
	ADD_CASE(testSuite, ThreadsTest, ThreadStateTransitionTest);
	ADD_CASE(testSuite, ThreadsTest, ThreadRunningStateTransitionTest);
	ADD_CASE(testSuite, ThreadsTest, ThreadObjectReuseTest);
	return testSuite;
}

void ThreadsTest::ThreadRunningStateTransitionTest()
{
	StartTrace(ThreadsTest.ThreadRunningStateTransitionTest);
	// "Normal case" goes from Created to Started,Running, toggles between Ready and Working and gets Terminated
	TerminateMeTestThread t;
	t_assertm(!t.IsReady(), "Is not Ready yet");
	t_assertm(!t.IsWorking(), "It is not working");
	t_assertm(!t.SetReady(), "Can not set Ready until running");
	t_assertm(!t.CheckRunningState(Thread::eReady, 0, 20 * MILISEC), "State can not be eReady until running");
	t_assertm(!t.CheckRunningState(Thread::eReady, 0, 20 * MILISEC), "State can not be eReady until running");
	t_assertm(!t.CheckRunningState(Thread::eWorking, 0, 20 * MILISEC), "State can not be eWorking until running");
	t_assertm(!t.CheckRunningState(Thread::eWorking, 0, 20 * MILISEC), "State can not be eWorking until running");

	t.Start();
	t_assertm(t.CheckState(Thread::eRunning), "State should be eRunning");
	t_assertm(t.IsRunning(), "Thread is running");
	t_assertm(t.CheckRunningState(Thread::eReady), "State should be eReady");
	t_assertm(t.IsReady(), "is ready by default");
	t_assertm(!t.IsWorking(), "It is not working (might fail sometimes because we are too fast)");
	t_assertm(!t.CheckRunningState(Thread::eWorking, 0, 20 * MILISEC), "State can not be eWorking");
	t_assertm(!t.CheckRunningState(Thread::eWorking, 0, 20 * MILISEC), "State can not be eWorking");
	t_assertm(t.SetWorking(), "Can be set to working");
	t_assertm(t.CheckRunningState(Thread::eWorking, 10), "State should be eWorking");
	t_assertm(!t.CheckRunningState(Thread::eReady, 0, 20 * MILISEC), "State can not be eReady");
	t_assertm(!t.CheckRunningState(Thread::eReady, 0, 20 * MILISEC), "State can not be eReady");
	t_assertm(t.IsWorking(), "It is working");
	t_assertm(t.IsRunning(), "Thread is still running");
	t_assertm(!t.IsReady(), "It is not ready when working");
	t_assertm(!t.SetWorking(), "Cannot be set to working again before set to ready");

	t_assertm(t.SetReady(), "Can be set back to ready");
	t_assertm(t.CheckRunningState(Thread::eReady, 10), "State should be eReady");

	t_assert(t.Terminate(10));
	t_assertm(!t.IsReady(), "Is not Ready anymore");
	t_assertm(!t.IsWorking(), "It is not working");
	t_assertm(!t.SetReady(), "Can not revert state");
	t_assertm(!t.SetWorking(), "Can not revert state");
}

void ThreadsTest::ThreadObjectReuseTest()
{
	StartTrace(ThreadsTest.ThreadObjectReuseTest);
	// the same thread object must be reusable
	// (if properly terminated before reuse)
	int i = 0;
	for (i = 0; i < 100; i++) {
		ExecCountThread t;
		if (t_assert(t.Start())) {
			// start once
			if ( t_assert(t.CheckState(Thread::eTerminated, 2)) && assertEqual(1L, t.fCount) ) {
				Trace("count: " << t.fCount);
				if (t_assert(t.Start())) {
					// start twice
					if ( t_assert(t.CheckState(Thread::eTerminated, 2)) && assertEqual(2L, t.fCount) ) {
						Trace("count: " << t.fCount);
						if (t_assert(t.Start())) {
							// start three times
							if (t_assert(t.CheckState(Thread::eTerminated, 2))) {
								Trace("count: " << t.fCount);
								assertEqual(3L, t.fCount);			// did it properly run three times
							} else {
								assertEqual(3L, t.fCount);
								cerr << "/";
								break;
							}
						} else {
							cerr << "ç";
						}
					} else {
						assertEqual(2L, t.fCount);
						cerr << "#";
						break;
					}
				} else {
					cerr << "@";
				}
			} else {
				assertEqual(1L, t.fCount);
				cerr << "%";
				break;
			}
		} else {
			cerr << "-";
		}
	}
}

class HookArgsTestThread: public Thread
{
public:
	HookArgsTestThread()
		: Thread("HookArgsTestThread") {};
	virtual ~HookArgsTestThread() {};
	virtual void Run() {
		CheckState(Thread::eRunning, 2);
		CheckState(Thread::eTerminationRequested, 2);
	};
	bool DoStartRequestedHook(ROAnything args) {
		fStartedArgs = args.DeepClone();
		return true;
	};
	void DoTerminationRequestHook(ROAnything args) {
		fTerminationArgs = args.DeepClone();
	};

	Anything fStartedArgs, fTerminationArgs;
};

void ThreadsTest::ThreadHookArgsTest()
{
	StartTrace(ThreadsTest.ThreadHookArgsTest);
	HookArgsTestThread t;
	Anything anyStartArgs, anyTerminationArgs;
	anyStartArgs["Start"] = "Go!";
	t_assertm( t.Start(Storage::Global(), anyStartArgs), "expected thread start to succeed");
	anyTerminationArgs["Stop"] = "RIP";
	t_assertm( t.Terminate(1, anyTerminationArgs), "expected termination request to succeed");
	assertAnyEqual(anyStartArgs, t.fStartedArgs);
	assertAnyEqual(anyTerminationArgs, t.fTerminationArgs);
}

void ThreadsTest::ThreadStateTransitionTest()
{
	StartTrace(ThreadsTest.ThreadStateTransitionTest);

	// "Normal case" goes from Created to Started,Running, toggles between Ready and Working and gets Terminated
	TerminateMeTestThread t;

	t_assertm(t.CheckState(Thread::eCreated), "State should be eCreated");
	assertEqual(Thread::eCreated, t.GetState());
	t_assert(t.Start());
	t_assertm(t.CheckState(Thread::eStarted, 10), "State should be eStarted");
	t_assertm(t.CheckState(Thread::eRunning, 10), "State should be eRunning");
	t_assertm(t.IsRunning(), "Thread should be Running");
	assertEqual(Thread::eRunning, t.GetState()); //Semantically equal to IsRunning()

	t_assert(t.Terminate(100));
	t_assertm(!t.IsRunning(), "Thread is not running anymore");
	t_assertm(t.CheckState(Thread::eTerminated, 10), "State should be eTerminated");

	//-- Can not be restarted ----------------------------------
	t_assertm(t.Start(), "should be restartable, since thread cannot be restarted");
	t_assertm(t.CheckState(Thread::eStarted, 10), "State should be eStarted");
	t_assertm(t.CheckState(Thread::eRunning, 10), "State should be eRunning");
	t_assert(t.Terminate(10));
	assertEqual(Thread::eTerminated, t.GetState()); // Can not start again
	t_assertm(!t.IsRunning(), "Thread can not be started again");
	t_assertm(t.CheckState(Thread::eTerminated, 10), "State should be eTerminated");

	//-- Terminate before Start ---------------------------------
	TerminateMeTestThread t2;
	t_assertm(t2.CheckState(Thread::eCreated, 10), "State should be eCreated");
	t_assert(t2.Terminate(10));
	assertEqual(Thread::eTerminated, t2.GetState());
	t_assertm(t2.Start(), "should be restartable, since already terminated");
	t_assertm(t2.CheckState(Thread::eStarted, 10), "State should be eStarted");
	t_assertm(t2.CheckState(Thread::eRunning, 10), "State should be eRunning");
	t_assert(t2.Terminate(10));
	assertEqual(Thread::eTerminated, t2.GetState()); // Can not start again
	t_assertm(!t2.IsRunning(), "Thread can not be started again");
	t_assertm(t2.CheckState(Thread::eTerminated, 10), "State should be eTerminated");

	//-- Terminate because precondition of Start fails ---
	TerminateMeTestThread t3(false);
	t_assertm(!t3.Start(), "start should fail");
	assertEqual(Thread::eTerminated, t3.GetState()); // Can not start again
	t_assertm(t3.CheckState(Thread::eStarted, 10), "State should be eStarted");
	t_assertm(t3.CheckState(Thread::eRunning, 10), "State should be eRunning");
	t_assertm(!t3.IsRunning(), "Thread can not be started again");
	t_assertm(t3.CheckState(Thread::eTerminated, 10), "State should be eTerminated");
}

class ContentionRunner: public Thread
{
public:
	ContentionRunner(ThreadsTest *env, long iterations, String name);
	virtual ~ContentionRunner() {};
	virtual void Run();
	virtual void StartRunning() {
		SetWorking();
	}

private:
	const long cRunSz;
	ThreadsTest *fTest;
};

class TimedRunner: public Thread
{
public:
	TimedRunner(ThreadsTest *env, long lTime, String name);
	virtual ~TimedRunner() {};
	virtual void Run();
	virtual bool StartRunning() {
		return SetWorking();
	}

private:
	const long cTimeSz;
	ThreadsTest *fTest;
};

//---- RecursiveSynchTestThread ----------------------------------------------------------
//:utility class - simple Thread to play around - Runs until Terminate is called
class RecursiveSynchTestThread : public TestThread
{
public:
	RecursiveSynchTestThread(Mutex &m, Condition &c);
	virtual ~RecursiveSynchTestThread() {}
	virtual void Run();
protected:
	Mutex &fMutex;
	Condition &fCond;

};

//--- RecursiveSynchTestThread -------------------------------------------
RecursiveSynchTestThread::RecursiveSynchTestThread(Mutex &m, Condition &c) : TestThread(), fMutex(m), fCond(c)
{ }

void RecursiveSynchTestThread::Run()
{
	StartTrace(RecursiveSynchTestThread.Run);
	CheckRunningState(eWorking);
	{
		Trace("I am working");
		MutexEntry me(fMutex);
		me.Use();
		{
			Trace("mutex locked once");
			MutexEntry me2(fMutex);
			me2.Use();
			Trace("mutex locked twice");
			fCond.Signal();
			Trace("condition signalled once");
		}
		fCond.Signal();
		Trace("condition signalled twice");
	}
	CheckState(eTerminationRequested);
}

ContentionRunner::ContentionRunner(ThreadsTest *env, long iterations, String name)
	: Thread(name)
	, cRunSz(iterations)
	, fTest(env)
{
	StartTrace(ContentionRunner.ContentionRunner);
}

void ContentionRunner::Run()
{
	StartTrace(ContentionRunner.Run);
	Trace(GetName() << " Id is: " << (long)GetId());
	CheckRunningState(eWorking);
	for (long i = 0; i < cRunSz; i++) {
		fTest->SimpleLockedAccess(i);
	}
}

TimedRunner::TimedRunner(ThreadsTest *env, long lTime, String name)
	: Thread(name)
	, cTimeSz(lTime)
	, fTest(env)
{
	StartTrace(TimedRunner.TimedRunner);
}

void TimedRunner::Run()
{
	StartTrace(TimedRunner.Run);
	Trace(GetName() << " Id is: " << (long)GetId());
	CheckRunningState(eWorking);
	fTest->CheckTime(cTimeSz);
}

void ThreadsTest::CheckTime(long lTime)
{
	StartTrace(ThreadsTest.RegisterTime);
	assertEqualm(fDiffTimer.Diff() / MILISECGRANULARITY , lTime, "TimeMeasured in Thread should be in same .1 second");
}

void ThreadsTest::SimpleLockedAccess(long i)
{
	MutexEntry me(fMutex);
	me.Use();

	if ( i > 2 ) {
		fLockedAny["index"] = i * 2 - i;
	} else {
		fLockedAny["index"] = i * 2;
	}
}

void ThreadsTest::SimpleMutexTest()
{
	StartTrace(ThreadsTest.SimpleMutexTest);
	fLockedAny = Anything();
	Trace("MyId is: " << Thread::MyId());
	ContentionRunner t1(this, 1, "Runner1"), t2(this, 1, "Runner2");
	t1.Start();
	t1.CheckState(Thread::eRunning);
	t2.Start();
	t2.CheckState(Thread::eRunning);
	t_assertm(t1.CheckRunningState(Thread::eReady), "State should be eReady");
	t_assertm(t2.CheckRunningState(Thread::eReady), "State should be eReady");
	// block running of threads first
	fMutex.Lock();
	// start em up
	t1.StartRunning();
	t2.StartRunning();

	t_assertm(t1.CheckRunningState(Thread::eWorking, 10), "State should be eWorking");
	t_assertm(t2.CheckRunningState(Thread::eWorking, 10), "State should be eWorking");

	// let them do some work now
	fMutex.Unlock();

	t1.CheckState(Thread::eTerminated);
	t2.CheckState(Thread::eTerminated);
}

void ThreadsTest::TimedRunTest()
{
	StartTrace(ThreadsTest.SimpleMutexTest);
	long testTime;
	// We do it 5 times
	for (testTime = 1; testTime < 6; testTime++) {
		long miliTestTime = testTime * MILISECGRANULARITY;
		long nanoTestTime = miliTestTime * MILISEC;
		DiffTimer thisTimer;
		Trace("MyId is: " << Thread::MyId());

		TimedRunner t1(this, testTime, "TimedRunner1");
		t1.Start();
		t_assertm(t1.CheckState(Thread::eRunning), "wait for thread to be started failed");
		thisTimer.Start();				// start time mesurement to enusure CheckRunning
		// observes the given wait time
		fDiffTimer.Start();				// start time measurement to ensure thred  really
		// works after StartRunning()
		// the following method is the core to be tested here:
		t_assertm(!(t1.CheckRunningState(Thread::eWorking, 0, nanoTestTime)), "State should be eReady");
		long waited = thisTimer.Diff();	// this way of mesurement will never give the same
		long centiSecWaited = waited / MILISECGRANULARITY;	// millisecond results, but it should be within the
		// same tenth of second. (Timer is stopped after CheckRunningState
		// returns)

		t_assertm(waited >= miliTestTime, 	 "Exact Time passed should be equal or greater time given to wait");
		assertEqualm(centiSecWaited , testTime,   "Time passed rounded to .1 seconds should equal time given to wait");
		// Start them up
		t_assert(t1.StartRunning());				// we will get out of this test, won't we?
		t1.CheckState(Thread::eTerminated);
	}
}

class SemaTestThread : public Thread
{
public:
	SemaTestThread(Semaphore &i_raSema, bool releaseafter = true);

protected:
	virtual void Run();
private:
	Semaphore &fSema;
	bool fReleaseAfter;
};

SemaTestThread::SemaTestThread(Semaphore &i_raSema, bool releaseafter)
	: Thread("SemaTestThread")
	, fSema(i_raSema)
	, fReleaseAfter(releaseafter)
{}

void SemaTestThread::Run()
{
	fSema.Acquire();
	if (fReleaseAfter) {
		fSema.Release();
	}
}

void ThreadsTest::SimpleSemaphoreTest()
{
	StartTrace(ThreadsTest.SimpleSemaphoreTest);
	// single resource sema test

	Semaphore sema(1);
	CheckSemaphoreCount(sema, 1);

	sema.Acquire();
	// since it is a single resource sema acquiring it again must fail
	t_assertm(sema.TryAcquire() == false, "Should not acquire sema again!");
	CheckSemaphoreCount(sema, 0);

	SemaTestThread tt1(sema);
	tt1.Start();
	t_assertm(tt1.CheckState(Thread::eRunning, 10), "State should be eRunning");
	CheckSemaphoreCount(sema, 0);

	sema.Release();
	t_assertm(tt1.CheckState(Thread::eTerminated, 10), "State should be eTerminated");
	CheckSemaphoreCount(sema, 1);

	// acquiring it again must succeed now
	t_assertm(sema.TryAcquire() == true, "Thread had problems with semaphore!");
	sema.Release();
}

void ThreadsTest::MultiSemaphoreTest()
{
	StartTrace(ThreadsTest.MultiSemaphoreTest);
	// multiple resources sema test
	Semaphore sema(5);
	CheckSemaphoreCount(sema, 5);

	// acquire 4 times
	sema.Acquire();
	sema.Acquire();
	sema.Acquire();
	sema.Acquire();
	CheckSemaphoreCount(sema, 1);

	// acquiring it again must succeed now
	if ( t_assertm(sema.TryAcquire() == true, "Thread had problems with semaphore!") ) {
		sema.Release();
	}
	// 4 acqd
	CheckSemaphoreCount(sema, 1);

	SemaTestThread tt1(sema, false), tt2(sema, false), tt3(sema, false), tt4(sema, false), tt5(sema, false), tt6(sema, false);
	// this thread should be able to acquire sema and then terminate

	tt1.Start();
	// this thread should have acquired the fifth sema so its state should be Terminated
	t_assertm(tt1.CheckState(Thread::eTerminated, 1), "State should be eTerminated");
	CheckSemaphoreCount(sema, 0);
	t_assertm(sema.TryAcquire() == false, "should have no more resources");
	// 5 acqd
	// now start 5 threads and check if all block first and then all terminate after releasing the semas
	tt6.Start();
	tt2.Start();
	tt3.Start();
	tt4.Start();
	tt5.Start();
	t_assertm(tt6.CheckState(Thread::eRunning, 1), "State should be eRunning");
	t_assertm(tt2.CheckState(Thread::eRunning, 1), "State should be eRunning");
	t_assertm(tt3.CheckState(Thread::eRunning, 1), "State should be eRunning");
	t_assertm(tt4.CheckState(Thread::eRunning, 1), "State should be eRunning");
	t_assertm(tt5.CheckState(Thread::eRunning, 1), "State should be eRunning");
	// let threads run and terminate one by one
	sema.Release();
	sema.Release();
	sema.Release();
	sema.Release();
	sema.Release();
	CheckSemaphoreCount(sema, 0);

	// now all threads should have terminated
	t_assertm(tt6.CheckState(Thread::eTerminated, 1), "State should be eTerminated");
	t_assertm(tt2.CheckState(Thread::eTerminated, 1), "State should be eTerminated");
	t_assertm(tt3.CheckState(Thread::eTerminated, 1), "State should be eTerminated");
	t_assertm(tt4.CheckState(Thread::eTerminated, 1), "State should be eTerminated");
	t_assertm(tt5.CheckState(Thread::eTerminated, 1), "State should be eTerminated");
	CheckSemaphoreCount(sema, 0);
	t_assertm(sema.TryAcquire() == false, "should still have no more resources");
	sema.Release();
	sema.Release();
	sema.Release();
	sema.Release();
	sema.Release();
	CheckSemaphoreCount(sema, 5);
	t_assertm(sema.TryAcquire() == true, "should be able to reaquire semaphore");
	CheckSemaphoreCount(sema, 4);
	sema.Release();
	CheckSemaphoreCount(sema, 5);
}

void ThreadsTest::SimpleConditionsTest()
{
	StartTrace(ThreadsTest.SimpleConditionsTest);
}

void ThreadsTest::LockedIteration(long iterations)
{
	StartTrace(ThreadsTest.LockedIteration);
	DiffTimer dt;
	fLockedAny = Anything();
	Trace("MyId is: " << Thread::MyId());
	ContentionRunner t1(this, iterations, "Runner1"), t2(this, iterations, "Runner2");
	t1.Start();
	t2.Start();
	t1.CheckState(Thread::eRunning);
	t2.CheckState(Thread::eRunning);
	t1.StartRunning();
	t2.StartRunning();

	t1.CheckState(Thread::eTerminated);
	t2.CheckState(Thread::eTerminated);
}

void ThreadsTest::MutexLockIterationTest()
{
	StartTrace(ThreadsTest.MutexLockIterationTest);
	LockedIteration(100);
	LockedIteration(1000);
//	LockedIteration(10000);
//	LockedIteration(100000);
}

void ThreadsTest::SimpleRecursiveTest()
{
	StartTrace(ThreadsTest.SimpleRecursiveTest);
	Mutex m("RecursiveMutexTest");
	MutexEntry me(m);
	me.Use();
	t_assert(m.GetCount() == 1);
	{
		MutexEntry me2(m);
		me2.Use();
		t_assert(m.GetCount() == 2);
		{
			MutexEntry me3(m);
			me3.Use();
			t_assert(m.GetCount() == 3);
			{
				MutexEntry me4(m);
				me4.Use();
				t_assert(m.GetCount() == 4);
			}
			t_assert(m.GetCount() == 3);
		}
		t_assert(m.GetCount() == 2);
	}
	t_assert(m.GetCount() == 1);
}

void ThreadsTest::SimpleRecursiveTryLockTest()
{
	StartTrace(ThreadsTest.SimpleRecursiveTryLockTest);
	Mutex m("RecursiveMutexTest");
	t_assert(m.TryLock());
	t_assert(m.GetCount() == 1);
	{
		t_assert(m.TryLock());
		t_assert(m.GetCount() == 2);
		{
			t_assert(m.TryLock());
			t_assert(m.GetCount() == 3);
			{
				t_assert(m.TryLock());
				t_assert(m.GetCount() == 4);
				m.Unlock();
			}
			t_assert(m.GetCount() == 3);
			m.Unlock();
		}
		t_assert(m.GetCount() == 2);
		m.Unlock();
	}
	t_assert(m.GetCount() == 1);
	m.Unlock();
}

void ThreadsTest::TwoThreadRecursiveTest()
{
	StartTrace(ThreadsTest.TwoThreadRecursiveTest);
	for (int i = 0; i < 20; i++) {
		Mutex m("RecursiveMutexTest");
		Condition c;
		RecursiveSynchTestThread rstt(m, c);
		MutexEntry me(m);
		me.Use();
		t_assert(m.GetCount() == 1);
		rstt.Start();
		{
			MutexEntry me2(m);
			me2.Use();
			t_assert(m.GetCount() == 2);
			rstt.CheckState(Thread::eRunning);
			rstt.SetWorking();
			{
				MutexEntry me3(m);
				me3.Use();
				t_assert(m.GetCount() == 3);
				{
					MutexEntry me4(m);
					me4.Use();
					c.Wait(m);
					c.Signal();
					t_assert(m.GetCount() == 4);
				}
				t_assert(m.GetCount() == 3);
			}
			t_assert(m.GetCount() == 2);
		}
		t_assert(m.GetCount() == 1);
		rstt.Terminate();
	}
}

void ThreadsTest::TwoThreadRecursiveTryLockTest()
{
	StartTrace(ThreadsTest.TwoThreadRecursiveTryLockTest);
	Mutex m("RecursiveMutexTest");
	Condition c;
	RecursiveSynchTestThread rstt(m, c);
	t_assert(m.TryLock());
	t_assert(m.GetCount() == 1);
	rstt.Start();
	{
		t_assert(m.TryLock());
		t_assert(m.GetCount() == 2);
		rstt.CheckState(Thread::eRunning);
		rstt.SetWorking();
		{
			t_assert(m.TryLock());
			t_assert(m.GetCount() == 3);
			{
				t_assert(m.TryLock());
				c.Wait(m);
				c.Signal();
				t_assert(m.GetCount() == 4);
				m.Unlock();
			}
			t_assert(m.GetCount() == 3);
			m.Unlock();
		}
		t_assert(m.GetCount() == 2);
		m.Unlock();
	}
	t_assert(m.GetCount() == 1);
	m.Unlock();
	rstt.Terminate();
}

void ThreadsTest::RecursiveMutexTest()
{
	SimpleRecursiveTest();
	SimpleRecursiveTryLockTest();
	TwoThreadRecursiveTest();
	TwoThreadRecursiveTryLockTest();
}

void ThreadsTest::CheckSemaphoreCount(Semaphore &sema, int expected)
{
	StartTrace(ThreadsTest.CheckSemaphoreCount);
	int count;
	int ret = sema.GetCount(count);
	assertEqual(0, ret);
	assertEqual(expected, count);
}
