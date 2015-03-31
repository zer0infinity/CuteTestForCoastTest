/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#include "ThreadPools.h"
#include "StringStream.h"
#include "SystemLog.h"
#include "WPMStatHandler.h"
#include "MT_Storage.h"
#include <iomanip>

ThreadPoolManager::ThreadPoolManager(const char *name)
	: fTerminated(true)
	, fMutex( name, coast::storage::Global() )
	, fName( name, coast::storage::Global() )
	, fRunningThreads(0L)
	, fStartedThreads(0L)
	, fTerminatedThreads(0L)
	, fpStatEvtHandler()
{
}

ThreadPoolManager::~ThreadPoolManager()
{
	StartTrace(ThreadPoolManager.~ThreadPoolManager);
	Terminate(1, GetPoolSize() * 1 + 5);
	PrintStatisticsOnStderr( GetName() );
}

void ThreadPoolManager::DoGetStatistic(Anything &statistics)
{
	if ( fpStatEvtHandler.get() ) {
		fpStatEvtHandler->Statistic( statistics );
	}
}

String ThreadPoolManager::GetName()
{
	return fName;
}

bool ThreadPoolManager::Init(int maxParallelRequests, ROAnything roaThreadArgs)
{
	StartTrace1(ThreadPoolManager.Init, "[" << GetName() << "]");
	if ( !fTerminated ) {
		// make sure existing workers are terminated properly
		if ( Terminate(1, GetPoolSize() * 1 + 5) != 0 ) {
			SYSWARNING("could not properly terminate the existing threads in the pool while initializing again");
			return false;
		}
	}
	fTerminated = false;
	// reset count of started threads
	fStartedThreads = 0L;
	fTerminatedThreads = 0L;
	if ( AllocPool(maxParallelRequests, roaThreadArgs) ) {
		if ( InitPool(roaThreadArgs) ) {
			fpStatEvtHandler = StatEvtHandlerPtrType( new WPMStatHandler( maxParallelRequests ) );
			return true;
		}
	}
	return false;
}

int ThreadPoolManager::Start(bool usePoolStorage, int poolStorageSize, int numOfPoolBucketSizes, ROAnything roaThreadArgs)
{
	StartTrace1(ThreadPoolManager.Start, "[" << GetName() << "]");
	if ( GetPoolSize() > 0 ) {
		long lStartSuccess = 0L;
		for (long i = 0, sz = GetPoolSize(); i < sz; ++i) {
			Thread *t = DoGetThread(i);
			if ( t ) {
				Allocator *pAlloc = coast::storage::Global();
				if (usePoolStorage) {
					// use different memory manager for each thread
					pAlloc = MT_Storage::MakePoolAllocator(poolStorageSize, numOfPoolBucketSizes, 0);
				}
				if ( pAlloc == NULL ) {
					SYSERROR("was not able to create PoolAllocator for Thread# " << i << ", check config!");
				} else {
					if ( t->Start( pAlloc, DoGetStartConfig(i, roaThreadArgs) ) ) {
						++lStartSuccess;
						SYSDEBUG("Thread# " << i << " started");
					} else {
						SYSERROR("Failed to start Thread# " << i << ", check config!");
					}
				}
			}
		}
		SYSDEBUG("started " << fStartedThreads << " of " << lStartSuccess << "(" << GetPoolSize() << ") started successfully.");
		{
			LockUnlockEntry me(fMutex);
			DiffTimer dt(DiffTimer::eSeconds);

			while ( (fStartedThreads < lStartSuccess) && (dt.Diff() < 30) ) {
				fCond.TimedWait(fMutex, 1);
				SYSDEBUG("started " << fStartedThreads << " of " << lStartSuccess << " successful @" << (long)dt.Diff() << "s");
			}
			Trace("Time waited for start: " << dt.Diff() << " fStartedThreads:" << fStartedThreads << " PoolSize:" << GetPoolSize());
			SYSDEBUG("Time waited for start: " << dt.Diff() << " fStartedThreads:" << fStartedThreads << " PoolSize:" << GetPoolSize());

			dt.Reset();
			while ( ( ( fRunningThreads + fTerminatedThreads ) < lStartSuccess) && (dt.Diff() < 30) ) {
				SYSINFO("before waiting on condition @" << (long)dt.Diff() << "s");
				fCond.TimedWait(fMutex, 1);
				SYSDEBUG("running " << fRunningThreads << " of " << lStartSuccess << " (" << fTerminatedThreads << " terminated) @" << (long)dt.Diff() << "s");
			}
			Trace("Time waited for running: " << dt.Diff() << " fRunningThreads:" << fRunningThreads << " PoolSize:" << GetPoolSize());
			SYSDEBUG("Time waited for running: " << dt.Diff() << " fRunningThreads:" << fRunningThreads << " PoolSize:" << GetPoolSize());
			if ( lStartSuccess < GetPoolSize() ) {
				SYSERROR("Not all Thread.Start() calls were successful (out of memory?)! Only " << fStartedThreads << " of " << lStartSuccess << "(" << GetPoolSize() << ") could be started successfully.");
			}
			if ( fStartedThreads < lStartSuccess ) {
				SYSWARNING("Only " << fStartedThreads << " of " << lStartSuccess << "(" << GetPoolSize() << ") passed Thread::eStarted successfully.");
			}
			if ( fRunningThreads < lStartSuccess ) {
				SYSWARNING("Only " << fRunningThreads << " of " << lStartSuccess << "(" << GetPoolSize() << ") are (still) running. (" << fTerminatedThreads << " terminated)");
			}
		}
		// this section is intended to let all threads go into their run method before starting their work
		// here we signal that they should begin working;
		for (long j = 0; j < lStartSuccess; ++j) {
			Thread *t = DoGetThread(j);
			if ( t ) {
				t->CheckState(Thread::eRunning, 0, 1000);
				SYSDEBUG("Thread# " << j << " checked to run");
			}
		}
		return fStartedThreads - GetPoolSize();
	}
	return -1;
}

int ThreadPoolManager::Join(long lMaxSecsToWait)
{
	StartTrace1(ThreadPoolManager.Join, "[" << GetName() << "]");
	long lStillRunning = 0L;
	{
		LockUnlockEntry me(fMutex);
		DiffTimer dt;
		long lWaited( (lMaxSecsToWait > 0) ? 0L : -1L ), lIncr( (lMaxSecsToWait > 0) ? 1L : 0L );
		if ( lIncr ) {
			Trace("waiting on Thread Join for at most " << lMaxSecsToWait << "s");
		} else {
			Trace("waiting on Thread Join forever!");
		}
		while ( ( ( fRunningThreads > 0 ) || ( fTerminatedThreads < fStartedThreads ) ) && ( lWaited < lMaxSecsToWait ) ) {
			Trace("[" << GetName() << "] still " << fRunningThreads << " running but only " << fTerminatedThreads << " terminated Threads, doing a TimedWait");
			fCond.TimedWait(fMutex, 1);
			lWaited += lIncr;
		}
		Trace("Time waited for join: " << dt.Diff() << "ms");
		lStillRunning = fRunningThreads;
		Trace("[" << GetName() << "] still running: " << lStillRunning);
		SYSDEBUG("still running: " << lStillRunning);
	}
	return lStillRunning;
}

bool ThreadPoolManager::AwaitReady(long secs)
{
	StartTrace1(ThreadPoolManager.AwaitReady, "[" << GetName() << "]");
	DiffTimer aTimer(DiffTimer::eSeconds);
	int msgCount = 0;
	long lCurrRequests( fpStatEvtHandler->GetCurrentParallelRequests() );
	// check for active requests
	while ( fpStatEvtHandler.get() && ( lCurrRequests > 0 ) && ( aTimer.Diff() < secs ) ) {
		Thread::Wait(1);						// wait for 1 second
		lCurrRequests = fpStatEvtHandler->GetCurrentParallelRequests();
		++msgCount;
		if ( (msgCount % 5) == 0 ) {
			OStringStream os;
			os << "MaxSecToWait: " << std::setw(4) << secs <<
			   "  SecsWaited: "	 << std::setw(4) << msgCount <<
			   "  Pending requests: " <<  std::setw(6) <<  lCurrRequests << std::endl;
			SystemLog::Info( os.str() );
		}
	}
	lCurrRequests = fpStatEvtHandler->GetCurrentParallelRequests();
	if ( lCurrRequests > 0 ) {
		SYSWARNING("Still " << lCurrRequests << " working threads after waiting for " << msgCount << "s");
	}
	return ( fpStatEvtHandler->GetCurrentParallelRequests() <= 0 );
}

int ThreadPoolManager::Terminate(long lWaitToTerminate, long lWaitOnJoin)
{
	StartTrace1(ThreadPoolManager.Terminate, "[" << GetName() << "]");
	int result = DoTerminate(lWaitToTerminate);
	fTerminated = true;
	// just do a Join when not all threads have terminated
	if ( ( result == 0 ) && ( Join(lWaitOnJoin) == 0L ) ) {
		// only delete the pool if all threads have terminated
		DoDeletePool();
	}
	return result;
}

int ThreadPoolManager::DoTerminate(long lWaitToTerminate)
{
	StartTrace1(ThreadPoolManager.DoTerminate, "[" << GetName() << "]");
	int result = 0, iSign = 1;
	for (long i = 0, sz = GetPoolSize(); i < sz; ++i) {
		Thread *lt = (Thread *)DoGetThread(i);
		if ( !lt ) {
			SYSERROR("could not get Thread at position " << i);
			iSign = -1;
		} else {
			// count only the non-terminated threads
			if ( !lt->Terminate(lWaitToTerminate) ) {
				++result;
				SYSERROR("Termination of thread[" << i << "] failed!");
				Trace("Termination of thread[" << i << "] failed!");
			}
		}
	}
	// signal errors using negative value
	result *= iSign;
	Trace("returning " << result);
	return result;
}

void ThreadPoolManager::Update(tObservedPtr pObserved, tArgsRef roaUpdateArgs)
{
	ROAnything roaRunStateNew, roaThreadStateNew;
	roaUpdateArgs.LookupPath(roaRunStateNew, "RunningState.New");
	roaUpdateArgs.LookupPath(roaThreadStateNew, "ThreadState.New");
	{
		StartTrace1(ThreadPoolManager.Update, "[" << GetName() << "] CallId:" << (long)Thread::MyId());
		LockUnlockEntry me(fMutex);
		TraceAny(roaUpdateArgs, "state event received");
		if ( !roaThreadStateNew.IsNull() ) {
			switch ( roaThreadStateNew.AsLong(-1)) {
				case Thread::eCreated:
				case Thread::eStartRequested:
					break;

				case Thread::eStarted:
					++fStartedThreads;
					break;

				case Thread::eRunning:
					++fRunningThreads;
					break;

				case Thread::eTerminationRequested:
					break;

				case Thread::eTerminated:
					--fRunningThreads;
					++fTerminatedThreads;
					break;

				default:
					break;
			}
		}
		if ( !roaRunStateNew.IsNull() ) {
			switch ( roaRunStateNew.AsLong(-1) ) {
				case Thread::eReady:
					if ( fpStatEvtHandler.get() ) {
						fpStatEvtHandler->HandleStatEvt( WPMStatHandler::eLeave );
					}
					break;

				case Thread::eWorking:
					if ( fpStatEvtHandler.get() ) {
						fpStatEvtHandler->HandleStatEvt( WPMStatHandler::eEnter );
					}
					break;

				default:
					break;
			}
		}
		fCond.BroadCast();
	}
}

bool ThreadPoolManager::AllocPool(long poolSize, ROAnything roaThreadArgs)
{
	StartTrace(ThreadPoolManager.AllocPool);
	bool bSuccess( false );
	if ( poolSize > 0 ) {
		bSuccess = DoAllocPool(poolSize, roaThreadArgs);
	}
	return bSuccess;
}

bool ThreadPoolManager::DoAllocPool(long poolSize, ROAnything roaThreadArgs)
{
	StartTrace(ThreadPoolManager.DoAllocPool);

	for (long i = 0; i < poolSize; ++i) {
		Thread *t = DoAllocThread(i, DoGetInitConfig(i, roaThreadArgs));
		if (!t) {
			Trace("Alloc of thread[" << i << "] failed");
			return false;
		}
		fThreadPool[i] = Anything((IFAObject *)t);
	}
	return true;
}

bool ThreadPoolManager::InitPool(ROAnything roaThreadArgs)
{
	StartTrace(ThreadPoolManager.InitPool);
	TraceAny(roaThreadArgs, "roaThreadArgs");
	for (long i = 0, sz = GetPoolSize(); i < sz; ++i) {
		Thread *t = DoGetThread(i);
		if ( !t ) {
			SYSWARNING("could not allocate thread[" << i << "]");
			return false;
		}
		if ( t->Init( DoGetInitConfig(i, roaThreadArgs) ) != 0 ) {
			Trace("init of thread[" << i << "] failed");
			return false;
		}
		t->AddObserver(this);
	}
	return true;
}

void ThreadPoolManager::DoDeletePool()
{
	StartTrace(ThreadPoolManager.DoDeletePool);
	for (long i = GetPoolSize() - 1; i >= 0; --i) {
		Thread *t = DoGetThread(i);
		fThreadPool.Remove(i);
		delete t;
	}
}

Thread *ThreadPoolManager::DoGetThread(long i)
{
	Thread *pThr( NULL );
	if ( i >= 0 && i < GetPoolSize() ) {
		pThr = (Thread *)fThreadPool[i].AsIFAObject(0);
	}
	StatTrace(ThreadPoolManager.DoGetThread, "idx: " << i << " ThrdId: " << ( ( pThr != NULL ) ? pThr->GetId() : -1 ), coast::storage::Current() );
	return pThr;
}

long ThreadPoolManager::GetPoolSize()
{
	return fThreadPool.GetSize();
}

ROAnything ThreadPoolManager::DoGetInitConfig(long i, ROAnything roaThreadArgs)
{
	return roaThreadArgs[i];
}

ROAnything ThreadPoolManager::DoGetStartConfig(long i, ROAnything roaThreadArgs)
{
	return roaThreadArgs[i];
}

WorkerThread::WorkerThread(const char *name)
	: Thread(name)
	, fRefreshAllocator(false)
{
}

WorkerThread::~WorkerThread()
{
}

int WorkerThread::Init(ROAnything workerInit)
{
	StartTrace(WorkerThread.Init);
	DoInit(workerInit);
	return 0;
}

void WorkerThread::Run()
{
	// if you add Traces here, expect PoolAllocator to tell you about unfreed memory !!
	while ( CheckRunningState( eWorking ) ) {
		DoProcessWorkload();
		SetReady();
	}
}

void WorkerThread::DoReadyHook(ROAnything)
{
	if ( fRefreshAllocator ) {
		// StatTrace memory can still be on current storage because its code will be inserted in a subscope
		StatTrace(WorkerThread.DoReadyHook, "WorkerThread [" << GetName() << "] fAllocator->Refresh", coast::storage::Current());
		// reorganize allocator memory and hope that no more memory is allocated anymore
		if ( fAllocator ) {
			fAllocator->Refresh();
		}
	}
}

WorkerPoolManager::WorkerPoolManager(const char *name)
	: fMutex( name, coast::storage::Global() )
	, fName( name, coast::storage::Global() )
	, fPoolSize(0)
	, fBlockRequestHandling(false)
	, fTerminated(true)
	, fpStatEvtHandler()
{
	StartTrace(WorkerPoolManager.Ctor);
}

WorkerPoolManager::~WorkerPoolManager()
{
	StartTrace(WorkerPoolManager.Dtor);
	PrintStatisticsOnStderr( GetName() );
}

bool WorkerPoolManager::CanReInitPool()
{
	return true;
}

String WorkerPoolManager::GetName()
{
	return fName;
}

int WorkerPoolManager::Init(int maxParallelRequests, int usePoolStorage, int poolStorageSize, int numOfPoolBucketSizes, ROAnything roaWorkerArgs)
{
	StartTrace(WorkerPoolManager.Init);
	Finis(5);	// nothing to do if yet uninitialized

	fTerminated = false;

	if ( AllocPool(maxParallelRequests, roaWorkerArgs) ) {
		Trace("allocation succeeded, preparing pool...");
		return PreparePool(usePoolStorage, poolStorageSize, numOfPoolBucketSizes, roaWorkerArgs);
	}
	Trace("returning -1");
	return -1;
}

bool WorkerPoolManager::Finis(long lWaitTime)
{
	StartTrace(WorkerPoolManager.Finis);
	bool reallyterminated = Terminate(lWaitTime);
	Assert(reallyterminated);
	ROAnything roaWorkerArgs;
	DoDeletePool(roaWorkerArgs);
	return reallyterminated;
}

bool WorkerPoolManager::Terminate(long secs)
{
	StartTrace(WorkerPoolManager.Terminate);
	// sends signals to all running requests
	// that should terminate the threads
	// it is not 100% a solution because
	// delivery of signals is dependent on
	// the state of the thread
	bool success = true;
	if (!fTerminated) {
		Trace("i am still not terminated, have to Terminate " << GetPoolSize() << " workers with timeout " << secs << "s");
		for (long i = 0; i < GetPoolSize(); ++i) {
			Trace("at worker " << i);
			WorkerThread *hs = DoGetWorker(i);
			success = (hs->Terminate(secs) && success);
		}
		fTerminated = true;
	}
	return success;
}

bool WorkerPoolManager::AllocPool(long poolSize, ROAnything roaWorkerArgs)
{
	StartTrace(WorkerPoolManager.AllocPool);
	TraceAny(roaWorkerArgs, "roaWorkerArgs");
	fPoolSize = (poolSize > 0) ? poolSize : 1;
	Assert(GetPoolSize() > 0);
	DoAllocPool(roaWorkerArgs);
	// check if at least something was done
	return ( DoGetWorker(0) != NULL );
}

int WorkerPoolManager::PreparePool(int usePoolStorage, int poolStorageSize, int numOfPoolBucketSizes, ROAnything roaWorkerArgs)
{
	StartTrace(WorkerPoolManager.PreparePool);
	// since an array uses the default constructor
	// we have to initialize each object with the
	// processor it is working with
	if ( CanReInitPool() ) {
		Trace("CanReInitPool() was true, do InitPool");
		return InitPool(usePoolStorage != 0, poolStorageSize, numOfPoolBucketSizes, roaWorkerArgs);
	}
	// it makes no sense to start the thread pool if no processor is available
	String logMessage("cannot re-init pool");
	Trace(logMessage);
	SystemLog::Alert(logMessage);
	return -1;
}

int WorkerPoolManager::InitPool(bool usePoolStorage, long poolStorageSize, int numOfPoolBucketSizes, ROAnything roaWorkerArgs)
{
	StartTrace(WorkerPoolManager.InitPool);
	Trace("fPoolSize = " << GetPoolSize());
	for (long i = 0; i < GetPoolSize(); ++i) {
		Trace("initializing worker number " << i << usePoolStorage ? "with pool allocator" : "with global allocator");
		WorkerThread *wt = DoGetWorker(i);
		Trace("got worker at " << long(wt));
		wt->Init(roaWorkerArgs);
		wt->AddObserver(this);
		Trace("init done");
		if (usePoolStorage) {
			// use different memory manager for each thread
			wt->Start(MT_Storage::MakePoolAllocator(poolStorageSize, numOfPoolBucketSizes), roaWorkerArgs);
		} else {
			wt->Start(coast::storage::Global(), roaWorkerArgs);
		}
		Trace("Start done");
		wt->CheckState(Thread::eRunning);
		Trace("CheckState done");
	}
	//allocate hard for now; maybe later we can do it with a factory
	fpStatEvtHandler = StatEvtHandlerPtrType( new WPMStatHandler(GetPoolSize()) );
	return 0;
}

void WorkerPoolManager::Update(tObservedPtr pObserved, tArgsRef roaUpdateArgs)
{
	ROAnything roaRunStateNew;
	if ( roaUpdateArgs.LookupPath(roaRunStateNew, "RunningState.New") ) {
		StartTrace1(WorkerPoolManager.Update, "[" << GetName() << "] CallId:" << (long)Thread::MyId());
		TraceAny(roaUpdateArgs, "state event received");
		switch ( roaRunStateNew.AsLong(-1) ) {
			case Thread::eReady:
				if ( fpStatEvtHandler.get() ) {
					fpStatEvtHandler->HandleStatEvt( WPMStatHandler::eLeave );
				}
				break;

			case Thread::eWorking:
				if ( fpStatEvtHandler.get() ) {
					fpStatEvtHandler->HandleStatEvt( WPMStatHandler::eEnter );
				}
				break;

			default:
				break;
		}
		fCond.BroadCast();
	}
}

long WorkerPoolManager::ResourcesUsed()
{
	// accessor to current active requests
	if ( fpStatEvtHandler.get() ) {
		return fpStatEvtHandler->GetCurrentParallelRequests();
	}
	return 0L;
}

void WorkerPoolManager::BlockRequests()
{
	// accessor to current active requests
	StatTrace(WorkerPoolManager.BlockRequests, "Blocking requests", coast::storage::Current());
	{
		LockUnlockEntry me(fMutex);
		fBlockRequestHandling = true;
		fCond.BroadCast();
	}
}

void WorkerPoolManager::UnblockRequests()
{
	// accessor to current active requests
	StatTrace(WorkerPoolManager.UnblockRequests, "Unblocking requests", coast::storage::Current());
	{
		LockUnlockEntry me(fMutex);
		fBlockRequestHandling = false;
		fCond.BroadCast();
	}
}

long WorkerPoolManager::GetPoolSize()
{
	// accessor to current max running
	// requests
	return fPoolSize;
}

bool WorkerPoolManager::AwaitEmpty(long sec)
{
	StartTrace(WorkerPoolManager.AwaitEmpty);
	// check for active requests running
	// but wait at most sec seconds
	DiffTimer aTimer(DiffTimer::eSeconds);
	int msgCount = 0;
	long lCurrRequests = fpStatEvtHandler.get() ? fpStatEvtHandler->GetCurrentParallelRequests() : 0L;
	// check for active requests
	while ( fpStatEvtHandler.get() && ( lCurrRequests > 0 ) && ( aTimer.Diff() < sec ) ) {
		Thread::Wait(1);						// wait for 1 second
		lCurrRequests = fpStatEvtHandler->GetCurrentParallelRequests();
		++msgCount;
		if ( (msgCount % 5) == 0 ) {
			OStringStream os;
			os << "MaxSecToWait: " << std::setw(4) << sec <<
			   "  SecsWaited: "	 << std::setw(4) << msgCount <<
			   "  Pending requests: " <<  std::setw(6) <<  lCurrRequests << std::endl;
			SystemLog::Info( os.str() );
		}
	}
	lCurrRequests = fpStatEvtHandler.get() ? fpStatEvtHandler->GetCurrentParallelRequests() : 0L;
	if ( lCurrRequests > 0 ) {
		SYSWARNING("Still " << lCurrRequests << " working threads after waiting for " << msgCount << "s");
	}
	return ( lCurrRequests <= 0 );
}

WorkerThread *WorkerPoolManager::FindNextRunnable(long lFindWorkerHint)
{
	StartTrace(WorkerPoolManager.FindNextRunnable);
	// at least one should be runnable, since the semaphore prevents the main thread from entering
	// this code when no slot is empty;
	WorkerThread *hs( NULL );
	bool bIsReady( false );
	long lCurrRequests = fpStatEvtHandler.get() ? fpStatEvtHandler->GetCurrentParallelRequests() : 0L;
	while ( ( GetPoolSize() <= lCurrRequests ) || fBlockRequestHandling ) {
		// release mutex and wait for condition
		fCond.TimedWait( fMutex, 0, 100000 );
		lCurrRequests = fpStatEvtHandler.get() ? fpStatEvtHandler->GetCurrentParallelRequests() : 0L;
	}
	Assert( GetPoolSize() > lCurrRequests );
	Trace("I slipped past the Critical Region and there are " << lCurrRequests << " requests in work and " << ( GetPoolSize() - lCurrRequests ) << " free workers");
	// use the lFindWorkerHint param as hint to get the next free WorkerThread
	if ( lFindWorkerHint < 0 ) {
		Trace("no hint given, selecting next available worker");
		lFindWorkerHint = ( lCurrRequests + 1L ) % GetPoolSize();
	} else {
		Trace("worker hint given: " << lFindWorkerHint << " which results in worker at index: " << ( lFindWorkerHint % fPoolSize ) );
		lFindWorkerHint = lFindWorkerHint % GetPoolSize();
	}
	while ( ( hs = DoGetWorker( lFindWorkerHint ) ) ) {
		if ( hs->IsReady( bIsReady ) && bIsReady ) {
			StatTrace( WorkerPoolManager.FindNextRunnable, "ready worker[" << lFindWorkerHint << "] found", coast::storage::Current() );
			break;
		}
		++lFindWorkerHint;
		lFindWorkerHint = ( lFindWorkerHint % GetPoolSize() );
	}
	return hs;
}

void WorkerPoolManager::DoGetStatistic(Anything &statistics)
{
	if ( fpStatEvtHandler.get() ) {
		fpStatEvtHandler->Statistic( statistics );
	}
}
