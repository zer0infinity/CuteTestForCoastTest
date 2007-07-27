/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

//--- interface include ---------------------------------------------------------
#include "ThreadPools.h"

//--- standard modules used ----------------------------------------------------
#include "StringStream.h"
#include "SysLog.h"
#include "DiffTimer.h"
#include "WPMStatHandler.h"
#include "Dbg.h"

//--- c-library modules used ---------------------------------------------------

//---- ThreadPoolManager ---------------------------------------------------------
ThreadPoolManager::ThreadPoolManager()
	: fTerminated(true)
	, fMutex("ThreadPoolManager")
	, fRunningThreads(0)
	, fStartedThreads(0)
{
}

ThreadPoolManager::~ThreadPoolManager()
{
	StartTrace(ThreadPoolManager.~ThreadPoolManager);
	Terminate(1, GetPoolSize() * 1 + 5);
}

bool ThreadPoolManager::Init(int maxParallelRequests, ROAnything roaThreadArgs)
{
	StartTrace(ThreadPoolManager.Init);
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
	if ( AllocPool(maxParallelRequests, roaThreadArgs) ) {
		return InitPool(roaThreadArgs);
	}
	return false;
}

int ThreadPoolManager::Start(bool usePoolStorage, int poolStorageSize, int numOfPoolBucketSizes, ROAnything roaThreadArgs)
{
	StartTrace(ThreadPoolManager.Start);
	if ( GetPoolSize() > 0 ) {
		for (long i = 0, sz = GetPoolSize(); i < sz; ++i) {
			Thread *t = DoGetThread(i);
			if ( t ) {
				Allocator *pAlloc = Storage::Global();
				if (usePoolStorage) {
					// use different memory manager for each thread
					pAlloc = MT_Storage::MakePoolAllocator(poolStorageSize, numOfPoolBucketSizes, 0);
				}
				t->Start( pAlloc, DoGetStartConfig(i, roaThreadArgs) );
			}
		}
		MutexEntry me(fMutex);
		me.Use();
		DiffTimer dt(1);

		while ( (fStartedThreads < GetPoolSize()) && (dt.Diff() < 30) ) {
			fCond.TimedWait(fMutex, 1);
		}
		Trace("Time waited for start: " << dt.Diff() << " fStartedThreads:" << fStartedThreads << " PoolSize:" << GetPoolSize());

		Assert( fStartedThreads <= GetPoolSize() );
		return fStartedThreads - GetPoolSize();
	}
	return -1;
}

int ThreadPoolManager::Join(long lMaxSecsToWait)
{
	StartTrace1(ThreadPoolManager.Join, "this:" << (long)this);
	long lStillRunning = 0L;
	{
		MutexEntry me(fMutex);
		me.Use();
		DiffTimer dt;
		long lWaited = ( (lMaxSecsToWait > 0) ? 0L : -1L ), lIncr = ( (lMaxSecsToWait > 0) ? 1L : 0L );
		if ( lIncr ) {
			Trace("waiting on Thread Join for at most " << lMaxSecsToWait << "s");
		} else {
			Trace("waiting on Thread Join forever!");
		}
		while ( ( fRunningThreads > 0 ) && ( lWaited < lMaxSecsToWait ) ) {
			Trace("still " << fRunningThreads << " running Threads, doing a TimedWait");
			fCond.TimedWait(fMutex, 1);
			lWaited += lIncr;
		}
		Trace("Time waited for join: " << dt.Diff() << "ms");
		lStillRunning = fRunningThreads;
		Trace("still running: " << lStillRunning);
	}
	return lStillRunning;
}

int ThreadPoolManager::Terminate(long lWaitToTerminate, long lWaitOnJoin)
{
	StartTrace1(ThreadPoolManager.Terminate, "this:" << (long)this);
	int result = DoTerminate(lWaitToTerminate);
	fTerminated = true;
	// just do a Join when not all threads have terminated
	if ( ( result == 0 ) || ( Join(lWaitOnJoin) == 0L ) ) {
		// only delete the pool if all threads have terminated
		DoDeletePool();
	}
	return result;
}

int ThreadPoolManager::DoTerminate(long lWaitToTerminate)
{
	StartTrace1(ThreadPoolManager.DoTerminate, "this:" << (long)this);
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

void ThreadPoolManager::Update(Thread *t, ROAnything roaStateArgs)
{
	MutexEntry me(fMutex);
	me.Use();
	{
		StartTrace(ThreadPoolManager.Update);
		TraceAny(roaStateArgs, "state event received");
		switch ( roaStateArgs["ThreadState"]["New"].AsLong(-1)) {
			case Thread::eTerminated:
				--fRunningThreads;
				break;

			case Thread::eRunning:
				++fStartedThreads;
				++fRunningThreads;
				break;
			case Thread::eCreated:
			case Thread::eStartRequested:
			case Thread::eStarted:
			case Thread::eTerminationRequested:
				break;
			default:
				break;

		}
	}
	fCond.BroadCast();
}

bool ThreadPoolManager::AllocPool(long poolSize, ROAnything roaThreadArgs)
{
	StartTrace(ThreadPoolManager.AllocPool);
	Assert(poolSize > 0);
	return ( poolSize > 0 && DoAllocPool(poolSize, roaThreadArgs) );
}

bool ThreadPoolManager::DoAllocPool(long poolSize, ROAnything roaThreadArgs)
{
	StartTrace(ThreadPoolManager.DoAllocPool);

	for (long i = 0; i < poolSize; ++i) {
		Thread *t = DoAllocThread(DoGetInitConfig(i, roaThreadArgs));
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
	StartTrace(ThreadPoolManager.DoGetThread);
	if ( i < 0 || i >= GetPoolSize()) {
		return 0;
	}
	return (Thread *)fThreadPool[i].AsIFAObject(0);
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

//--- WorkerThread implementation -------------------------------
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
	while ( IsRunning() ) {
		if ( CheckRunningState(eWorking) && IsRunning() ) {
			DoProcessWorkload();
			if ( IsRunning() ) {
				SetReady();
			}
		}
	}
}

void WorkerThread::DoReadyHook(ROAnything)
{
	if ( fRefreshAllocator ) {
		// StatTrace memory can still be on current storage because its code will be inserted in a subscope
		StatTrace(WorkerThread.DoReadyHook, "WorkerThread [" << GetName() << "] fAllocator->Refresh", Storage::Current());
		// reorganize allocator memory and hope that no more memory is allocated anymore
		if ( fAllocator ) {
			fAllocator->Refresh();
		}
	}
}

//---- WorkerPoolManager ------------------------------------------------
WorkerPoolManager::WorkerPoolManager(const char *name)
	: fPoolSize(0)
	, fCurrentParallelRequests(0)
	, fBlockRequestHandling(false)
	, fMutex(name)
	, fName(name)
	, fStatEvtHandler(0)
{
	StartTrace(WorkerPoolManager.Ctor);
}

WorkerPoolManager::~WorkerPoolManager()
{
	StartTrace(WorkerPoolManager.Dtor);
	if (fStatEvtHandler) {
		Anything statistic;
		fStatEvtHandler->Statistic(statistic);
		String strbuf;
		StringStream stream(strbuf);
		if (fName.Length()) {
			stream << "Statistics for [" << fName << "] ";
		}
		statistic.PrintOn(stream) << "\n";
		stream.flush();
		SysLog::WriteToStderr(strbuf);
		delete fStatEvtHandler;
	}
	fStatEvtHandler = 0;
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
	bool reallyterminated = Finis(5);	// nothing to do if yet uninitialized

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
		Trace("i am still not terminated, have to Terminate " << fPoolSize << " workers with timeout " << secs << "s");
		for (long i = 0; i < fPoolSize; ++i) {
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
	Assert(fPoolSize > 0);
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
	SysLog::Alert(logMessage);
	return -1;
}

int WorkerPoolManager::InitPool(bool usePoolStorage, long poolStorageSize, int numOfPoolBucketSizes, ROAnything roaWorkerArgs)
{
	StartTrace(WorkerPoolManager.InitPool);
	Trace("fPoolSize = " << fPoolSize);
	for (long i = 0; i < fPoolSize; ++i) {
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
			wt->Start(Storage::Global(), roaWorkerArgs);
		}
		Trace("Start done");
		wt->CheckState(Thread::eRunning);
		Trace("CheckState done");
	}
	if ( fStatEvtHandler ) {
		delete fStatEvtHandler;
	}
	//allocate hard for now; maybe later we can do it with a factory
	fStatEvtHandler = new WPMStatHandler(fPoolSize);
	return 0;
}

void WorkerPoolManager::Enter(ROAnything workload)
{
	// guard the entry to request handling
	// we're doing flow control on the main thread
	// causing it to wait for a request thread to
	// be available

	MutexEntry me(fMutex);
	me.Use();
	StartTrace(WorkerPoolManager.Enter);

	while ((fPoolSize <= fCurrentParallelRequests) || fBlockRequestHandling) {
		fCond.Wait( fMutex );		// release mutex and wait for condition
	}

	Assert(fPoolSize > fCurrentParallelRequests);
	Trace("I slipped past the Critical Region and there are " << fCurrentParallelRequests << " requests in work");

	// use the request param as hint to get the next free WorkerThread
	long request = workload["request"].AsLong(fCurrentParallelRequests + 1L);

	// find a worker object that can run this request
	WorkerThread *hr = FindNextRunnable(request);

	hr->SetWorking(workload);
}

void WorkerPoolManager::Update(Thread *pObserved, ROAnything aUpdateArgs)
{
	MutexEntry me(fMutex);
	me.Use();
	{
		StartTrace(WorkerPoolManager.Update);
		TraceAny(aUpdateArgs, "state event received");
		switch ( aUpdateArgs["RunningState"]["New"].AsLong(-1) ) {
			case Thread::eReady:
				--fCurrentParallelRequests;
				Trace("Requests still running: " << fCurrentParallelRequests);
				if (fStatEvtHandler) {
					fStatEvtHandler->HandleStatEvt(WPMStatHandler::eLeave);
				}
				break;

			case Thread::eWorking:
				++fCurrentParallelRequests;
				Trace("Requests now running: " << fCurrentParallelRequests);
				if (fStatEvtHandler) {
					fStatEvtHandler->HandleStatEvt(WPMStatHandler::eEnter);
				}
				break;

			default:
				break;
		}
	}
	fCond.BroadCast();
}

long WorkerPoolManager::ResourcesUsed()
{
	// accessor to current active requests
	MutexEntry me(fMutex);
	me.Use();
	return fCurrentParallelRequests;
}

void WorkerPoolManager::BlockRequests()
{
	// accessor to current active requests
	StatTrace(WorkerPoolManager.BlockRequests, "Blocking requests", Storage::Current());
	MutexEntry me(fMutex);
	me.Use();
	fBlockRequestHandling = true;
	fCond.Signal();
}

void WorkerPoolManager::UnblockRequests()
{
	// accessor to current active requests
	StatTrace(WorkerPoolManager.UnblockRequests, "Unblocking requests", Storage::Current());
	MutexEntry me(fMutex);
	me.Use();
	fBlockRequestHandling = false;
	fCond.Signal();
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
	MutexEntry me(fMutex);
	me.Use();	// to check for fCurrentParallelRequests we have to acquire the mutex
	DiffTimer aTimer(1);
	int msgCount = 0;
	// check for active requests
	while (	(fCurrentParallelRequests > 0) && (aTimer.Diff() < sec) ) {
		fMutex.Unlock();						// unlock so other threads can check back
		Thread::Wait(1);						// wait for 1 second
		fMutex.Lock();							// reacquire the mutex
		++msgCount;
		if ((msgCount % 5) == 0) {
			OStringStream os;
			os << "MaxSecToWait: " << setw(4) << sec <<
			   "  SecsWaited: "	 << setw(4) << msgCount <<
			   "  Pending requests: " <<  setw(6) <<  fCurrentParallelRequests << endl;
			SysLog::WriteToStderr(os.str());
		}
	}
	return (fCurrentParallelRequests <= 0);
}

WorkerThread *WorkerPoolManager::FindNextRunnable(long requestNr)
{
	if (requestNr < 0) {	// safeguard against negative numbers
		requestNr = 0;
	}

	unsigned long at = requestNr % fPoolSize;
	WorkerThread *hs = 0;

	// at least one should be runnable, since the semaphore prevents the main thread from entering
	// this code when no slot is empty;
	for ( hs = DoGetWorker(at); !hs->IsReady(); hs = DoGetWorker(++at % fPoolSize) ) ;

#ifdef LOW_TRACING
	SysLog::WriteToStderr("s{" << requestNr << "," << fCurrentParallelRequests << "}" << "\n");
#endif

	return hs;
}

void WorkerPoolManager::Statistic(Anything &item)
{
	if (fStatEvtHandler) {
		fStatEvtHandler->Statistic(item);
	}
}
