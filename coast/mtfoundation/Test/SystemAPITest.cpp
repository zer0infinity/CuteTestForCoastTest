/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#include "TestSuite.h"
#include "SystemAPI.h"
#include "SystemAPITest.h"
#include "SystemLog.h"
#include "SystemBase.h"
#include "Anything.h"
#include "Tracer.h"
#if !defined(WIN32)
#include <errno.h>
#include <sys/time.h>
#endif

// builds up a suite of testcases, add a line for each testmethod
Test *SystemAPITest::suite ()
{
	StartTrace(SystemAPITest.suite);
	TestSuite *testSuite = new TestSuite;
	ADD_CASE(testSuite, SystemAPITest, MUTEXTest);
	ADD_CASE(testSuite, SystemAPITest, SEMATest);
	ADD_CASE(testSuite, SystemAPITest, CONDITIONTimedWaitTimeoutTest);
	ADD_CASE(testSuite, SystemAPITest, CONDITIONSignalSingleTest);
	ADD_CASE(testSuite, SystemAPITest, CONDITIONSignalManyTest);
	ADD_CASE(testSuite, SystemAPITest, CONDITIONBroadcastSingleTest);
	ADD_CASE(testSuite, SystemAPITest, CONDITIONBroadcastManyTest);
	return testSuite;
}

class SimpleTestThread
{
public:
	SimpleTestThread(bool bDaemon = false, bool detached = true, bool suspended = false, bool bound = false);
    bool Start(THREADWRAPPERFUNCPROTO(wrapper), void *ThreadParam);

	THREAD fThreadId;
	bool fDaemon, fDetached, fSuspended, fBound;
};

SimpleTestThread::SimpleTestThread(bool bDaemon, bool detached, bool suspended, bool bound)
	: fDaemon(bDaemon)
	, fDetached(detached)
	, fSuspended(suspended)
	, fBound(bound)
{
}

bool SimpleTestThread::Start(THREADWRAPPERFUNCPROTO(wrapper), void *ThreadParam)
{
	StartTrace(SimpleTestThread.Start);
	bool b[4];
	b[0] = fBound;
	b[1] = fDetached;
	b[2] = fDaemon;
	b[3] = fSuspended;
	bool ret = STARTTHREAD(ThreadParam, b, &fThreadId, wrapper);
	return ret;
}

struct mutexthreadparam {
	mutexthreadparam(MUTEX &i_mtx, Anything &i_result) : mtx(i_mtx), result(i_result) {}
	MUTEX	&mtx;
	Anything &result;
};

static bool AssertMessageIntoAny(Anything &resultany, bool condition, const char *conditionasstring, const char *message, long line, const char *file)
{
	Anything any;
	\
	any["val"] = (bool)condition;
	any["cond"] = conditionasstring;
	any["line"] = line;
	any["file"] = file;
	any["msg"] = message;
	\
	resultany.Append(any);
	\
	return condition;
}

#define t_assertm_intoany(condition, message, resultany) AssertMessageIntoAny(resultany,condition,#condition,message,__LINE__,__FILE__)

void TestSleep( int sleepTimeInMiliseconds )
{
	coast::system::MicroSleep(1000L * sleepTimeInMiliseconds);
}

void SystemAPITest::PrintResult(Anything result )
{
	// the following sleep is to decouple late writings from the threadfunc into the result-anything
	TestSleep(10);
	// put the messages collected within thread
	for (int i = 0; i < result.GetSize(); i++) {
		assertImplementation(	result[i]["val"].AsBool(false),
								TString(result[i]["cond"].AsCharPtr()),
								result[i]["line"].AsLong(),
								TString(result[i]["file"].AsCharPtr()),
								TString(result[i]["msg"].AsCharPtr()));
	}
}

THREADWRAPPERFUNCDECL(MUTEXTestThread, thrdparam)
{
	StartTrace(SimpleTestThread.Wrapperfunc);
	mutexthreadparam &param = *(mutexthreadparam *)thrdparam;
	int iRet = 0;
	// should fail the first time because we are in another thread right here
	if (t_assertm_intoany(!TRYLOCK(param.mtx, iRet), "trylock should have failed", param.result)) {
		// only take the lock, if we didn't get it already by TRYLOCK
		// should block here until main thread releases mutex
		Trace("waiting in mutex...");
		t_assertm_intoany(LOCKMUTEX(param.mtx, iRet), "mutex lock failed", param.result);
	}
	// slow down to check TRYLOCK from within mainthread
	TestSleep(30);
	Trace("unlocking mutex");
	t_assertm_intoany(UNLOCKMUTEX(param.mtx, iRet), "mutex unlock failed", param.result);
	// wakeup joiners etc
	DELETETHREAD();
	// cleanup of thread resources
	THRDETACH( THRID() );
#if !defined(WIN32)
	// suppress compiler warning
	return 0;
#endif
}

void SystemAPITest::MUTEXTest()
{
	StartTrace(SystemAPITest.MUTEXTest);
	// try to create a SimpleMutex, lock it trylock it and unlock it
	// assume that a SimpleMutex can only be locked once, even on some systems mutexes are already
	// recursively lockable by the same thread, so we do not test recursive behavior here
	MUTEX mtx;
	// assert some simple API things
	int iRet = 0;
	t_assertm(&mtx == (MUTEXPTR)&mtx, "check implementation of MUTEXPTR-macro");
	t_assertm(&mtx == GETMUTEXPTR(mtx), "check implementation of GETMUTEXPTR-macro");
	t_assertm(CREATEMUTEX(mtx, iRet), "mutex creation failed");
	t_assertm(LOCKMUTEX(mtx, iRet), "mutex lock failed");

	SimpleTestThread thread;
	Anything result;
	mutexthreadparam params(mtx, result);
	if (t_assert(thread.Start(MUTEXTestThread, (void *)&params))) {
		TestSleep(10);
		Trace("releasing mutex");
		t_assertm(UNLOCKMUTEX(mtx, iRet), "mutex release should succeed");
		// thread should now have locked the mutex
		TestSleep(10);
		if (t_assertm(!TRYLOCK(mtx, iRet), "trylock should have failed")) {
			// if we've got it already otherwise we deadlock just below
			// synchronize for thread termination
			t_assertm(LOCKMUTEX(mtx, iRet), "mutex lock failed");
		}
		t_assertm(UNLOCKMUTEX(mtx, iRet), "mutex release should succeed");
		t_assertm(DELETEMUTEX(mtx, iRet), "mutex deletion failed");
		TestSleep(100); // try to really wait for thread to be done...
	}
	PrintResult( result );
}

struct semathreadparam {
	semathreadparam(SEMA &i_sema, Anything &i_result) : sema(i_sema), result(i_result) {}
	SEMA	&sema;
	Anything &result;
};

THREADWRAPPERFUNCDECL(SEMATestThread, thrdparam)
{
	semathreadparam &param = *(semathreadparam *)thrdparam;
	// should fail the first time because we are in another thread right here
	if (!t_assertm_intoany(!TRYSEMALOCK(param.sema), "trylock should have failed", param.result)) {
		// oops, we got unwanted lock, release it or we might deadlock
		t_assertm_intoany(UNLOCKSEMA(param.sema), "sema unlock failed", param.result);
	}
	// should block here until main thread produced one item
	t_assertm_intoany(LOCKSEMA(param.sema), "sema lock failed", param.result);
	// slow down to check TRYLOCK from within mainthread
	TestSleep(20);
	// produce an item for the main thread
	t_assertm_intoany(UNLOCKSEMA(param.sema), "sema unlock failed", param.result);
	// wakeup joiners etc
	DELETETHREAD();
	// cleanup of thread resources
	THRDETACH( THRID() );
#if !defined(WIN32)
	// suppress compiler warning
	return 0;
#endif
}

THREADWRAPPERFUNCDECL(SEMATestThreadFunc2, thrdparam)
{
	semathreadparam &param = *(semathreadparam *)thrdparam;
	// should fail the first time because we are in another thread right here
	t_assertm_intoany(UNLOCKSEMA(param.sema), "sema unlock failed", param.result);
	// wakeup joiners etc
	DELETETHREAD();
	// cleanup of thread resources
	THRDETACH( THRID() );
#if !defined(WIN32)
	// suppress compiler warning
	return 0;
#endif
}

void SystemAPITest::SEMATest()
{
	StartTrace(SystemAPITest.SEMATest);
	SEMA sema;
	// assert some simple API things
	t_assertm(&sema == (SEMAPTR)&sema, "check implementation of SEMAPTR-macro");
	t_assertm(&sema == GETSEMAPTR(sema), "check implementation of GETSEMAPTR-macro");

	// create a semaphore which initially has no free resources
	t_assertm(CREATESEMA(sema, 0), "sema creation with count of zero failed");
	// now a lock should fail, because no resource is free
	t_assertm(!TRYSEMALOCK(sema), "sema trylock should fail");
	// a signal should be possible, one resource available
	t_assertm(UNLOCKSEMA(sema), "sema unlock failed");
	// a lock of this resource should work now
	t_assertm(TRYSEMALOCK(sema), "sema trylock failed");
	t_assertm(DELETESEMA(sema), "sema deletion failed");

	t_assertm(CREATESEMA(sema, 1), "sema creation failed");
	// one resource should be available
	t_assertm(TRYSEMALOCK(sema), "sema trylock failed");
	// now we should block
	t_assertm(!TRYSEMALOCK(sema), "sema trylock should have failed");
	// releasing the resource should work
	t_assertm(UNLOCKSEMA(sema), "sema unlock failed");
	// testing LOCKSEMA now
	t_assertm(LOCKSEMA(sema), "sema lock failed");
	t_assertm(DELETESEMA(sema), "sema deletion failed");

	Anything result, result2;
	SimpleTestThread thread1, thread2;
	t_assertm(CREATESEMA(sema, 0), "sema creation with count of zero failed");
	semathreadparam params(sema, result);
	if (thread1.Start(SEMATestThread, (void *)&params)) {
		// thread blocks on sema until we 'produce' one item
		TestSleep(20);
		// let thread run
		t_assertm(UNLOCKSEMA(sema), "sema unlock failed");
		TestSleep(10);
		// try to get an item, thread waits and then produces one item to let us go ahead
		if (t_assertm(!TRYSEMALOCK(sema), "thread should not yet have produced an item")) {
			// otherwise, we got a timing problem, avoid deadlock below.
			// now wait on item
			t_assertm(LOCKSEMA(sema), "sema lock failed");
		}
		t_assertm(DELETESEMA(sema), "sema deletion failed");
	}
	t_assertm(CREATESEMA(sema, 0), "sema creation with count of zero failed");
	semathreadparam params2(sema, result2);
	if (thread1.Start(SEMATestThreadFunc2, (void *)&params) && thread2.Start(SEMATestThreadFunc2, (void *)&params2)) {
		t_assertm(LOCKSEMA(sema), "sema lock failed");
		t_assertm(LOCKSEMA(sema), "sema lock failed");
		t_assertm(!TRYSEMALOCK(sema), "sema trylock should have failed");
		t_assertm(DELETESEMA(sema), "sema deletion failed");
		TestSleep(100); // wait for threads to finish
	}
	PrintResult( result );
	PrintResult( result2 );
}

struct condthreadparam {
	condthreadparam(COND &i_cond, MUTEX &i_mtx, int &i_count) : cond(i_cond), mtx(i_mtx), count(i_count) {}
	COND	&cond;
	MUTEX	&mtx;
	Anything result;
	int	&count;
};

THREADWRAPPERFUNCDECL(CONDITIONTestWaitOnSignalFunc, thrdparam)
{
	StartTrace(SystemAPITest.CONDITIONTestWaitOnSignalFunc);
	condthreadparam &param = *(condthreadparam *)thrdparam;
	int iRet = 0;
	// should fail the first time because we are in another thread right here
	// wait 10ms for condition to be signalled, should fail because we sleep for some seconds in mainthread
	// need to lock mutex before we can wait on it
	t_assertm_intoany(LOCKMUTEX(param.mtx, iRet), "mutex lock failed", param.result);
	Trace("*** Waiting 1 ms");
	t_assertm_intoany((CONDTIMEDWAIT(param.cond, GETMUTEXPTR(param.mtx), 0, 1000000) == TIMEOUTCODE), "wait on condition should timeout", param.result);
	// wait on condition until mainthread signals it
	Trace("*** Wait because mainthread works");
	t_assertm_intoany(CONDWAIT(param.cond, GETMUTEXPTR(param.mtx)), "wait on condition should work", param.result);
	Trace("val of count is:" << (long)param.count);
	param.count = param.count + 1;
	Trace("val of count is:" << (long)param.count);
	Trace("*** Unlock mutex after cond-signaled");
	t_assertm_intoany(UNLOCKMUTEX(param.mtx, iRet), "mutex unlock failed", param.result);
	if (!SIGNALCOND(param.cond)) {
		SystemLog::Error("SIGNALCOND failed");
	}

	// wakeup joiners etc
	DELETETHREAD();
	// cleanup of thread resources
	THRDETACH( THRID() );
#if !defined(WIN32)
	// suppress compiler warning
	return 0;
#endif
}
void SystemAPITest::CONDITIONSTestMain(int nofthreads, bool broadcast)
{
	StartTrace(SystemAPITest.CONDITIONSTestMain);
	COND cond;
	MUTEX mtx;
	int count = 0, iRet = 0;
	t_assertm(CREATECOND(cond), "cond creation failed");
	t_assertm(CREATEMUTEX(mtx, iRet), "mutex creation failed");
	if (!t_assert(nofthreads >= 1)) {
		return;
	}
	SimpleTestThread **pthread;
	pthread = new SimpleTestThread*[nofthreads];
	condthreadparam **params;
	params = new condthreadparam*[nofthreads];
	for (int j = 0; j < nofthreads; j++) {
		pthread[j] = new SimpleTestThread;
		params[j] = new condthreadparam(cond, mtx, count);
		t_assert(pthread[j]->Start(CONDITIONTestWaitOnSignalFunc, (void *)params[j]));
	}
	SleepAndSignal(*params[0], nofthreads, broadcast);
	t_assertm(DELETECOND(cond), "cond deletion failed");
	t_assertm(DELETEMUTEX(mtx, iRet), "mutex deletion failed");
	// put the messages collected within thread
	for (int i = 0; i < nofthreads; i++) {
		PrintResult( params[i]->result );
		delete params[i];
		delete pthread[i];
	}
	delete[] params;
	delete[] pthread;
}

void SystemAPITest::SleepAndSignal(condthreadparam &p, int expectedcount, bool broadcast)
{
	StartTrace(SystemAPITest.SleepAndSignal);
	TestSleep(200); // allow waiting thread to timeout on condition
	Trace("    Signaling cond after 0.2 secs sleep");
	int numberofsignals = broadcast ? 1 : expectedcount;
#ifdef WIN32
	numberofsignals = 1; // signaling always broadcasts and awakes all threads
#endif
	int iRet = 0;
	for (long i = numberofsignals; i > 0; i--) {
		Trace("signal number " << (long)numberofsignals - i + 1L);
		t_assertm(broadcast ? BROADCASTCOND(p.cond) : SIGNALCOND(p.cond), "cond signal should have worked");
		// synchronize on thread
		t_assertm(LOCKMUTEX(p.mtx, iRet), "mutex lock should succeed");
		while (p.count < (expectedcount + 1 - i)) {
			if (CONDTIMEDWAIT(p.cond, GETMUTEXPTR(p.mtx), 5, 0) == TIMEOUTCODE) {
				t_assertm(false, "unexpected timeout on condition");
				break;
			}
		}
		t_assertm(UNLOCKMUTEX(p.mtx, iRet), "mutex unlock failed");
	}
	assertEqual(expectedcount, p.count);
	Trace("   MutexUnlock ");
}
void SystemAPITest::CONDITIONSignalSingleTest()
{
	StartTrace(SystemAPITest.CONDITIONSignalSingleTest);
	CONDITIONSTestMain(1, false);
}

void SystemAPITest::CONDITIONSignalManyTest()
{
	StartTrace(SystemAPITest.CONDITIONSignalManyTest);
	CONDITIONSTestMain(5, false);
}

void SystemAPITest::CONDITIONBroadcastSingleTest()
{
	StartTrace(SystemAPITest.CONDITIONBroadcastSingleTest);
	CONDITIONSTestMain(1, true);
}

void SystemAPITest::CONDITIONBroadcastManyTest()
{
	StartTrace(SystemAPITest.CONDITIONBroadcastManyTest);
	CONDITIONSTestMain(5, true);
}
void SystemAPITest::CONDITIONTimedWaitTimeoutTest()
{
	StartTrace(SystemAPITest.CONDITIONTimedWaitTimeoutTest);
	COND cond;
	MUTEX mtx;
	// assert some simple API things
	t_assertm(&cond == (CONDPTR)&cond, "check implementation of CONDAPTR-macro");
	t_assertm(&cond == GETCONDPTR(cond), "check implementation of GETCONDPTR-macro");
	t_assertm(CREATECOND(cond), "cond creation failed");
	t_assertm(DELETECOND(cond), "cond deletion failed");

	int iRet = 0;
	t_assertm(CREATEMUTEX(mtx, iRet), "mutex creation failed");
	t_assertm(CREATECOND(cond), "cond creation failed");
	t_assertm(LOCKMUTEX(mtx, iRet), "mutex lock should succeed");
	const long WAITUSEC = 100 * 1000; // = 100 ms
#if defined(WIN32)
	clock_t t_start = clock();
#else
	struct timeval beforetime;
	t_assertm(!gettimeofday(&beforetime, 0), "cannot check time wait accuracy");
#endif
	t_assertm((CONDTIMEDWAIT(cond, GETMUTEXPTR(mtx), 0, 1000 * WAITUSEC) == TIMEOUTCODE), "should run into timout");

#if defined(WIN32)
	clock_t t_end = clock();
	Trace("clocks per sec:" << (long)CLOCKS_PER_SEC);
	long difference = (t_end - t_start) * 1000000 / CLOCKS_PER_SEC;
#else
	struct timeval aftertime;
	t_assertm(!gettimeofday(&aftertime, 0), "cannot check time wait accuracy");
	long difference = (aftertime.tv_sec - beforetime.tv_sec) * 1000000 + (aftertime.tv_usec - beforetime.tv_usec);
#endif
	if ( !t_assertm(abs(difference - WAITUSEC) < 20000, "assert 20 milisecond accuracy of condition timed wait") ) {
		String strTimes("WAITUSEC:");
		strTimes << (long)WAITUSEC << " difference:" << difference;
		SystemLog::WriteToStderr(strTimes);
	}

	t_assertm(UNLOCKMUTEX(mtx, iRet), "mutex unlock failed");
	t_assertm(DELETECOND(cond), "cond deletion failed");
	t_assertm(DELETEMUTEX(mtx, iRet), "mutex deletion failed");
}
