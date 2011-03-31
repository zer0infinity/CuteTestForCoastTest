/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#include "Threads.h"
#include "InitFinisManagerMTFoundation.h"
#include "SystemLog.h"
#include "DiffTimer.h"
#include "StringStream.h"
#include "TraceLocks.h"
#include "MT_Storage.h"
#include <cstring>
#if !defined(WIN32)
#include <errno.h>
#endif

//#define TRACE_LOCKS_IMPL 1
//#define POOL_STARTEDHOOK 1

//---- AllocatorUnref ------------------------------------------------------------
AllocatorUnref::AllocatorUnref(Thread *t)
	: fThread(t)
{
}

AllocatorUnref::~AllocatorUnref()
{
	if ( fThread ) {
		Allocator *a = fThread->fAllocator;
		if ( a ) {
			if ( a != Coast::Storage::Global() && ( a->CurrentlyAllocated() > 0L ) ) {
				long lMaxCount = 3L;
				while ( ( a->CurrentlyAllocated() > 0L ) && ( --lMaxCount >= 0L ) ) {
					// give some 20ms slices to finish everything
					// it is in almost every case Trace-logging when enabled
					// normally only used in opt-wdbg or dbg mode
					SystemLog::WriteToStderr("#", 1);
					Thread::Wait(0L, 20000000);
				}
			}
			// now the allocator is no longer needed...
			MT_Storage::UnrefAllocator(a);
			fThread->fAllocator = 0;
		}
	}
}

//:subclasses may be defined to perform cleanup in thread specific storage
// while thread is still alive. CleanupHandlers are supposed to be singletons..
class MutexCountTableCleaner : public CleanupHandler
{
public:
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

class ThreadInitializer : public InitFinisManagerMTFoundation
{
public:
	ThreadInitializer(unsigned int uiPriority)
		: InitFinisManagerMTFoundation(uiPriority) {
		IFMTrace("ThreadInitializer created\n");
	}

	virtual void DoInit() {
		IFMTrace("ThreadInitializer::DoInit\n");
		if (THRKEYCREATE(Thread::fgCleanerKey, 0)) {
			SystemLog::Error("TlsAlloc of Thread::fgCleanerKey failed");
		}
		Thread::fgpNumOfThreadsMutex = new SimpleMutex("NumOfThreads", Coast::Storage::Global());
	}

	virtual void DoFinis() {
		IFMTrace("ThreadInitializer::DoFinis\n");
		delete Thread::fgpNumOfThreadsMutex;
		Thread::fgpNumOfThreadsMutex = NULL;
		if (THRKEYDELETE(Thread::fgCleanerKey) != 0) {
			SystemLog::Error("TlsFree of Thread::fgCleanerKey failed" );
		}
	}
};

static ThreadInitializer *psgThreadInitializer = new ThreadInitializer(15);

#if defined(__APPLE__)	//!@FIXME check if this is still needed with most current version of framework
// notice: never delete this class or its internal structure!
// check with the base class FinalCleaner for understanding the sequence of deletion
// the CleanupInitializer instance will be deleted from within FinalCleaner::~FinalCleaner
class CleanupAllocator
{
private:
	static ThreadInitializer *globalCleanupInitializer;
public:
	CleanupAllocator() {
		SystemLog::Info("----CREATED CleanupAllocator----");
	}

	~CleanupAllocator() {
		SystemLog::Info("----DESTROYED CleanupAllocator----");
	}

	static void initializeCleanupAllocator() {
		if (!globalCleanupInitializer) {
			globalCleanupInitializer = new ThreadInitializer();
		}
	}
};

CleanupInitializer *CleanupAllocator::globalCleanupInitializer = 0;
static CleanupAllocator fgInitKey;
#endif

Thread::Thread(const char *name, bool daemon, bool detached, bool suspended, bool bound, Allocator *a)
	: NamedObject()
	, tObservableBase(name, a)
	, fAllocator(a)
	, fAllocCleaner(this)
	, fThreadId(0)
	, fParentThreadId(0)
	, fDaemon(daemon)
	, fDetached(detached)
	, fSuspended(suspended)
	, fBound(bound)
	, fStateMutex("ThreadStateMutex", (fAllocator) ? fAllocator : Coast::Storage::Global())
	, fStateCond()
	, fRunningState(eReady)
	, fState(eCreated)
	, fSignature(0xaaddeeff)
	, fThreadName(name, strlen(name), (fAllocator) ? fAllocator : Coast::Storage::Global())
	, fanyArgTmp( Coast::Storage::Global() )
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

	// doesn't really solve the problem if a subclass would do sthg in the hook method
	// but at least it will go to the state eTerminated
	if ( ( fState < eTerminated ) && !Terminate() ) {
		SYSERROR("Terminate() failed");
	}

	// check if and who is still waiting on the state mutex if possible
	bool bDidLock( fStateMutex.TryLock() );
	long lMaxCount = 10L;
	while ( ( !bDidLock || IsAlive() ) && ( --lMaxCount >= 0L ) ) {
		// aha, someone is still locking it!
		// try to give some time and then terminate finally even the mutex was not locked
		// give some 20ms slices to finish everything
		if ( bDidLock ) {
			fStateCond.TimedWait(fStateMutex, 0, 10000000);
			fStateMutex.Unlock();
			SystemLog::WriteToStderr("*", 1);
		} else {
			Thread::Wait(0L, 20000000);
			SystemLog::WriteToStderr("~", 1);
		}
		bDidLock = fStateMutex.TryLock();
	}
	if ( bDidLock ) {
		fStateMutex.Unlock();
	} else {
		SystemLog::Error(String("Thread::~Thread<").Append(GetName()).Append(">: fStateMutex was still in use!"));
	}
	if ( IsAlive() ) {
		SystemLog::Error(String("Thread::~Thread<").Append(GetName()).Append(">: ThreadId: ").Append( (long)GetId() ).Append(" did not terminate properly (state: ").Append((long)fState).Append("), still destructing it, you should check this!"));
	}
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
	EThreadState aState( GetState( false, eStarted ) );
	if ( ( aState > eCreated ) && ( aState < eTerminated ) ) {
		SYSERROR("ThreadId: " << (long)GetId() << " is still alive, eg. did not detach from system thread yet, exiting!");
	} else {
		// reset alive signature to allow re-use of object
		fSignature = 0x0f0055AA;
		if ( GetState() == eTerminated ) {
			SetState(eCreated);
		}
		if ( GetState() < eStartRequested ) {
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
						if ( fAllocator->GetId() == 0 ) {
							fAllocator->SetId((long)fThreadId);
						}
						SetState(eStartInProgress, args);
#if !defined(POOL_STARTEDHOOK)
						SetState(eStarted, args); //XXX moved to IntRun
#endif
					} else {
						SYSERROR("could not start and attach system thread");
						fThreadId = 0;
						fSignature = 0xaaddeeff;
					}
					delete [] b;
					Trace("Start MyId(" << MyId() << ") started GetId( " << (long)fThreadId << ")" );
					return ret;
				}
			} else {
				SystemLog::Error("No valid allocator supplied; no thread started");
			}
		} else {
			SystemLog::Error("Thread::Start has already been called");
		}
		SetState(eTerminationRequested);
	}
	return false;
}

void Thread::Exit(int)
{
	// now we should be able to safely reset the fThreadId, needed only in case the Thread gets reused
	// not to trace a fThreadId which just finished to live
	// IMPORTANT: these values must be set BEFORE deleting and detaching the system thread
	// -> if not, the values will not get updated in 'this' object!

	// schedule cleanup of thread resources
	int iDetachCode( 0 );
	// even though pthread_detach will not work on non-joined threads (EINVAL), we will call the method
	// to cleanup resources on solaris/linux as it seems to do what expected
	if ( ( ( iDetachCode = THRDETACH( fThreadId ) ) != 0 ) && ( iDetachCode != EINVAL ) ) {
		SYSERROR("pthread_detach failed: " << SystemLog::SysErrorMsg( iDetachCode ));
	}
	StatTrace(Thread.Exit, "destroying system thread object id: " << GetId(), Coast::Storage::Global());
	// reset alive flag, must be set in Start() again!
	fSignature = 0xaaddeeff;
	fThreadId = 0;
	SetState( Thread::eTerminated );
}

void Thread::BroadCastEvent(Anything evt)
{
	StatTrace(Thread.BroadCastEvent, "notifying observers", Coast::Storage::Current());
	NotifyAll(evt);
	fStateCond.BroadCast();
	StatTrace(Thread.BroadCastEvent, "state broadcasted", Coast::Storage::Current());
}

bool Thread::CheckState(EThreadState state, long timeout, long nanotimeout)
{
	LockUnlockEntry me(fStateMutex);
	return IntCheckState(state, timeout, nanotimeout);
}

bool Thread::IntCheckState(EThreadState state, long timeout, long nanotimeout)
{
	if ( fState == state ) {
		StatTrace(Thread.IntCheckState, "State already reached! fState(" << (long)fState << ")==state(" << (long)state << ") IntId: " << (long)GetId() << " ParId: " << fParentThreadId << " CallId: " << MyId(), Coast::Storage::Current());
		return true;
	}
	if ( fState > state ) {
		StatTrace(Thread.IntCheckState, "State passed! fState(" << (long)fState << ")>state(" << (long)state << ") IntId: " << (long)GetId() << " ParId: " << fParentThreadId << " CallId: " << MyId(), Coast::Storage::Current());
		return false;
	}
	if ( timeout || nanotimeout ) {
		DiffTimer dt;
		long militimeout = timeout * 1000L + nanotimeout / 1000000;
		while ( ( fState < state ) && ( dt.Diff() < (militimeout) ) ) {
			StatTrace(Thread.IntCheckState, "still waiting on fState(" << (long)fState << ")==state(" << (long)state << ") IntId: " << (long)GetId() << " CallId: " << MyId(), Coast::Storage::Current());
			fStateCond.TimedWait(fStateMutex, timeout, nanotimeout);
		}
	} else {
		while ( fState < state ) {
			StatTrace(Thread.IntCheckState, "still waiting on fState(" << (long)fState << ")==state(" << (long)state << ") IntId: " << (long)GetId() << " CallId: " << MyId(), Coast::Storage::Current());
			fStateCond.Wait(fStateMutex);
		}
	}
	StatTrace(Thread.IntCheckState, "fState(" << (long)fState << ")==state(" << (long)state << ") is " << (( fState == state ) ? "true" : "false") << " IntId: " << (long)GetId() << " ParId: " << fParentThreadId << " CallId: " << MyId(), Coast::Storage::Current());
	return ( fState == state );
}

// a helper macro for debugging...
#define CHECKRUNNING_STATE_DBG(message)  StatTrace(Thread.CheckRunningState, (message) \
		  <<  " fRunningState: " << (long) fRunningState << " state: " << (long) state << " fState: " << (long) fState \
		  <<  " realDiff: " << realDiff << " compDiff: " << compDiff \
		  <<  " Seconds: " << seconds << " NanoSeconds: " << nanoSeconds \
		  <<  " Timeout given: " << (timeout*1000L), Coast::Storage::Current());

bool Thread::CheckRunningState(ERunningState state, long timeout, long nanotimeout)
{
	LockUnlockEntry me(fStateMutex);
	return IntCheckRunningState(state, timeout, nanotimeout);
}
bool Thread::IntCheckRunningState(ERunningState state, long timeout, long nanotimeout)
{
	StatTrace(Thread.IntCheckRunningState, "waiting on fRunningState(" << (long)fRunningState << ")>=state(" << (long)state << ") IntId: " << (long)GetId() << " ParId: " << fParentThreadId << " CallId: " << MyId(), Coast::Storage::Current());
	if ( fState < eRunning ) {
		StatTrace(Thread.CheckRunningState, "not running yet, waiting until it is running. IntId: " << (long)GetId() << " ParId: " << fParentThreadId << " CallId: " << MyId(), Coast::Storage::Current());
		bool ret = IntCheckState(eRunning, timeout, nanotimeout);
		if (!ret) {
			StatTrace(Thread.CheckRunningState, "timeout while waiting. IntId: " << (long)GetId() << " ParId: " << fParentThreadId << " CallId: " << MyId(), Coast::Storage::Current());
			return ret;
		}
	}

	if ( fState > eRunning ) {
		StatTrace(Thread.CheckRunningState, "not running anymore. IntId: " << (long)GetId() << " ParId: " << fParentThreadId << " CallId: " << MyId(), Coast::Storage::Current());
		return false;
	}
	Assert(fState == eRunning);

	if ( fRunningState == state ) {
		StatTrace(Thread.CheckRunningState, "State already reached! IntId: " << (long)GetId() << " ParId: " << fParentThreadId << " CallId: " << MyId(), Coast::Storage::Current());
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
		while ( ( fRunningState != state ) && ( fState == eRunning ) ) {
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
		while ( ( fRunningState != state ) && ( fState == eRunning ) ) {
			fStateCond.Wait(fStateMutex);
		}
	}
	return ( fRunningState == state && fState == eRunning );
}

Thread::EThreadState Thread::GetState(bool trylock, EThreadState dfltState)
{
	if (!trylock) {
		LockUnlockEntry me(fStateMutex);
		StatTrace(Thread.GetState, "Locked access, fState:" << (long)fState << " IntId: " << (long)GetId() << " ParId: " << fParentThreadId << " CallId: " << MyId(), Coast::Storage::Current());
		return fState;
	}
	if (fStateMutex.TryLock()) {
		StatTrace(Thread.GetState, "Locked access, fState:" << (long)fState << " IntId: " << (long)GetId() << " ParId: " << fParentThreadId << " CallId: " << MyId(), Coast::Storage::Current());
		dfltState = fState;
		fStateMutex.Unlock();
	} else {
		StatTrace(Thread.GetState, "Lock failed, returning default:" << (long)dfltState << " IntId: " << (long)GetId() << " ParId: " << fParentThreadId << " CallId: " << MyId(), Coast::Storage::Current());
	}
	return dfltState;
}

bool Thread::SetState(EThreadState state, ROAnything args)
{
	bool bRetCode( false );
	LockUnlockEntry me(fStateMutex);
	if (state == fState + 1) {
		StatTrace(Thread.SetState, "state(" << (long)state << ")==fState(" << (long)fState << ")+1 " << (long)state << " IntId: " << (long)GetId() << " ParId: " << fParentThreadId << " CallId: " << MyId(), Coast::Storage::Current());
		bRetCode = IntSetState(state, args);
	} else if ( ( state == eTerminationRequested ) && ( fState < eTerminationRequested ) ) {
		StatTrace(Thread.SetState, "state(eTerminationRequested) && fState(" << (long)fState << ")<eTerminationRequested " << (long)state << " IntId: " << (long)GetId() << " ParId: " << fParentThreadId << " CallId: " << MyId(), Coast::Storage::Current());
		bRetCode = IntSetState( ( ( fState == eCreated ) || ( fState == eStartRequested ) ) ? eTerminated : state, args);
	} else if ( ( fState == eTerminated ) && ( state == eCreated ) ) {
		StatTrace(Thread.SetState, "fState(eTerminated) && state(eCreated) => re-use Thread " << " IntId: " << (long)GetId() << " ParId: " << fParentThreadId << " CallId: " << MyId(), Coast::Storage::Current());
		bRetCode = IntSetState(state, args);
	} else if ( ( state == eTerminatedRunMethod ) && ( MyId() == (long)GetId() ) ) {
		StatTrace(Thread.SetState, "state==eTerminatedRunMethod && MyId==GetId IntId: " << (long)GetId() << " ParId: " << fParentThreadId << " CallId: " << MyId(), Coast::Storage::Current());
		bRetCode = IntSetState(state, args);
	} else if ( ( state == eTerminated ) && ( MyId() == (long)GetId() ) ) {
		StatTrace(Thread.SetState, "state==eTerminated && MyId==GetId IntId: " << (long)GetId() << " ParId: " << fParentThreadId << " CallId: " << MyId(), Coast::Storage::Current());
		bRetCode = IntSetState(state, args);
	} else if ( state == eTerminated ) {
		StatTrace(Thread.SetState, "state==eTerminated => no-op! IntId: " << (long)GetId() << " ParId: " << fParentThreadId << " CallId: " << MyId(), Coast::Storage::Current());
	}
	return bRetCode;
}

bool Thread::IntSetState(EThreadState state, ROAnything args)
{
	bool bRet = false;
	if ( CallStateHooks(state, args) ) {
		Anything anyEvt;
		anyEvt["this"] = static_cast<IFAObject*>(this);
		anyEvt["ParentThreadId"] = (long)fParentThreadId;
		anyEvt["ThreadId"] = (long)GetId();
		anyEvt["ThreadState"]["Old"] = (long)fState;
		anyEvt["ThreadState"]["New"] = (long)state;
		if ( !args.IsNull() ) {
			anyEvt["Args"] = args.DeepClone();
		}
		fState = state;
		bRet = true;
		BroadCastEvent(anyEvt);
	}
	return bRet;
}

bool Thread::CallStateHooks(EThreadState state, ROAnything args)
{
	switch (state) {
		case eStartRequested:
			StatTrace(Thread.CallStateHooks, "eStartRequested", Coast::Storage::Current());
			return DoStartRequestedHook(args);
		case eStartInProgress:
			StatTrace(Thread.CallStateHooks, "eStartInProgress", Coast::Storage::Current());
			fanyArgTmp = args.DeepClone(Coast::Storage::Global());
			break;
		case eStarted:
			StatTrace(Thread.CallStateHooks, "eStarted", Coast::Storage::Current());
			DoStartedHook(args);
			break;
		case eRunning:
			StatTrace(Thread.CallStateHooks, "eRunning", Coast::Storage::Current());
			DoRunningHook(args);
			break;
		case eTerminationRequested:
			StatTrace(Thread.CallStateHooks, "eTerminationRequested", Coast::Storage::Current());
			DoTerminationRequestHook(args);
			break;
		case eTerminatedRunMethod:
			StatTrace(Thread.CallStateHooks, "eTerminatedRunMethod", Coast::Storage::Current());
			DoTerminatedRunMethodHook();
			break;
		case eTerminated:
			StatTrace(Thread.CallStateHooks, "eTerminated", Coast::Storage::Global());
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
void Thread::DoRunningHook(ROAnything) {};
void Thread::DoTerminationRequestHook(ROAnything) {};
void Thread::DoTerminatedRunMethodHook() {};
void Thread::DoTerminatedHook() {};

bool Thread::SetRunningState(ERunningState state, ROAnything args)
{
	bool bRetCode( false );
	LockUnlockEntry me(fStateMutex);
	// allocate things used before and after call to CallRunningStateHooks() on Coast::Storage::Global() because Allocator could be refreshed during call
	StatTrace(Thread.SetRunningState, "-- entering -- CallId: " << MyId(), Coast::Storage::Current());

	if ( ( fState != eRunning ) || ( fRunningState == state ) ) {
		bRetCode = false;
	} else {
		ERunningState oldState = fRunningState;
		fRunningState = state;
		// after this call we potentially have refreshed Allocator
		CallRunningStateHooks(state, args);
		{
			// scoping to force compiler not to move around automatic variables which could make strange things happen
			Anything anyEvt;
			anyEvt["ThreadId"] = (long)GetId();
			anyEvt["RunningState"]["Old"] = (long)oldState;
			anyEvt["RunningState"]["New"] = (long)state;
			if ( !args.IsNull() ) {
				anyEvt["Args"] = args.DeepClone();
			}
			bRetCode = true;
			BroadCastEvent(anyEvt);
		}
	}
	StatTrace(Thread.SetRunningState, "-- leaving --", Coast::Storage::Current());
	return bRetCode;
}

void Thread::CallRunningStateHooks(ERunningState state, ROAnything args)
{
	switch (state) {
		case eReady:
			StatTrace(Thread.CallRunningStateHooks, "eReady", Coast::Storage::Current());
			DoReadyHook(args);
			break;
		case eWorking:
			StatTrace(Thread.CallRunningStateHooks, "eWorking", Coast::Storage::Current());
			DoWorkingHook(args);
		default:
			;
	}
}

void Thread::DoReadyHook(ROAnything) {};
void Thread::DoWorkingHook(ROAnything) {};

bool Thread::IsReady( bool &bIsReady )
{
	bool bCouldLock( false );
	if ( IsAlive() ) {
		if ( fStateMutex.TryLock() ) {
			// now we have locked the mutex
			bIsReady = ( ( fState == eRunning ) && ( fRunningState == eReady ) );
			fStateMutex.Unlock();
			bCouldLock = true;
			StatTrace(Thread.IsReady, "ThrdId: " << (long)GetId() << " CallId: " << MyId() << (bIsReady ? " true" : " false"), Coast::Storage::Current());
		} else {
			StatTrace(Thread.IsReady, "ThrdId: " << (long)GetId() << " CallId: " << MyId() << (bIsReady ? " true" : " false") << " (StateMutex was not lockable)!", Coast::Storage::Current());
		}
	} else {
		bIsReady = false;
	}
	return bCouldLock;
}

bool Thread::IsWorking( bool &bIsWorking )
{
	bool bCouldLock( false );
	if ( IsAlive() ) {
		if ( fStateMutex.TryLock() ) {
			// now we have locked the mutex
			bIsWorking = ( ( fState == eRunning ) && ( fRunningState == eWorking ) );
			fStateMutex.Unlock();
			bCouldLock = true;
			StatTrace(Thread.IsWorking, "ThrdId: " << (long)GetId() << " CallId: " << MyId() << (bIsWorking ? " true" : " false"), Coast::Storage::Current());
		} else {
			StatTrace(Thread.IsWorking, "ThrdId: " << (long)GetId() << " CallId: " << MyId() << (bIsWorking ? " true" : " false") << " (StateMutex was not lockable)!", Coast::Storage::Current());
		}
	} else {
		bIsWorking = false;
	}
	return bCouldLock;
}

bool Thread::IsRunning( bool &bIsRunning )
{
	// assume that if we can't lock the mutex this Thread is not running
	bool bCouldLock( false );
	if ( IsAlive() ) {
		if ( fStateMutex.TryLock() ) {
			// now we have locked the mutex
			bIsRunning = ( fState == eRunning );
			fStateMutex.Unlock();
			bCouldLock = true;
			StatTrace(Thread.IsRunning, "fState:" << (long)fState << " ThrdId: " << (long)GetId() << " CallId: " << MyId() << (bIsRunning ? " true" : " false"), Coast::Storage::Current());
		} else {
			StatTrace(Thread.IsRunning, "fState:" << (long)fState << " ThrdId: " << (long)GetId() << " CallId: " << MyId() << (bIsRunning ? " true" : " false") << " (StateMutex was not lockable)!", Coast::Storage::Current());
		}
	} else {
		bIsRunning = false;
	}
	return bCouldLock;
}

//:Try to set Ready state
bool Thread::SetReady(ROAnything args)
{
	StatTrace(Thread.SetReady, "ThrdId: " << (long)GetId() << " CallId: " << MyId(), Coast::Storage::Current());
	return SetRunningState(eReady, args);
}

//:Try to set Working state
bool Thread::SetWorking(ROAnything args)
{
	StatTrace(Thread.SetWorking, "ThrdId: " << (long)GetId() << " CallId: " << MyId(), Coast::Storage::Current());
	return SetRunningState(eWorking, args);
}

bool Thread::Terminate(long timeout, ROAnything args)
{
	StartTrace1(Thread.Terminate, "Timeout: " << timeout << " ThrdId: " << (long)GetId() << " CallId: " << MyId());
	bool bRet = true;
	SetState(eTerminationRequested, args);
	bRet = CheckState(eTerminated, timeout);
	return bRet;
}

void Thread::IntRun()
{
	// IntId might be wrong since thread starts in parallel with the forking thread
	StatTrace(Thread.IntRun, "IntId: " << (long)GetId() << " ParId: " << fParentThreadId << " CallId: " << MyId() << " --- entering", Coast::Storage::Global());
#if defined(POOL_STARTEDHOOK)
	if ( !CheckState(eStartInProgress) )
#else
	if ( !CheckState(eStarted) ) //XXX remove
#endif
	{
		// needs syslog messages
		if ( !SetState(eTerminatedRunMethod) ) {
			StatTrace(Thread.IntRun, "SetState(eTerminatedRunMethod) failed MyId(" << MyId() << ") GetId( " << (long)GetId() << ")", Coast::Storage::Global());
		}
		return;
	}
#ifdef TRACE_LOCKS_IMPL
	SystemLog::WriteToStderr(String("Thr[") << fName << "]<" << Thread::MyId() << ">" << "\n");
#endif
	StatTrace(Thread.IntRun, "Registering thread(fAllocator) MyId(" << MyId() << ") GetId( " << (long)GetId() << ")", Coast::Storage::Global());
	// adds allocator reference to thread local store
	MT_Storage::RegisterThread(fAllocator);

	// now call DoStartedHook which has access to threads allocator using Coast::Storage::Current()
#if defined(POOL_STARTEDHOOK)
	if ( SetState(eStarted, fanyArgTmp) && CheckState(eStarted) )
#endif
	{
		if ( SetState(eRunning) ) {
			{
				LockUnlockEntry me(*fgpNumOfThreadsMutex);
				++fgNumOfThreads;
			}
			{
				String strWho(GetName());
				strWho.Append("::Run [").Append(MyId()).Append(']');
				MemChecker rekcehc(strWho, ( ( Coast::Storage::Current() != Coast::Storage::Global() ) ? Coast::Storage::Current() : (Allocator *)0 ) );
				// do the real work
				Run();
			}
			{
				LockUnlockEntry me(*fgpNumOfThreadsMutex);
				--fgNumOfThreads;
			}
		} else {
			StatTrace(Thread.IntRun, "SetState(eRunning) failed MyId(" << MyId() << ") GetId( " << (long)GetId() << ")", Coast::Storage::Global());
		}
	}
	if ( !SetState(eTerminatedRunMethod) ) {
		StatTrace(Thread.IntRun, "SetState(eTerminatedRunMethod) failed MyId(" << MyId() << ") GetId( " << (long)GetId() << ")", Coast::Storage::Global());
	}
	StatTrace(Thread.IntRun, "IntId: " << (long)GetId() << " ParId: " << fParentThreadId << " CallId: " << MyId() << " --- leaving", Coast::Storage::Global());
}

void Thread::Wait(long secs, long nanodelay)
{
	StatTrace(Thread.Wait, "secs:" << secs << " nano:" << nanodelay, Coast::Storage::Current());

	SimpleMutex m("ThreadWaitMutex", Coast::Storage::Global());
	SimpleMutex::ConditionType c;
	LockUnlockEntry me(m);
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
	LockUnlockEntry me(*fgpNumOfThreadsMutex);
	return fgNumOfThreads;
}

bool Thread::RegisterCleaner(CleanupHandler *handler)
{
#if defined(__APPLE__)
	CleanupAllocator::initializeCleanupAllocator();
#endif
	StartTrace1(Thread.RegisterCleaner, "CallId: " << MyId());
	Anything *handlerList = 0;
	if (!GETTLSDATA(fgCleanerKey, handlerList, Anything)) {
		handlerList = new Anything(Anything::ArrayMarker(),Coast::Storage::Global());
		if (!SETTLSDATA(fgCleanerKey, handlerList)) {
			// failed: immediately destroy Anything to avoid leak..
			delete handlerList;
			SystemLog::Error( "Thread::RegisterCleaner(CleanupHandler *) could not register cleanup handler" );
			return false;	// giving up..
		}
	}
	String adrKey;
	adrKey << (long)handler;
	(*handlerList)[adrKey] = Anything(reinterpret_cast<IFAObject *>(handler), Coast::Storage::Global());

	return true;
}

bool Thread::CleanupThreadStorage()
{
	StatTrace(Thread.CleanupThreadStorage, "CallId: " << MyId() << " entering", Coast::Storage::Global());

	bool bRet = true;
	Anything *handlerList = 0;
	if (GETTLSDATA(fgCleanerKey, handlerList, Anything) && handlerList) {
		for (long i = (handlerList->GetSize() - 1); i >= 0; --i) {
			CleanupHandler *handler = reinterpret_cast<CleanupHandler *>((*handlerList)[i].AsIFAObject(0));
			if (handler) {
				handler->Cleanup();
			}
		}
		delete handlerList;
		if (!SETTLSDATA(fgCleanerKey, 0)) {
			SystemLog::Error( "Thread::CleanupThreadStorage(CleanupHandler *) could not cleanup handler" );
			bRet = false;
		}
	}

	// unregister PoolAllocator of thread -> influences Coast::Storage::Current() retrieval
	MT_Storage::UnregisterThread();
	return bRet;
}

//---- Semaphore ------------------------------------------------------------

Semaphore::Semaphore(unsigned i_nCount)
{
	StartTrace(Semaphore.Semaphore);
	if ( !CREATESEMA(fSemaphore, i_nCount) ) {
		SystemLog::Error("Sema create failed");
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
	StatTrace(Semaphore.Acquire, "Sema&:" << (long)&fSemaphore << " CallId: " << Thread::MyId(), Coast::Storage::Current());
	return LOCKSEMA(fSemaphore);
}

bool Semaphore::TryAcquire()
{
	StatTrace(Semaphore.TryAcquire, "Sema&:" << (long)&fSemaphore << " CallId: " << Thread::MyId(), Coast::Storage::Current());
	return TRYSEMALOCK(fSemaphore);
}

int Semaphore::GetCount(int &count)
{
	int iRet( GETSEMACOUNT(fSemaphore, count) );
	StatTrace(Semaphore.GetCount, "Sema&:" << (long)&fSemaphore << " CallId: " << Thread::MyId() << " count: " << count << " iRet: " << iRet, Coast::Storage::Current());
	return iRet;
}

void Semaphore::Release()
{
	StatTrace(Semaphore.Release, "Sema&:" << (long)&fSemaphore << " CallId: " << Thread::MyId(), Coast::Storage::Current());
	if (!UNLOCKSEMA(fSemaphore)) {
		SystemLog::Error("UNLOCKSEMA failed");
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
	int iRet = 0;
	bool bRet( false );
	if ( !( bRet = CREATEMUTEX(fMutex, iRet) ) ) {
		SystemLog::Error(String("SimpleMutex<", Coast::Storage::Global()).Append(fName).Append(">: CREATEMUTEX failed: ").Append(SystemLog::SysErrorMsg(iRet)));
	}
	StatTrace(SimpleMutex.SimpleMutex, "id:" << (long)&fMutex << " CallId: " << Thread::MyId() << " [" << fName << "] code:" << iRet << ( bRet ? " succeeded" : " failed" ), Coast::Storage::Current());
}

SimpleMutex::~SimpleMutex()
{
	int iRet = 0;
	bool bRet( false );
	if ( !( bRet = DELETEMUTEX(fMutex, iRet) ) ) {
		SystemLog::Error(String("SimpleMutex<", Coast::Storage::Global()).Append(fName).Append(">: DELETEMUTEX failed: ").Append(SystemLog::SysErrorMsg(iRet)));
	}
	StatTrace(SimpleMutex.~SimpleMutex, "id:" << (long)&fMutex << " CallId: " << Thread::MyId() << " [" << fName << "] code:" << iRet << ( bRet ? " succeeded" : " failed" ), Coast::Storage::Current());
}

void SimpleMutex::Lock()
{
	int iRet = 0;
	bool bRet( false );
	if ( !( bRet = LOCKMUTEX(fMutex, iRet) ) ) {
		SystemLog::Error(String("SimpleMutex<", Coast::Storage::Global()).Append(fName).Append(">: LOCKMUTEX failed: ").Append(SystemLog::SysErrorMsg(iRet)));
	}
	StatTrace(SimpleMutex.Lock, "id:" << (long)&fMutex << " CallId: " << Thread::MyId() << " code:" << iRet << ( bRet ? " succeeded" : " failed" ), Coast::Storage::Current());
}

void SimpleMutex::Unlock()
{
	int iRet = 0;
	bool bRet( false );
	if ( !( bRet = UNLOCKMUTEX(fMutex, iRet) ) ) {
		SystemLog::Error(String("SimpleMutex<", Coast::Storage::Global()).Append(fName).Append(">: UNLOCKMUTEX failed: ").Append(SystemLog::SysErrorMsg(iRet)));
	}
	StatTrace(SimpleMutex.Unlock, "id:" << (long)&fMutex << " CallId: " << Thread::MyId() << " code:" << iRet, Coast::Storage::Current());
}

bool SimpleMutex::TryLock()
{
	int iRet = 0;
	bool bRet = TRYLOCK(fMutex, iRet);
	StatTrace(SimpleMutex.TryLock, "id:" << (long)&fMutex << " CallId: " << Thread::MyId() << " code:" << iRet << ( bRet ? " succeeded" : " failed" ), Coast::Storage::Current());
	if ( !bRet && (iRet != EBUSY) ) {
		SystemLog::Error(String("SimpleMutex<", Coast::Storage::Global()).Append(fName).Append(">: TRYLOCKMUTEX failed: ").Append(SystemLog::SysErrorMsg(iRet)));
	}
	return bRet;
}

//---- Mutex ------------------------------------------------------------
long Mutex::fgMutexId = 0;
SimpleMutex *Mutex::fgpMutexIdMutex = NULL;
THREADKEY Mutex::fgCountTableKey = 0;	// WIN32 defined it 0xFFFFFFFF !!

class MutexInitializer : public InitFinisManagerMTFoundation
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
		Mutex::fgpMutexIdMutex = new SimpleMutex("MutexIdMutex", Coast::Storage::Global());
		if (THRKEYCREATE(Mutex::fgCountTableKey, 0)) {
			SystemLog::Error("TlsAlloc of Mutex::fgCountTableKey failed");
		}
	}

	virtual void DoFinis() {
		InitFinisManager::IFMTrace("MutexInitializer::DoFinis\n");
		if (THRKEYDELETE(Mutex::fgCountTableKey) != 0) {
			SystemLog::Error("TlsFree of Mutex::fgCountTableKey failed" );
		}
		delete Mutex::fgpMutexIdMutex;
		Mutex::fgpMutexIdMutex = NULL;
	}
};
static MutexInitializer *psgMutexInitializer = new MutexInitializer(10);
//---- MutexCountTableCleaner ------------------------------------------------------------
MutexCountTableCleaner MutexCountTableCleaner::fgCleaner;

bool MutexCountTableCleaner::DoCleanup()
{
	StatTrace(MutexCountTableCleaner.DoCleanup, "ThrdId: " << Thread::MyId(), Coast::Storage::Global());
	Anything *countarray = 0;
	if (GETTLSDATA(Mutex::fgCountTableKey, countarray, Anything)) {
		// as the countarray behavior changed, mutex entries which were used by the thread
		//  are still listed but should all have values of 0
		long lSize((*countarray).GetSize());
		bool bHadLockEntries = false;
		while ( --lSize >= 0L ) {
			if ( (*countarray)[lSize].AsLong(-1L) > 0L ) {
				bHadLockEntries = true;
			}
		}
		// trace errors only
		if ( bHadLockEntries ) {
			String strbuf(Coast::Storage::Global());
			OStringStream stream(strbuf);
			stream << "MutexCountTableCleaner::DoCleanup: ThrdId: " << Thread::MyId() << "\n  countarray still contained Mutex locking information!\n";
			(*countarray).Export(stream, 2);
			stream.flush();
			SystemLog::WriteToStderr(strbuf);
		}
		delete countarray;
		countarray = 0;
		if (SETTLSDATA(Mutex::fgCountTableKey, countarray)) {
			return true;
		}
	}
	return false;
}

class GlobalIdMutexNotInitialized : public std::exception
{
public:
	virtual const char *what() const throw() {
		static const char pMsg[] = "FATAL: fgpMutexIdMutex not created yet!!";
		SYSERROR(pMsg);
		return pMsg;
	}
};

Mutex::Mutex(const char *name, Allocator *a)
	: fMutexId(a)
	, fLocker(-1)
	, fName(name, -1, a)
{
	if ( !fgpMutexIdMutex ) {
		throw GlobalIdMutexNotInitialized();
	} else {
		LockUnlockEntry aMtx(*fgpMutexIdMutex);
		fMutexId.Append(++fgMutexId);
	}
	int iRet = 0;
	if ( !CREATEMUTEX(fMutex, iRet) ) {
		SystemLog::Error(String("Mutex<").Append(fName).Append(">: CREATEMUTEX failed: ").Append(SystemLog::SysErrorMsg(iRet)));
	}
#ifdef TRACE_LOCKS_IMPL
	SystemLog::WriteToStderr(String("[") << fName << " Id " << GetId() << "]<" << GetId() << "> A" << "\n");
#endif
}

Mutex::~Mutex()
{
#ifdef TRACE_LOCKS_IMPL
	SystemLog::WriteToStderr(String("[") << fName << " Id " << GetId() << "]<" << GetId() << "> D" << "\n");
#endif
	// cleanup countarray
	int iRet = 0;
	if ( !DELETEMUTEX(fMutex, iRet) ) {
		SystemLog::Error(String("Mutex<").Append(fName).Append(">: DELETEMUTEX failed: ").Append(SystemLog::SysErrorMsg(iRet)));
	}
}

long Mutex::GetCount()
{
	Anything *countarray = 0;
	long lCount = 0L;
	GETTLSDATA(fgCountTableKey, countarray, Anything);
	if (countarray) {
		StatTraceAny(Mutex.GetCount, (*countarray), "countarray", Coast::Storage::Current());
		lCount = ((ROAnything)(*countarray))[fMutexId].AsLong(0L);
	}
	StatTrace(Mutex.GetCount, "Count:" << lCount << " CallId: " << Thread::MyId(), Coast::Storage::Current());
	return lCount;
}

bool Mutex::SetCount(long newCount)
{
	StatTrace(Mutex.SetCount, "CallId: " << Thread::MyId() << " newCount: " << newCount, Coast::Storage::Current());
	Anything *countarray = 0;
	GETTLSDATA(fgCountTableKey, countarray, Anything);

	if (!countarray && newCount > 0) {
		countarray = new Anything(Anything::ArrayMarker(),Coast::Storage::Global());
		Thread::RegisterCleaner(&MutexCountTableCleaner::fgCleaner);
		if (!SETTLSDATA(fgCountTableKey, countarray)) {
			SystemLog::Error("Mutex::SetCount: could not store recursive locking structure in TLS!");
			return false;
		}
	}
	if (countarray) {
		StatTraceAny(Mutex.SetCount, (*countarray), "countarray", Coast::Storage::Current());
		if (newCount <= 0) {
			(*countarray).Remove(fMutexId);
		} else {
			(*countarray)[fMutexId] = newCount;
			StatTraceAny(Mutex.SetCount, (*countarray), "Mutex::SetCount: Id[" << fMutexId << "] count: " << newCount, Coast::Storage::Current());
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
		StatTrace(Mutex.Lock, "First try to lock failed, eg. not recursive lock -> 'hard'-locking now. Id: " <<  GetId() << " CallId: " << Thread::MyId(), Coast::Storage::Current());
		int iRet = 0;
		if ( !LOCKMUTEX(fMutex, iRet) ) {
			SystemLog::Error(String("Mutex<").Append(fName).Append(">: LOCKMUTEX failed: ").Append(SystemLog::SysErrorMsg(iRet)));
		}
#ifdef TRACE_LOCKS_IMPL
		if (fLocker != -1) {
			SystemLog::WriteToStderr(String("[") << fName << " Id " << GetId() << "]<" << GetCount() << "," << fLocker << "> overriding current locker!" << "\n");
		}
#endif
		fLocker = Thread::MyId();
		SetCount(1);
#ifdef TRACE_LOCKS_IMPL
		SystemLog::WriteToStderr(String("[") << fName << " Id " << GetId() << "]<" << GetCount() << "," << fLocker << "> L" << "\n");
#endif
	} else {
		StatTrace(Mutex.Lock, "TryLock success, Id: " <<  GetId() << " CallId: " << Thread::MyId(), Coast::Storage::Current());
	}
	TRACE_LOCK_ACQUIRE(fName);
}

void Mutex::Unlock()
{
	if ( fLocker == Thread::MyId() ) {
		long actCount = GetCount() - 1;
		SetCount(actCount);
		StatTrace(Mutex.Unlock, "new lockcount:" << actCount << " Id: " <<  GetId() << " CallId: " << Thread::MyId(), Coast::Storage::Current());
#ifdef TRACE_LOCKS_IMPL
		SystemLog::WriteToStderr(String("[") << fName << " Id " << GetId() << "]<" << GetCount() << "," << fLocker << "> TR" << "\n");
#endif
		if (actCount <= 0) {
#ifdef TRACE_LOCKS_IMPL
			SystemLog::WriteToStderr(String("[") << fName << " Id " << GetId() << "]<" << GetCount() << "," << fLocker << "> R" << "\n");
#endif
			fLocker = -1;
			SetCount(0);
			int iRet = 0;
			if ( !UNLOCKMUTEX(fMutex, iRet) ) {
				SystemLog::Error(String("Mutex<").Append(fName).Append(">: UNLOCKMUTEX failed: ").Append(SystemLog::SysErrorMsg(iRet)));
			}
		}
	} else {
		String logMsg("[");
		logMsg << fName << " Id " << GetId() << "]<" << GetCount() << "," << fLocker << ">";
		logMsg << " Locking error: calling unlock while not holding the lock";
		SystemLog::Error(logMsg);
#ifdef TRACE_LOCKS_IMPL
		SystemLog::WriteToStderr(logMsg << "\n");
#endif
	}
	TRACE_LOCK_RELEASE(fName);
}

bool Mutex::TryLock()
{
	int iRet = 0;
	bool hasLocked = TRYLOCK(fMutex, iRet);
	StatTrace(Mutex.TryLock, "Mutex " << (hasLocked ? "" : "not") << " locked Id: " <<  GetId() << " CallId: " << Thread::MyId() << " Locker: " << fLocker << " Count: " << GetCount(), Coast::Storage::Current());
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
			SystemLog::WriteToStderr(String("[") << fName << " Id " << GetId() << "]<" << GetCount() << "," << fLocker << "> TL" << "\n");
#endif
		} else {
			// consecutive acquire
			++lCount;
			SetCount(lCount);
			UNLOCKMUTEX(fMutex, iRet);
#ifdef TRACE_LOCKS_IMPL
			SystemLog::WriteToStderr(String("[") << fName << " Id " << GetId() << "]<" << GetCount() << "," << fLocker << "> TLA" << "\n");
#endif
		}
	} else if ( fLocker == Thread::MyId() ) {
#ifdef TRACE_LOCKS_IMPL
		SystemLog::WriteToStderr(String("[") << fName << " Id " << GetId() << "]<" << GetCount() << "," << fLocker << "> TLA" << "\n");
#endif
		lCount += GetCount();
		SetCount(lCount);
		hasLocked = true;
	}
#else
	if ( hasLocked ) {
		fLocker = Thread::MyId();
		SetCount(lCount);
#ifdef TRACE_LOCKS_IMPL
		SystemLog::WriteToStderr(String("[") << fName << " Id " << GetId() << "]<" << GetCount() << "," << fLocker << "> TL" << "\n");
#endif
		StatTrace(Mutex.TryLock, "first lock, count:" << lCount << " Id: " <<  GetId() << " CallId: " << Thread::MyId(), Coast::Storage::Current());
	} else {
		if ( fLocker == Thread::MyId() ) {
			lCount += GetCount();
			SetCount(lCount);
			hasLocked = true;
			StatTrace(Mutex.TryLock, "recursive lock, count:" << lCount << " Id: " <<  GetId() << " CallId: " << Thread::MyId(), Coast::Storage::Current());
#ifdef TRACE_LOCKS_IMPL
			SystemLog::WriteToStderr(String("[") << fName << " Id " << GetId() << "]<" << GetCount() << "," << fLocker << "> TLA" << "\n");
#endif
		} else {
			if ( iRet != EBUSY ) {
				SystemLog::Error(String("Mutex<").Append(fName).Append(">: TRYLOCKMUTEX failed: ").Append(SystemLog::SysErrorMsg(iRet)));
			}
		}
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
	StartTrace1(Mutex.ReleaseForWait, "Id: " <<  GetId() << " CallId: " << Thread::MyId());
#ifdef TRACE_LOCKS_IMPL
	SystemLog::WriteToStderr(String("[") << fName << " Id " << GetId() << "]<" << GetCount() << "," << fLocker << "> RFW" << "\n");
#endif
	fLocker = -1;
}

void Mutex::AcquireAfterWait()
{
	StartTrace1(Mutex.AcquireAfterWait, "Id: " <<  GetId() << " CallId: " << Thread::MyId() << " fLocker: " << fLocker);
	fLocker = Thread::MyId();
#ifdef TRACE_LOCKS_IMPL
	SystemLog::WriteToStderr(String("[") << fName << " Id " << GetId() << "]<" << GetCount() << "," << fLocker << "> AAW" << "\n");
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

void RWLock::Lock(eLockMode mode) const
{
	LOCKRWLOCK(fLock, (mode == eReading));
	TRACE_LOCK_ACQUIRE(fName);
}

void RWLock::Unlock(eLockMode mode) const
{
	UNLOCKRWLOCK(fLock, (mode == eReading));
	TRACE_LOCK_RELEASE(fName);
}

bool RWLock::TryLock(eLockMode mode) const
{
	bool hasLocked = TRYRWLOCK(fLock, (mode == eReading));
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
	StatTrace(SimpleCondition.SimpleCondition, "", Coast::Storage::Current());
	if ( !CREATECOND(fSimpleCondition) ) {
		SystemLog::Error("CREATECOND failed");
	}
}

SimpleCondition::~SimpleCondition()
{
	StatTrace(SimpleCondition.~SimpleCondition, "", Coast::Storage::Current());
	if ( !DELETECOND(fSimpleCondition) ) {
		SystemLog::Error("DELETECOND failed");
	}
}

int SimpleCondition::Wait(SimpleMutex &m)
{
	StatTrace(SimpleCondition.Wait, "releasing SimpleMutex[" << m.fName << "] CallId:" << Thread::MyId(), Coast::Storage::Current());
	if (!CONDWAIT(fSimpleCondition, m.GetInternal())) {
		SystemLog::Error("CONDWAIT failed");
	}
	StatTrace(SimpleCondition.Wait, "reacquired SimpleMutex[" << m.fName << "] CallId:" << Thread::MyId(), Coast::Storage::Current());
	return 0;
}

int SimpleCondition::TimedWait(SimpleMutex &m, long secs, long nanosecs)
{
	StatTrace(SimpleCondition.TimedWait, "releasing SimpleMutex[" << m.fName << "] secs:" << secs << " nano:" << nanosecs << " CallId:" << Thread::MyId(), Coast::Storage::Current());
	int iRet = CONDTIMEDWAIT(fSimpleCondition, m.GetInternal(), secs, nanosecs);
	StatTrace(SimpleCondition.TimedWait, "reacquired SimpleMutex[" << m.fName << "] CallId:" << Thread::MyId(), Coast::Storage::Current());
	return iRet;
}

int SimpleCondition::Signal()
{
	StatTrace(SimpleCondition.Signal, "", Coast::Storage::Current());
	return SIGNALCOND(fSimpleCondition);
}

int SimpleCondition::BroadCast()
{
	StatTrace(SimpleCondition.BroadCast, "", Coast::Storage::Current());
	return BROADCASTCOND(fSimpleCondition);
}

long SimpleCondition::GetId()
{
	long lId = (long)GetInternal();
	StatTrace(SimpleCondition.GetId, lId, Coast::Storage::Current());
	return lId;
}

//---- Condition ------------------------------------------------
Condition::Condition()
{
	if ( !CREATECOND(fCondition) ) {
		SystemLog::Error("CREATECOND failed");
	}
}

Condition::~Condition()
{
	if ( !DELETECOND(fCondition) ) {
		SystemLog::Error("DELETECOND failed");
	}
}

int Condition::Wait(Mutex &m)
{
	StatTrace(Condition.Wait, "releasing Mutex[" << m.fName << "] Id: " << GetId() << " CallId:" << Thread::MyId(), Coast::Storage::Current());
	m.ReleaseForWait();
	if ( !CONDWAIT(fCondition, m.GetInternal()) ) {
		SystemLog::Error("CONDWAIT failed");
	}
	StatTrace(Condition.Wait, "reacquired Mutex[" << m.fName << "] Id: " << GetId() << " CallId:" << Thread::MyId(), Coast::Storage::Current());
	m.AcquireAfterWait();
	return 0;
}

int Condition::TimedWait(Mutex &m, long secs, long nanosecs)
{
	StatTrace(Condition.TimedWait, "releasing Mutex[" << m.fName << "] secs:" << secs << " nano:" << nanosecs << " Id: " << GetId() << " CallId:" << Thread::MyId(), Coast::Storage::Current());
	m.ReleaseForWait();
	int ret = CONDTIMEDWAIT(fCondition, m.GetInternal(), secs, nanosecs);
	StatTrace(Condition.Wait, "reacquired Mutex[" << m.fName << "] Id: " << GetId() << " CallId:" << Thread::MyId(), Coast::Storage::Current());
	m.AcquireAfterWait();
	return ret;
}

int Condition::Signal()
{
	StatTrace(Condition.Signal, "Id: " << GetId() << " CallId: " << Thread::MyId(), Coast::Storage::Current());
	return SIGNALCOND(fCondition);
}

int Condition::BroadCast()
{
	StatTrace(Condition.BroadCast, "Id: " << GetId() << " CallId: " << Thread::MyId(), Coast::Storage::Current());
	return BROADCASTCOND(fCondition);
}

long Condition::GetId()
{
	long lId = (long)GetInternal();
	StatTrace(Condition.GetId, lId, Coast::Storage::Current());
	return lId;
}
