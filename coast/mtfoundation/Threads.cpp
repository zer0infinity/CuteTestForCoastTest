/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

//--- interface include ---------------------------------------------------------
#include "Threads.h"

//--- project modules used -----------------------------------------------------
#include "InitFinisManagerMTFoundation.h"

//--- standard modules used ----------------------------------------------------
#include "SysLog.h"
#include "DiffTimer.h"
#include "StringStream.h"
#include "TraceLocks.h"
#include "Dbg.h"

//--- c-library modules used ---------------------------------------------------

//#define  TRACE_LOCKS_IMPL 1

//---- AllocatorUnref ------------------------------------------------------------
AllocatorUnref::AllocatorUnref(Thread *t) : fThread(t)
{
}

AllocatorUnref::~AllocatorUnref()
{
	if (fThread) {
		Allocator *a = fThread->fAllocator;
		if (a) {
#ifdef MEM_DEBUG
			long lMaxCount = 3L;
			while ( ( a->CurrentlyAllocated() > 0L ) && ( --lMaxCount >= 0L ) ) {
				// give some 20ms slices to finish everything
				// it is in almost every case Trace-logging when enabled
				// normally only used in opt-wdbg or dbg mode
				Thread::Wait(0L, 20000000);
			}
#endif
			// now the allocator is no longer needed...
			MT_Storage::UnrefAllocator(a);
			fThread->fAllocator = 0;
		}
	}
}

//:subclasses may be defined to perform cleanup in thread specific storage
// while thread is still alive. CleanupHandlers are supposed to be singletons..
class EXPORTDECL_MTFOUNDATION MutexCountTableCleaner : public CleanupHandler
{
public:
	//:Constructor
	MutexCountTableCleaner() {}

	static MutexCountTableCleaner fgCleaner;

protected:
	//:method used to cleanup specific settings within
	// thread specific storage
	virtual bool DoCleanup();
};

//---- Thread ------------------------------------------------------------
THREADKEY Thread::fgCleanerKey = 0;

//:thread count variable
long Thread::fgNumOfThreads = 0;

//:guard of thread count
SimpleMutex *Thread::fgpNumOfThreadsMutex = NULL;

class EXPORTDECL_MTFOUNDATION ThreadInitializer : public InitFinisManagerMTFoundation
{
public:
	ThreadInitializer(unsigned int uiPriority)
		: InitFinisManagerMTFoundation(uiPriority) {
		IFMTrace("ThreadInitializer created\n");
	}

	~ThreadInitializer()
	{}

	virtual void DoInit() {
		IFMTrace("ThreadInitializer::DoInit\n");
		if (THRKEYCREATE(Thread::fgCleanerKey, 0)) {
			SysLog::Error("TlsAlloc of Thread::fgCleanerKey failed");
		}
		Thread::fgpNumOfThreadsMutex = new SimpleMutex("NumOfThreads");
	}

	virtual void DoFinis() {
		IFMTrace("ThreadInitializer::DoFinis\n");
		delete Thread::fgpNumOfThreadsMutex;
		Thread::fgpNumOfThreadsMutex = NULL;
		if (THRKEYDELETE(Thread::fgCleanerKey) != 0) {
			SysLog::Error("TlsFree of Thread::fgCleanerKey failed" );
		}
	}
};

static ThreadInitializer *psgThreadInitializer = new ThreadInitializer(1);

Thread::Thread(const char *name, bool daemon, bool detached, bool suspended, bool bound, Allocator *a)
	: NamedObject(name)
	, fAllocCleaner(this)
	, fAllocator(a)
	, fObserversMutex("ThreadObservers", (fAllocator) ? fAllocator : Storage::Global())
	, fObservers((fAllocator) ? fAllocator : Storage::Global())
	, fThreadId(0)
	, fParentThreadId(0)
	, fDaemon(daemon)
	, fDetached(detached)
	, fSuspended(suspended)
	, fBound(bound)
	, fStateMutex("ThreadState", (fAllocator) ? fAllocator : Storage::Global())
	, fRunningState(eReady)
	, fState(eCreated)
	, fSignature(0x0f0055AA)
	, fThreadName(name, strlen(name), (fAllocator) ? fAllocator : Storage::Global())
{
	StartTrace1(Thread.Constructor, "IntId: " << (long)GetId() << " ParId: " << fParentThreadId << " CallId: " << MyId() << " Name [" << GetName() << "]");
	if (fAllocator) {
		fAllocator->Ref();
	}
	fParentThreadId = MyId();
}

Thread::~Thread()
{
	StartTrace1(Thread.Destructor, "IntId: " << (long)GetId() << " ParId: " << fParentThreadId << " CallId: " << MyId() << " Name [" << GetName() << "]");
	THRDETACH();

	// doesn't really solve the problem if a subclass would do sthg in the hook method
	// but at least it will go to the state eTerminated
	Terminate();

	// info: destruction of fAllocCleaner will unregister
	//       the allocator (and eventually lead to its destruction)
	fSignature = 0xaaddeeff;
}

void Thread::SetName(const char *name)
{
	fThreadName = name;
}

bool Thread::GetName(String &str) const
{
	str = fThreadName;
	return (str.Length() > 0);
}

int Thread::Init(ROAnything args)
{
	StartTrace(Thread.Init);
	return 0;
}

bool Thread::Start(Allocator *pAllocator, ROAnything args)
{
	StartTrace1(Thread.Start, "IntId: " << (long)GetId() << " ParId: " << fParentThreadId << " CallId: " << MyId());
	// prec: Thread *MUST NOT* already run!
	if (GetState() == eTerminated) {
		SetState(eCreated);
	}
	if (GetState() < eStartRequested) {
		if (pAllocator || fAllocator) {
			if (pAllocator && fAllocator != pAllocator) {
				Allocator *oldAlloc = fAllocator;
				fAllocator = pAllocator;

				// CAUTION: Thread *MUST NOT* have any
				// instance variables that still use
				// the previous allocator!
				MT_Storage::UnrefAllocator(oldAlloc);	// previously used allocator is replaced
				MT_Storage::RefAllocator(fAllocator);
			}

			Assert(fAllocator);
			if ( SetState(eStartRequested, args) ) {
				bool *b = new bool[4];
				b[0] = fBound;
				b[1] = fDetached;
				b[2] = fDaemon;
				b[3] = fSuspended;
				bool ret = STARTTHREAD(this, b, &fThreadId, ThreadWrapper);
				if ( ret ) {
					SetState(eStarted, args);
					if ( fAllocator->GetId() == 0 ) {
						// use the address of this thread object as 'unique' id for the allocator
						// fThreadId is not good in the case the Thread gets reused!
						fAllocator->SetId((long)this);
					}
				}
				delete [] b;
				Trace("Start MyId(" << MyId() << ") started GetId( " << (long)fThreadId << ")" );
				return ret;
			}
		} else {
			SysLog::Error("No valid allocator supplied; no thread started");
		}
	} else {
		SysLog::Error("Thread::Start has already been called");
	}
	SetState(eTerminationRequested);
	return false;
}

void Thread::Exit(int )
{
	StartTrace(Thread.Exit);
	DELETETHREAD();
}

void Thread::AddObserver(ThreadObserver *to)
{
	StartTrace(Thread.AddObserver);
	SimpleMutexEntry me(fObserversMutex);
	me.Use();
	fObservers.Append(Anything((IFAObject *)to));
}

void Thread::NotifyAll(Anything evt)
{
	StartTrace(Thread.NotifyAll);
	TraceAny(evt, "event to notify");
	SimpleMutexEntry me(fObserversMutex);
	me.Use();

	for (long i = 0, sz = fObservers.GetSize(); i < sz; ++i) {
		ThreadObserver *to = (ThreadObserver *)fObservers[i].AsIFAObject(0);
		if (to) {
			to->Update(this, evt);
		} else {
			TraceAny(fObservers, "Somethings wrong");
		}
	}
}

void Thread::BroadCastEvent(Anything evt)
{
	StatTrace(Thread.BroadCastEvent, "notifying observers", Storage::Current());
	NotifyAll(evt);
	fStateCond.BroadCast();
	StatTrace(Thread.BroadCastEvent, "state broadcasted", Storage::Current());
}

bool Thread::CheckState(EThreadState state, long timeout, long nanotimeout)
{
	SimpleMutexEntry me(fStateMutex);
	me.Use();
	return IntCheckState(state, timeout, nanotimeout);
}

bool Thread::IntCheckState(EThreadState state, long timeout, long nanotimeout)
{
	if (fState >= state) {
		StatTrace(Thread.IntCheckState, "State already reached! IntId: " << (long)GetId() << " ParId: " << fParentThreadId << " CallId: " << MyId(), Storage::Current());
		return true;
	}

	if (timeout || nanotimeout) {
		DiffTimer dt;
		long militimeout = timeout * 1000L + nanotimeout / 1000000;
		while ( (fState < state) && ( dt.Diff() < (militimeout) )) {
			StatTrace(Thread.IntCheckState, "still waiting on fState(" << (long)fState << ")>=state(" << (long)state << ") IntId: " << (long)GetId() << " CallId: " << MyId(), Storage::Current());
			fStateCond.TimedWait(fStateMutex, timeout, nanotimeout);
		}
	} else {
		while (fState < state) {
			StatTrace(Thread.IntCheckState, "still waiting on fState(" << (long)fState << ")>=state(" << (long)state << ") IntId: " << (long)GetId() << " CallId: " << MyId(), Storage::Current());
			fStateCond.Wait(fStateMutex);
		}
	}
	StatTrace(Thread.IntCheckState, "fState(" << (long)fState << ")>=state(" << (long)state << ") is " << (( fState >= state ) ? "true" : "false") << " IntId: " << (long)GetId() << " ParId: " << fParentThreadId << " CallId: " << MyId(), Storage::Current());
	return ( fState >= state );
}

// a helper macro for debugging...
#define CHECKRUNNING_STATE_DBG(message)  StatTrace(Thread.CheckRunningState, (message) \
		  <<  " fRunningState: " << (long) fRunningState << " state: " << (long) state << " fState: " << (long) fState \
		  <<  " realDiff: " << realDiff << " compDiff: " << compDiff \
		  <<  " Seconds: " << seconds << " NanoSeconds: " << nanoSeconds \
		  <<  " Timeout given: " << (timeout*1000L), Storage::Current());

bool Thread::CheckRunningState(ERunningState state, long timeout, long nanotimeout)
{
	StatTrace(Thread.CheckRunningState, "waiting on fRunningState(" << (long)fRunningState << ")>=state(" << (long)state << ") IntId: " << (long)GetId() << " ParId: " << fParentThreadId << " CallId: " << MyId(), Storage::Current());
	SimpleMutexEntry me(fStateMutex);
	me.Use();

	if ( fState < eRunning ) {
		StatTrace(Thread.CheckRunningState, "not running yet, waiting until it is running. IntId: " << (long)GetId() << " ParId: " << fParentThreadId << " CallId: " << MyId(), Storage::Current());
		bool ret = IntCheckState(eRunning, timeout, nanotimeout);
		if (!ret) {
			StatTrace(Thread.CheckRunningState, "timeout while waiting. IntId: " << (long)GetId() << " ParId: " << fParentThreadId << " CallId: " << MyId(), Storage::Current());
			return ret;
		}
	}

	if ( fState > eRunning ) {
		StatTrace(Thread.CheckRunningState, "not running anymore. IntId: " << (long)GetId() << " ParId: " << fParentThreadId << " CallId: " << MyId(), Storage::Current());
		return false;
	}
	Assert(fState == eRunning);

	if ( fRunningState == state ) {
		StatTrace(Thread.CheckRunningState, "State already reached! IntId: " << (long)GetId() << " ParId: " << fParentThreadId << " CallId: " << MyId(), Storage::Current());
		return true;
	}

	if (timeout || nanotimeout) {
		// In below code we have to consider the fact that TimedWait(t) may return
		// earlier then the specified t value. This may be caused by signalling the
		// condition (and then we still want to wait our timeout amount of time)
		// presumed fRunningState != state
		// More likely: Even if the condition is not signalled there are situations where
		// the underlying system calls used in DiffTimer and SimpleCondition differ
		// slightly in their time measurement. We have to consider that and recalculate
		// the wait.

		DiffTimer dt;
		long realDiff =  timeout * 1000L + nanotimeout / 1000000;	// the time we like to wait, passed as argument to us
		long compDiff = 0;					// the adjusted time we really have to wait
		while (fRunningState != state && fState == eRunning) {
			compDiff = dt.Diff();	// get time passed since DiffTimer was instantiated
			compDiff =  realDiff - compDiff;	// new value to wait
			long nanoSeconds = 0;
			long int seconds = 0;
			if (compDiff <= 0) {
				CHECKRUNNING_STATE_DBG("Break.");
				break;
			}

			nanoSeconds = ((compDiff % 1000) * 1000000);
			seconds = (compDiff  / 1000);
			CHECKRUNNING_STATE_DBG("Before CondTimedWait.");
			fStateCond.TimedWait(fStateMutex, seconds, nanoSeconds);
		}
	} else {
		while (fRunningState != state && fState == eRunning) {
			fStateCond.Wait(fStateMutex);
		}
	}
	return ( fRunningState == state && fState == eRunning );
}

Thread::EThreadState Thread::GetState(bool trylock, EThreadState dfltState)
{
	if (!trylock) {
		SimpleMutexEntry me(fStateMutex);
		me.Use();
		StatTrace(Thread.GetState, "Locked access, fState:" << (long)fState << " IntId: " << (long)GetId() << " ParId: " << fParentThreadId << " CallId: " << MyId(), Storage::Current());
		return fState;
	}
	if (fStateMutex.TryLock()) {
		StatTrace(Thread.GetState, "Locked access, fState:" << (long)fState << " IntId: " << (long)GetId() << " ParId: " << fParentThreadId << " CallId: " << MyId(), Storage::Current());
		dfltState = fState;
		fStateMutex.Unlock();
	} else {
		StatTrace(Thread.GetState, "Lock failed, returning default:" << (long)dfltState << " IntId: " << (long)GetId() << " ParId: " << fParentThreadId << " CallId: " << MyId(), Storage::Current());
	}
	return dfltState;
}

bool Thread::SetState(EThreadState state, ROAnything args)
{
	SimpleMutexEntry me(fStateMutex);
	me.Use();
	if (state == fState + 1) {
		StatTrace(Thread.SetState, "state(" << (long)state << ")==fState(" << (long)fState << ")+1 " << (long)state << " IntId: " << (long)GetId() << " ParId: " << fParentThreadId << " CallId: " << MyId(), Storage::Current());
		return IntSetState(state, args);
	}
	if (state == eTerminationRequested && fState < eTerminationRequested) {
		StatTrace(Thread.SetState, "state(eTerminationRequested) && fState(" << (long)fState << ")<eTerminationRequested " << (long)state << " IntId: " << (long)GetId() << " ParId: " << fParentThreadId << " CallId: " << MyId(), Storage::Current());
		return IntSetState((fState == eCreated || fState == eStartRequested) ? eTerminated : state, args);
	}
	if (fState == eTerminated && state == eCreated) {
		StatTrace(Thread.SetState, "fState(eTerminated) && state(eCreated) => re-use Thread " << " IntId: " << (long)GetId() << " ParId: " << fParentThreadId << " CallId: " << MyId(), Storage::Current());
		return IntSetState(state, args);
	}
	if (state == eTerminatedRunMethod && MyId() == (long)GetId()) {
		StatTrace(Thread.SetState, "state==eTerminatedRunMethod && MyId==GetId IntId: " << (long)GetId() << " ParId: " << fParentThreadId << " CallId: " << MyId(), Storage::Current());
		return IntSetState(state, args);
	}
	if (state == eTerminated && MyId() == (long)GetId()) {
		StatTrace(Thread.SetState, "state==eTerminated && MyId==GetId IntId: " << (long)GetId() << " ParId: " << fParentThreadId << " CallId: " << MyId(), Storage::Current());
		bool bRet = IntSetState(state, args);
		// now we should be able to safely reset the fThreadId, needed only in case the Thread gets reused
		// not to trace a fThreadId which just finished to live
		fThreadId = 0;
		return bRet;
	}
	if (state == eTerminated) {
		StatTrace(Thread.SetState, "state==eTerminated => no-op! IntId: " << (long)GetId() << " ParId: " << fParentThreadId << " CallId: " << MyId(), Storage::Current());
	}
	return false;
}

bool Thread::IntSetState(EThreadState state, ROAnything args)
{
	bool bRet = false;
	if (CallStateHooks(state, args)) {
		Anything anyEvt;
		anyEvt["ThreadState"]["Old"] = (long)fState;
		anyEvt["ThreadState"]["New"] = (long)state;
		if ( !args.IsNull() ) {
			anyEvt["Args"] = args.DeepClone();
		}
		fState = state;
		BroadCastEvent(anyEvt);
		bRet = true;
	}
	return bRet;
}

bool Thread::CallStateHooks(EThreadState state, ROAnything args)
{
	switch (state) {
		case eStartRequested:
			StatTrace(Thread.CallStateHooks, "eStartRequested", Storage::Current());
			return DoStartRequestedHook(args);
		case eStarted:
			StatTrace(Thread.CallStateHooks, "eStarted", Storage::Current());
			DoStartedHook(args);
			break;
		case eTerminationRequested:
			StatTrace(Thread.CallStateHooks, "eTerminationRequested", Storage::Current());
			DoTerminationRequestHook(args);
			break;
		case eTerminatedRunMethod:
			StatTrace(Thread.CallStateHooks, "eTerminatedRunMethod", Storage::Current());
			DoTerminatedRunMethodHook();
			break;
		case eTerminated:
			StatTrace(Thread.CallStateHooks, "eTerminated", Storage::Global());
			DoTerminatedHook();
			break;
		default:
			;
	}
	return true;
}

bool Thread::DoStartRequestedHook(ROAnything)
{
	return true;
};
void Thread::DoStartedHook(ROAnything) {};
void Thread::DoTerminationRequestHook(ROAnything) {};
void Thread::DoTerminatedRunMethodHook() {};
void Thread::DoTerminatedHook() {};

bool Thread::SetRunningState(ERunningState state, ROAnything args)
{
	SimpleMutexEntry me(fStateMutex);
	me.Use();

	// allocate things used before and after call to CallRunningStateHooks() on Storage::Global() because Allocator could be refreshed during call

	StatTrace(Thread.SetRunningState, "-- entering -- CallId: " << MyId(), Storage::Current());

	if (fState != eRunning || fRunningState == state) {
		return false;
	}

	ERunningState oldState = fRunningState;
	fRunningState = state;
	// after this call we potentially have refreshed Allocator
	CallRunningStateHooks(state, args);
	{
		// scoping to force compiler not to move around automatic variables which could make strange things happen
		Anything anyEvt;
		anyEvt["RunningState"]["Old"] = (long)oldState;
		anyEvt["RunningState"]["New"] = (long)state;
		if ( !args.IsNull() ) {
			anyEvt["Args"] = args.DeepClone();
		}
		BroadCastEvent(anyEvt);
	}
	StatTrace(Thread.SetRunningState, "-- leaving --", Storage::Current());
	return true;
}

void Thread::CallRunningStateHooks(ERunningState state, ROAnything args)
{
	switch (state) {
		case eReady:
			StatTrace(Thread.CallRunningStateHooks, "eReady", Storage::Current());
			DoReadyHook(args);
			break;
		case eWorking:
			StatTrace(Thread.CallRunningStateHooks, "eWorking", Storage::Current());
			DoWorkingHook(args);
		default:
			;
	}
}

void Thread::DoReadyHook(ROAnything) {};
void Thread::DoWorkingHook(ROAnything) {};

bool Thread::IsReady()
{
	// assume that if we can't lock the mutex this WorkerThread object is not ready
	bool isReady = false;
	if (fStateMutex.TryLock()) {
		// now we have locked the mutex
		isReady = (fRunningState == eReady && fState == eRunning);
		fStateMutex.Unlock();
		StatTrace(Thread.IsReady, "ThrdId: " << (long)GetId() << " CallId: " << MyId() << (isReady ? " true" : " false"), Storage::Current());
	} else {
		StatTrace(Thread.IsReady, "ThrdId: " << (long)GetId() << " CallId: " << MyId() << (isReady ? " true" : " false") << " (StateMutex was not lockable)!", Storage::Current());
	}
	return isReady;
}

bool Thread::IsWorking()
{
	// assume that if we can't lock the mutex this WorkerThread object is working
	bool isWorking = true;
	if (fStateMutex.TryLock()) {
		// now we have locked the mutex
		isWorking = (fRunningState == eWorking && fState == eRunning);
		fStateMutex.Unlock();
		StatTrace(Thread.IsWorking, "ThrdId: " << (long)GetId() << " CallId: " << MyId() << (isWorking ? " true" : " false"), Storage::Current());
	} else {
		StatTrace(Thread.IsWorking, "ThrdId: " << (long)GetId() << " CallId: " << MyId() << (isWorking ? " true" : " false") << " (StateMutex was not lockable)!", Storage::Current());
	}
	return isWorking;
}

bool Thread::IsRunning()
{
	// assume that if we can't lock the mutex this Thread is not running
	bool isRunning = true;
	if (fStateMutex.TryLock()) {
		// now we have locked the mutex
		isRunning = (fState == eRunning);
		fStateMutex.Unlock();
		StatTrace(Thread.IsRunning, "fState:" << (long)fState << " ThrdId: " << (long)GetId() << " CallId: " << MyId() << (isRunning ? " true" : " false"), Storage::Current());
	} else {
		StatTrace(Thread.IsRunning, "fState:" << (long)fState << " ThrdId: " << (long)GetId() << " CallId: " << MyId() << (isRunning ? " true" : " false") << " (StateMutex was not lockable)!", Storage::Current());
	}
	return isRunning;
}

//:Try to set Ready state
bool Thread::SetReady(ROAnything args)
{
	StatTrace(Thread.SetReady, "ThrdId: " << (long)GetId() << " CallId: " << MyId(), Storage::Current());
	return SetRunningState(eReady, args);
}

//:Try to set Working state
bool Thread::SetWorking(ROAnything args)
{
	StatTrace(Thread.SetWorking, "ThrdId: " << (long)GetId() << " CallId: " << MyId(), Storage::Current());
	return SetRunningState(eWorking, args);
}

bool Thread::Terminate(long timeout, ROAnything args)
{
	StartTrace1(Thread.Terminate, "Timeout: " << timeout << " ThrdId: " << (long)GetId() << " CallId: " << MyId());

	SetState(eTerminationRequested, args);

	return CheckState(eTerminated, timeout);
}

void Thread::IntRun()
{
	// IntId might be wrong since thread starts in parallel with the forking thread
	StartTrace1(Thread.IntRun, "IntId: " << (long)GetId() << " ParId: " << fParentThreadId << " CallId: " << MyId());
	if (!CheckState(eStarted)) {
		// needs syslog messages
		if (!SetState(eTerminatedRunMethod)) {
			Trace("SetState(eTerminatedRunMethod) failed MyId(" << MyId() << ") GetId( " << (long)GetId() << ")" );
		}
		return;
	}
#ifdef TRACE_LOCKS_IMPL
	SysLog::WriteToStderr(String("Thr[") << fName << "]<" << Thread::MyId() << ">" << "\n");
#endif
	Trace("IntRun MyId(" << MyId() << ") GetId( " << (long)GetId() << ")" );
	// adds allocator reference to thread local store
	MT_Storage::RegisterThread(fAllocator);
	if (SetState(eRunning)) {
		{
			SimpleMutexEntry me(*fgpNumOfThreadsMutex);
			++fgNumOfThreads;
		}
		// do the real work
		Run();
		{
			SimpleMutexEntry me(*fgpNumOfThreadsMutex);
			--fgNumOfThreads;
		}
	} else {
		Trace("SetState(eRunning) failed MyId(" << MyId() << ") GetId( " << (long)GetId() << ")" );
	}
	if (!SetState(eTerminatedRunMethod)) {
		Trace("SetState(eTerminatedRunMethod) failed MyId(" << MyId() << ") GetId( " << (long)GetId() << ")");
	}
}

void Thread::Wait(long secs, long nanodelay)
{
	StatTrace(Thread.Wait, "secs:" << secs << " nano:" << nanodelay, Storage::Current());

	SimpleMutex m((const char *)0);	// FIXME: server shutdown may
	// interrupt the wait (avoid random
	// leak by not using a name)
	SimpleMutexEntry me(m);
	me.Use();

	SimpleCondition c;
	int ret = 0;
	do {
		ret = c.TimedWait(m, secs, nanodelay);
	} while ( ret != TIMEOUTCODE );
}

long Thread::MyId()
{
	return THRID();
}

long Thread::NumOfThreads()
{
	StartTrace(Thread.NumOfThreads);
	SimpleMutexEntry me(*fgpNumOfThreadsMutex);
	return fgNumOfThreads;
}

bool Thread::RegisterCleaner(CleanupHandler *handler)
{
	StartTrace1(Thread.RegisterCleaner, "CallId: " << MyId());
	Anything *handlerList = 0;
	if (!GETTLSDATA(fgCleanerKey, handlerList, Anything)) {
		handlerList = new MetaThing(Storage::Global());
		if (!SETTLSDATA(fgCleanerKey, handlerList)) {
			// failed: immediately destroy Anything to avoid leak..
			delete handlerList;
			SysLog::Error( "Thread::RegisterCleaner(CleanupHandler *) could not register cleanup handler" );
			return false;	// giving up..
		}
	}
	String adrKey;
	adrKey << (long)handler;
	(*handlerList)[adrKey] = (IFAObject *)handler;

	return true;
}

bool Thread::CleanupThreadStorage()
{
	StatTrace(Thread.CleanupThreadStorage, "CallId: " << MyId() << " entering", Storage::Global());

	bool bRet = true;
	Anything *handlerList = 0;
	if (GETTLSDATA(fgCleanerKey, handlerList, Anything) && handlerList) {
		for (long i = (handlerList->GetSize() - 1); i >= 0; --i) {
			CleanupHandler *handler = (CleanupHandler *)((*handlerList)[i].AsIFAObject(0));
			if (handler) {
				handler->Cleanup();
			}
		}
		delete handlerList;
		if (!SETTLSDATA(fgCleanerKey, 0)) {
			SysLog::Error( "Thread::CleanupThreadStorage(CleanupHandler *) could not cleanup handler" );
			bRet = false;
		}
	}

	if ( !( bRet = SetState(Thread::eTerminated) ) ) {
		SysLog::Warning( String("SetState(eTerminated) failed MyId(", -1, Storage::Global()) << (long)fThreadId << ")");
	}

	return bRet;
}

//---- Semaphore ------------------------------------------------------------

Semaphore::Semaphore(unsigned i_nCount)
{
	StartTrace(Semaphore.Semaphore);
	if ( !CREATESEMA(fSemaphore, i_nCount) ) {
		SysLog::Error("Sema create failed");
	}
}

Semaphore::~Semaphore()
{
	StartTrace(Semaphore.~Semaphore);
	if ( ! DELETESEMA(fSemaphore) ) {
		Trace("failed to close semaphore");
	}
}

bool Semaphore::Acquire()
{
	StatTrace(Semaphore.Acquire, "Sema&:" << (long)&fSemaphore << " ThrdId: " << Thread::MyId(), Storage::Current());
	return LOCKSEMA(fSemaphore);
}

bool Semaphore::TryAcquire()
{
	StatTrace(Semaphore.TryAcquire, "Sema&:" << (long)&fSemaphore << " ThrdId: " << Thread::MyId(), Storage::Current());
	return TRYSEMALOCK(fSemaphore);
}

#if !defined(WIN32) && ( !defined(__sun) || defined(USE_POSIX) )
int Semaphore::GetCount(int &count)
{
	StatTrace(Semaphore.GetCount, "Sema&:" << (long)&fSemaphore << " ThrdId: " << Thread::MyId(), Storage::Current());

	return GETSEMACOUNT(fSemaphore, count);
}
#endif

void Semaphore::Release()
{
	StatTrace(Semaphore.Release, "Sema&:" << (long)&fSemaphore << " ThrdId: " << Thread::MyId(), Storage::Current());
	if (!UNLOCKSEMA(fSemaphore)) {
		SysLog::Error("UNLOCKSEMA failed");
	}
}

SemaphoreEntry::SemaphoreEntry(Semaphore &i_aSemaphore)
	: fSemaphore (i_aSemaphore)
{
	fSemaphore.Acquire();
}

SemaphoreEntry::~SemaphoreEntry()
{
	fSemaphore.Release();
}

//---- SimpleMutex ------------------------------------------------------------

SimpleMutex::SimpleMutex(const char *name, Allocator *a)
	: fName(name, -1, a)
{
//	StartTrace1(SimpleMutex.SimpleMutex, fName);
	if ( !CREATEMUTEX(fMutex) ) {
		SysLog::Error("CREATEMUTEX failed");
	}
}

SimpleMutex::~SimpleMutex()
{
	StatTrace(SimpleMutex.~SimpleMutex, fName, Storage::Current());
	if ( !DELETEMUTEX(fMutex) ) {
		SysLog::Error(String("SimpleMutex<") << fName << ">: DELETEMUTEX failed");
	}
}

void SimpleMutex::Lock()
{
	StatTrace(SimpleMutex.Lock, fName, Storage::Current());
	if ( !LOCKMUTEX(fMutex) ) {
		SysLog::Error("LOCKMUTEX failed");
	}
}

void SimpleMutex::Unlock()
{
	StatTrace(SimpleMutex.Unlock, fName, Storage::Current());
	if ( !UNLOCKMUTEX(fMutex) ) {
		SysLog::Error("UNLOCKMUTEX failed");
	}
}

bool SimpleMutex::TryLock()
{
	StatTrace(SimpleMutex.TryLock, fName, Storage::Current());
	return TRYLOCK(fMutex);
}

//---- Mutex ------------------------------------------------------------
long Mutex::fgMutexId = 0;
SimpleMutex *Mutex::fgpMutexIdMutex = NULL;
THREADKEY Mutex::fgCountTableKey = 0;	// WIN32 defined it 0xFFFFFFFF !!

class EXPORTDECL_MTFOUNDATION MutexInitializer : public InitFinisManagerMTFoundation
{
public:
	MutexInitializer(unsigned int uiPriority)
		: InitFinisManagerMTFoundation(uiPriority) {
		IFMTrace("MutexInitializer created\n");
	}

	~MutexInitializer()
	{}

	virtual void DoInit() {
		IFMTrace("MutexInitializer::DoInit\n");
		Mutex::fgpMutexIdMutex = new SimpleMutex("MutexIdMutex");
		if (THRKEYCREATE(Mutex::fgCountTableKey, 0)) {
			SysLog::Error("TlsAlloc of Mutex::fgCountTableKey failed");
		}
	}

	virtual void DoFinis() {
		InitFinisManager::IFMTrace("MutexInitializer::DoFinis\n");
		if (THRKEYDELETE(Mutex::fgCountTableKey) != 0) {
			SysLog::Error("TlsFree of Mutex::fgCountTableKey failed" );
		}
		delete Mutex::fgpMutexIdMutex;
		Mutex::fgpMutexIdMutex = NULL;
	}
};
static MutexInitializer *psgMutexInitializer = new MutexInitializer(0);
//---- MutexCountTableCleaner ------------------------------------------------------------
MutexCountTableCleaner MutexCountTableCleaner::fgCleaner;

bool MutexCountTableCleaner::DoCleanup()
{
	StatTrace(MutexCountTableCleaner.DoCleanup, "ThrdId: " << Thread::MyId(), Storage::Global());
	MetaThing *countarray = 0;
	if (GETTLSDATA(Mutex::fgCountTableKey, countarray, MetaThing)) {
		if ((*countarray).GetSize()) {
			String strbuf(Storage::Global());
			OStringStream stream(strbuf);
			stream << "MutexCountTableCleaner::DoCleanup: ThrdId: " << Thread::MyId() << "\n  countarray still contained Mutex locking information!\n";
			(*countarray).Export(stream, 2);
			stream.flush();
			SysLog::WriteToStderr(strbuf);
		}
		delete countarray;
		countarray = 0;
		if (SETTLSDATA(Mutex::fgCountTableKey, countarray)) {
			return true;
		}
	}
	return false;
}

Mutex::Mutex(const char *name, Allocator *a)
	: fMutexId(a)
	, fLocker(-1)
	, fName(name, -1, a)
{
	{
		SimpleMutexEntry aMtx(*fgpMutexIdMutex);
		fMutexId.Append(++fgMutexId);
	}
	if ( !CREATEMUTEX(fMutex) ) {
		SysLog::Error("CREATEMUTEX failed");
	}
#ifdef TRACE_LOCKS_IMPL
	SysLog::WriteToStderr(String("[") << fName << " Id " << GetId() << "]<" << GetId() << "> A" << "\n");
#endif
}

Mutex::~Mutex()
{
#ifdef TRACE_LOCKS_IMPL
	SysLog::WriteToStderr(String("[") << fName << " Id " << GetId() << "]<" << GetId() << "> D" << "\n");
#endif
	if ( !DELETEMUTEX(fMutex) ) {
		SysLog::Error(String("Mutex<") << fName << ">: DELETEMUTEX failed");
	}
}

long Mutex::GetCount()
{
	MetaThing *countarray = 0;
	long lCount = 0L;
	GETTLSDATA(fgCountTableKey, countarray, MetaThing);
	if (countarray) {
		StatTraceAny(Mutex.GetCount, (*countarray), "countarray", Storage::Current());
		lCount = ((ROAnything)(*countarray))[fMutexId].AsLong(0L);
	}
	StatTrace(Mutex.GetCount, "Count:" << lCount << " ThrdId: " << Thread::MyId(), Storage::Current());
	return lCount;
}

bool Mutex::SetCount(long newCount)
{
	StatTrace(Mutex.SetCount, "ThrdId: " << Thread::MyId() << " newCount: " << newCount, Storage::Current());
	MetaThing *countarray = 0;
	GETTLSDATA(fgCountTableKey, countarray, MetaThing);

	if (!countarray && newCount > 0) {
		countarray = new MetaThing(Storage::Global());
		Thread::RegisterCleaner(&MutexCountTableCleaner::fgCleaner);
		if (!SETTLSDATA(fgCountTableKey, countarray)) {
			SysLog::Error("Mutex::SetCount: could not store recursive locking structure in TLS!");
			return false;
		}
	}
	if (countarray) {
		StatTraceAny(Mutex.SetCount, (*countarray), "countarray", Storage::Current());
		if (newCount <= 0) {
			(*countarray).Remove(fMutexId);
		} else {
			(*countarray)[fMutexId] = newCount;
		}
	}
	return true;
}

const String &Mutex::GetId()
{
#if defined(WIN32) && defined(TRACE_LOCK_UNLOCK)
	fMutexIdTL = (long)fMutex;
	return fMutexIdTL;
#else
	return fMutexId;
#endif
}

void Mutex::Lock()
{
	if (!TryLock()) {
		StatTrace(Mutex.Lock, "First try to lock failed, eg. not recursive lock -> 'hard'-locking now. Id: " <<  GetId() << " ThrdId: " << Thread::MyId(), Storage::Current());
		if ( !LOCKMUTEX(fMutex) ) {
			SysLog::Error("LOCKMUTEX failed");
		}
#ifdef TRACE_LOCKS_IMPL
		if (fLocker != -1) {
			SysLog::WriteToStderr(String("[") << fName << " Id " << GetId() << "]<" << GetCount() << "," << fLocker << "> overriding current locker!" << "\n");
		}
#endif
		fLocker = Thread::MyId();
		SetCount(1);
#ifdef TRACE_LOCKS_IMPL
		SysLog::WriteToStderr(String("[") << fName << " Id " << GetId() << "]<" << GetCount() << "," << fLocker << "> L" << "\n");
#endif
	} else {
		StatTrace(Mutex.Lock, "TryLock success, Id: " <<  GetId() << " ThrdId: " << Thread::MyId(), Storage::Current());
	}
	TRACE_LOCK_ACQUIRE(fName);
}

void Mutex::Unlock()
{
	if ( fLocker == Thread::MyId() ) {
		long actCount = GetCount() - 1;
		SetCount(actCount);
		StatTrace(Mutex.Unlock, "new lockcount:" << actCount << " Id: " <<  GetId() << " ThrdId: " << Thread::MyId(), Storage::Current());
#ifdef TRACE_LOCKS_IMPL
		SysLog::WriteToStderr(String("[") << fName << " Id " << GetId() << "]<" << GetCount() << "," << fLocker << "> TR" << "\n");
#endif
		if (actCount <= 0) {
#ifdef TRACE_LOCKS_IMPL
			SysLog::WriteToStderr(String("[") << fName << " Id " << GetId() << "]<" << GetCount() << "," << fLocker << "> R" << "\n");
#endif
			fLocker = -1;
			SetCount(0);
			if ( !UNLOCKMUTEX(fMutex) ) {
				SysLog::Error("UNLOCKMUTEX failed");
			}
		}
	} else {
		String logMsg("[");
		logMsg << fName << " Id " << GetId() << "]<" << GetCount() << "," << fLocker << ">";
		logMsg << " Locking error: calling unlock while not holding the lock";
		SysLog::Error(logMsg);
#ifdef TRACE_LOCKS_IMPL
		SysLog::WriteToStderr(logMsg << "\n");
#endif
	}
	TRACE_LOCK_RELEASE(fName);
}

bool Mutex::TryLock()
{
	bool hasLocked = TRYLOCK(fMutex);
	StatTrace(Mutex.TryLock, "Mutex " << (hasLocked ? "" : "not") << " locked Id: " <<  GetId() << " ThrdId: " << Thread::MyId() << " Locker: " << fLocker << " Count: " << GetCount(), Storage::Current());
	long lCount = 1;
#if defined(WIN32)
	if (hasLocked) {
		// WIN32 allows same thread to acquire the mutex more than once without deadlocking
		lCount = GetCount();
		if (lCount == 0) {
			// mutex acquired for the first time
			fLocker = Thread::MyId();
			++lCount;
			SetCount(lCount);
#ifdef TRACE_LOCKS_IMPL
			SysLog::WriteToStderr(String("[") << fName << " Id " << GetId() << "]<" << GetCount() << "," << fLocker << "> TL" << "\n");
#endif
		} else {
			// consecutive acquire
			++lCount;
			SetCount(lCount);
			UNLOCKMUTEX(fMutex);
#ifdef TRACE_LOCKS_IMPL
			SysLog::WriteToStderr(String("[") << fName << " Id " << GetId() << "]<" << GetCount() << "," << fLocker << "> TLA" << "\n");
#endif
		}
	} else if ( fLocker == Thread::MyId() ) {
#ifdef TRACE_LOCKS_IMPL
		SysLog::WriteToStderr(String("[") << fName << " Id " << GetId() << "]<" << GetCount() << "," << fLocker << "> TLA" << "\n");
#endif
		lCount += GetCount();
		SetCount(lCount);
		hasLocked = true;
	}
#else
	if (hasLocked) {
		fLocker = Thread::MyId();
		SetCount(lCount);
#ifdef TRACE_LOCKS_IMPL
		SysLog::WriteToStderr(String("[") << fName << " Id " << GetId() << "]<" << GetCount() << "," << fLocker << "> TL" << "\n");
#endif
		StatTrace(Mutex.TryLock, "first lock, count:" << lCount << " Id: " <<  GetId() << " ThrdId: " << Thread::MyId(), Storage::Current());
	} else if ( fLocker == Thread::MyId() ) {
		lCount += GetCount();
		SetCount(lCount);
		hasLocked = true;
		StatTrace(Mutex.TryLock, "recursive lock, count:" << lCount << " Id: " <<  GetId() << " ThrdId: " << Thread::MyId(), Storage::Current());
#ifdef TRACE_LOCKS_IMPL
		SysLog::WriteToStderr(String("[") << fName << " Id " << GetId() << "]<" << GetCount() << "," << fLocker << "> TLA" << "\n");
#endif
	}
#endif
#ifdef TRACE_LOCKS
	if (hasLocked) {
		TRACE_LOCK_ACQUIRE(fName);
	}
#endif
	return hasLocked;
}

void Mutex::ReleaseForWait()
{
	StartTrace1(Mutex.ReleaseForWait, "Id: " <<  GetId() << " ThrdId: " << Thread::MyId());
#ifdef TRACE_LOCKS_IMPL
	SysLog::WriteToStderr(String("[") << fName << " Id " << GetId() << "]<" << GetCount() << "," << fLocker << "> RFW" << "\n");
#endif
	fLocker = -1;
}

void Mutex::AcquireAfterWait()
{
	StartTrace1(Mutex.AcquireAfterWait, "Id: " <<  GetId() << " ThrdId: " << Thread::MyId() << " fLocker: " << fLocker);
	fLocker = Thread::MyId();
#ifdef TRACE_LOCKS_IMPL
	SysLog::WriteToStderr(String("[") << fName << " Id " << GetId() << "]<" << GetCount() << "," << fLocker << "> AAW" << "\n");
#endif
}

//---- RWLock ------------------------------------------------------------

RWLock::RWLock()
{
	CREATERWLOCK(fLock);
}

RWLock::RWLock(const char *name, Allocator *a)
	: fName(name, -1, a)
{
	CREATERWLOCK(fLock);
}

RWLock::RWLock(const RWLock &)
{
	// private no op -> don't use this;
}

void RWLock::operator=(const RWLock &)
{
	// private no op -> don't use this;
}

RWLock::~RWLock()
{
	DELETERWLOCK(fLock);
}

void RWLock::Lock(bool reading) const
{
	LOCKRWLOCK(fLock, reading);
	TRACE_LOCK_ACQUIRE(fName);
}

void RWLock::Unlock(bool reading) const
{
	UNLOCKRWLOCK(fLock, reading);
	TRACE_LOCK_RELEASE(fName);
}

bool RWLock::TryLock(bool reading) const
{
	bool hasLocked = TRYRWLOCK(fLock, reading);
#ifdef TRACE_LOCKS
	if (hasLocked) {
		TRACE_LOCK_ACQUIRE(fName);
	}
#endif
	return hasLocked;
}

//---- SimpleCondition ------------------------------------------------

SimpleCondition::SimpleCondition()
{
	StartTrace(SimpleCondition.SimpleCondition);
	if ( !CREATECOND(fSimpleCondition) ) {
		SysLog::Error("CREATECOND failed");
	}
}

SimpleCondition::~SimpleCondition()
{
	StartTrace(SimpleCondition.~SimpleCondition);
	if ( !DELETECOND(fSimpleCondition) ) {
		SysLog::Error("DELETECOND failed");
	}
}

int SimpleCondition::Wait(SimpleMutex &m)
{
	StatTrace(SimpleCondition.Wait, "releasing SimpleMutex[" << m.fName << "] CallId:" << Thread::MyId(), Storage::Current());
	if (!CONDWAIT(fSimpleCondition, m.GetInternal())) {
		SysLog::Error("CONDWAIT failed");
	}
	StatTrace(SimpleCondition.Wait, "reacquired SimpleMutex[" << m.fName << "] CallId:" << Thread::MyId(), Storage::Current());
	return 0;
}

int SimpleCondition::TimedWait(SimpleMutex &m, long secs, long nanosecs)
{
	StatTrace(SimpleCondition.TimedWait, "releasing SimpleMutex[" << m.fName << "] secs:" << secs << " nano:" << nanosecs << " CallId:" << Thread::MyId(), Storage::Current());
	int iRet = CONDTIMEDWAIT(fSimpleCondition, m.GetInternal(), secs, nanosecs);
	StatTrace(SimpleCondition.TimedWait, "reacquired SimpleMutex[" << m.fName << "] CallId:" << Thread::MyId(), Storage::Current());
	return iRet;
}

int SimpleCondition::Signal()
{
	StatTrace(SimpleCondition.Signal, "", Storage::Current());
	return SIGNALCOND(fSimpleCondition);
}

int SimpleCondition::BroadCast()
{
	StatTrace(SimpleCondition.BroadCast, "", Storage::Current());
	return BROADCASTCOND(fSimpleCondition);
}

long SimpleCondition::GetId()
{
	long lId = (long)GetInternal();
	StatTrace(SimpleCondition.GetId, lId, Storage::Current());
	return lId;
}

//---- Condition ------------------------------------------------
Condition::Condition()
{
	if ( !CREATECOND(fCondition) ) {
		SysLog::Error("CREATECOND failed");
	}
}

Condition::~Condition()
{
	if ( !DELETECOND(fCondition) ) {
		SysLog::Error("DELETECOND failed");
	}
}

int Condition::Wait(Mutex &m)
{
	StatTrace(Condition.Wait, "releasing Mutex[" << m.fName << "] Id: " << GetId() << " CallId:" << Thread::MyId(), Storage::Current());
	m.ReleaseForWait();
	if ( !CONDWAIT(fCondition, m.GetInternal()) ) {
		SysLog::Error("CONDWAIT failed");
	}
	StatTrace(Condition.Wait, "reacquired Mutex[" << m.fName << "] Id: " << GetId() << " CallId:" << Thread::MyId(), Storage::Current());
	m.AcquireAfterWait();
	return 0;
}

int Condition::TimedWait(Mutex &m, long secs, long nanosecs)
{
	StatTrace(Condition.TimedWait, "releasing Mutex[" << m.fName << "] secs:" << secs << " nano:" << nanosecs << " Id: " << GetId() << " CallId:" << Thread::MyId(), Storage::Current());
	m.ReleaseForWait();
	int ret = CONDTIMEDWAIT(fCondition, m.GetInternal(), secs, nanosecs);
	StatTrace(Condition.Wait, "reacquired Mutex[" << m.fName << "] Id: " << GetId() << " CallId:" << Thread::MyId(), Storage::Current());
	m.AcquireAfterWait();
	return ret;
}

int Condition::Signal()
{
	StatTrace(Condition.Signal, "Id: " << GetId() << " ThrdId: " << Thread::MyId(), Storage::Current());
	return SIGNALCOND(fCondition);
}

int Condition::BroadCast()
{
	StatTrace(Condition.BroadCast, "Id: " << GetId() << " ThrdId: " << Thread::MyId(), Storage::Current());
	return BROADCASTCOND(fCondition);
}

long Condition::GetId()
{
	long lId = (long)GetInternal();
	StatTrace(Condition.GetId, lId, Storage::Current());
	return lId;
}
