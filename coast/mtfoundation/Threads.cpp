/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

//--- interface include ---------------------------------------------------------
#include "Threads.h"

//--- standard modules used ----------------------------------------------------
#include "SysLog.h"
#include "DiffTimer.h"
#include "StringStream.h"
#include "TraceLocks.h"
#include "Dbg.h"

//--- c-library modules used ---------------------------------------------------

//#define  TRACE_LOCKS_IMPL 1

THREADKEY Thread::fgCleanerKey = 0;

//---- CleanupInitializer ------------------------------------------------------------
class EXPORTDECL_MTFOUNDATION CleanupInitializer : public FinalCleaner
{
public:
	//:Constructor geared towards solaris thread api
	CleanupInitializer() {
		StartTrace(CleanupInitializer.CleanupInitializer);
		if ( THRKEYCREATE(Thread::fgCleanerKey, 0)) {
			SysLog::Error("CleanupInitializer::TLS key create failed" );
		}
	}
	~CleanupInitializer() {
		StartTrace(CleanupInitializer.~CleanupInitializer);
		if (THRKEYDELETE(Thread::fgCleanerKey) != 0) {
			SysLog::Error("CleanupInitializer::TLS key delete failed" );
		}
	}
};

// notice: never delete this class or its internal structure!
// check with the base class FinalCleaner for understanding the sequence of deletion
// the CleanupInitializer instance will be deleted from within FinalCleaner::~FinalCleaner
class CleanupAllocator
{
public:
	CleanupAllocator() {
		new CleanupInitializer();
	}

};

static CleanupAllocator fgInitKey;

//---- AllocatorUnref ------------------------------------------------------------
AllocatorUnref::AllocatorUnref(Thread *t) : fThread(t)
{
}

AllocatorUnref::~AllocatorUnref()
{
	if (fThread) {
		Allocator *a = fThread->fAllocator;
		if (a) {
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

	//:method used to cleanup specific settings within
	// thread specific storage
	virtual bool DoCleanup();

	static MutexCountTableCleaner fgCleaner;
};

//---- Thread ------------------------------------------------------------

//:thread count variable
long Thread::fgNumOfThreads = 0;

//:guard of thread count
SimpleMutex Thread::fgNumOfThreadsMutex("NumOfThreads");

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
	StartTrace1(Thread.Constructor, "IntId: " << (long)GetId() << " ParId: " << fParentThreadId << " CallId: " << MyId());
	if (fAllocator) {
		fAllocator->Ref();
	}
	fParentThreadId = MyId();
}

Thread::~Thread()
{
	StartTrace1(Thread.Destructor, "IntId: " << (long)GetId() << " ParId: " << fParentThreadId << " CallId: " << MyId());
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
				// FIXME: params have still to be cleaned up
				bool *b = new bool[4];
				b[0] = fBound;
				b[1] = fDetached;
				b[2] = fDaemon;
				b[3] = fSuspended;
				bool ret = STARTTHREAD(this, b, &fThreadId, ThreadWrapper);
				if ( ret ) {
					SetState(eStarted, args);
				}
				delete [] b;
				Trace("Start MyId(" << MyId() << ") started GetId( " << (long)fThreadId << ")" );
				return ret;
			}
		} else {
			String msg("No valid allocator supplied; no thread started");
			SysLog::Error(msg);
		}
	} else {
		String msg("Thread::Start has already been called started");
		SysLog::Error(msg);
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
	SimpleMutexEntry me(fObserversMutex);
	me.Use();

	for (long i = 0; i < fObservers.GetSize(); i++) {
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
	NotifyAll(evt);
	fStateCond.BroadCast();
}

bool Thread::CheckState(EThreadState state, long timeout, long nanotimeout)
{
	StartTrace1(Thread.CheckState, "State: " << (long)state << " IntId: " << (long)GetId() << " ParId: " << fParentThreadId << " CallId: " << MyId());
	SimpleMutexEntry me(fStateMutex);
	me.Use();
	return IntCheckState(state, timeout, nanotimeout);
}

bool Thread::IntCheckState(EThreadState state, long timeout, long nanotimeout)
{
	StartTrace1(Thread.IntCheckState, "State: " << (long)state << " IntId: " << (long)GetId() << " ParId: " << fParentThreadId << " CallId: " << MyId());

	if (fState >= state) {
		return true;
	}

	if (timeout || nanotimeout) {
		DiffTimer dt;
		long militimeout = timeout * 1000L + nanotimeout / 1000000;
		while ( (fState < state) && ( dt.Diff() < (militimeout) )) {
			Trace("waiting on fState(" << (long)fState << ")>=state(" << (long)state << ") CallId: " << MyId());
			fStateCond.TimedWait(fStateMutex, timeout, nanotimeout);
		}
		Trace("fState: " << (long)fState << " IntId: " << (long)GetId() << " ParId: " << fParentThreadId << " CallId: " << MyId());
	} else {
		while (fState < state) {
			Trace("waiting on fState(" << (long)fState << ")>=state(" << (long)state << ") CallId: " << MyId());
			fStateCond.Wait(fStateMutex);
		}
		Trace("fState: " << (long)fState << " IntId: " << (long)GetId() << " ParId: " << fParentThreadId << " CallId: " << MyId());
	}
	return ( fState >= state );
}

// a helper macro for debugging...
#define CHECKRUNNING_STATE_DBG(message)  Trace((message) \
		  <<  " fRunningState: " << (long) fRunningState << " state: " << (long) state << " fState: " << (long) fState \
		  <<  " realDiff: " << realDiff << " compDiff: " << compDiff \
		  <<  " Seconds: " << seconds << " NanoSeconds: " << nanoSeconds \
		  <<  " Timeout given: " << (timeout*1000L));

bool Thread::CheckRunningState(ERunningState state, long timeout, long nanotimeout)
{
	StartTrace1(Thread.CheckRunningState, "State: " << (long)state << " IntId: " << (long)GetId() << " ParId: " << fParentThreadId << " CallId: " << MyId());
	SimpleMutexEntry me(fStateMutex);
	me.Use();

	if (fState < eRunning ) {
		Trace("thread not yet running, waiting until it is running");
		bool ret = IntCheckState(eRunning, timeout, nanotimeout);
		if (!ret) {
			Trace("timeout while waiting for eRunning");
			return ret;
		}
	}

	if (fState > eRunning ) {
		Trace("thread not running anymore, exiting");
		return false;
	}
	Assert(fState == eRunning);

	if (fRunningState == state) {
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
	StartTrace1(Thread.GetState, "IntId: " << (long)GetId() << " ParId: " << fParentThreadId << " CallId: " << MyId());
	if (!trylock) {
		SimpleMutexEntry me(fStateMutex);
		me.Use();
		Trace("fState:" << (long)fState);
		return fState;
	}
	if (fStateMutex.TryLock()) {
		Trace("could lock mutex");
		dfltState = fState;
		fStateMutex.Unlock();
	}
	Trace("returning:" << (long)dfltState);
	return dfltState;
}

bool Thread::SetState(EThreadState state, ROAnything args)
{
	StartTrace1(Thread.SetState, "State: " << (long)state << " IntId: " << (long)GetId() << " ParId: " << fParentThreadId << " CallId: " << MyId());
	SimpleMutexEntry me(fStateMutex);
	me.Use();

	if (state == fState + 1) {
		return IntSetState(state, args);
	}
	if (state == eTerminationRequested && fState < eTerminationRequested) {
		return IntSetState((fState == eCreated || fState == eStartRequested) ? eTerminated : state, args);
	}
	if (fState == eTerminated && state == eCreated) {
		return IntSetState(state, args);
	}
	if (state == eTerminated && MyId() == (long)GetId()) {
		return IntSetState(state, args);
	}
	if (state == eTerminated) {
		Trace("SetState:(" << (long)state << ") fState(" << (long)fState << ") IntId: " << (long)GetId() << " CallId: " << MyId());
	}
	return false;
}

bool Thread::IntSetState(EThreadState state, ROAnything args)
{
	if (CallStateHooks(state, args)) {
		fState = state;
		BroadCastEvent((long)fState);
		return true;
	}
	return false;
}

bool Thread::CallStateHooks(EThreadState state, ROAnything args)
{
	StartTrace1(Thread.CallStateHooks, "State :" << (long)state);

	switch (state) {
		case eStartRequested:
			return DoStartRequestedHook(args);
		case eTerminationRequested:
			DoTerminationRequestHook(args);
			break;
		case eTerminated:
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
void Thread::DoTerminationRequestHook(ROAnything) {};
void Thread::DoTerminatedHook() {};

bool Thread::SetRunningState(ERunningState state, ROAnything args)
{
	SimpleMutexEntry me(fStateMutex);
	me.Use();

	if (fState != eRunning || fRunningState == state) {
		return false;
	}

	fRunningState = state;
	CallRunningStateHooks(state, args);
	BroadCastEvent((long)fRunningState);
	return true;
}

void Thread::CallRunningStateHooks(ERunningState state, ROAnything args)
{
	StartTrace1(Thread.CallRunningStateHooks, "State :" << (long)state);

	switch (state) {
		case eReady:
			DoReadyHook(args);
			break;
		case eWorking:
			DoWorkingHook(args);
		default:
			;
	}
}

void Thread::DoReadyHook(ROAnything) {};
void Thread::DoWorkingHook(ROAnything) {};

bool Thread::IsReady()
{
	// assume that if we can't lock the mutex
	// this WorkerThread object is not ready

	bool isReady = false;
	if (fStateMutex.TryLock()) {
		// now we have locked the mutex
		isReady = (fRunningState == eReady && fState == eRunning);
		fStateMutex.Unlock();
	}
	return isReady;
}

bool Thread::IsWorking()
{
	StartTrace(Thread.IsWorking);
	// assume that if we can't lock the mutex
	// this WorkerThread object is working

	bool isWorking = true;
	if (fStateMutex.TryLock()) {
		// now we have locked the mutex
		isWorking = (fRunningState == eWorking && fState == eRunning);
		fStateMutex.Unlock();
	}
	return isWorking;
}

bool Thread::IsRunning()
{
	StartTrace(Thread.IsRunning);
	// assume that if we can't lock the mutex
	// this WorkerThread object is not ready

	bool isRunning = true;
	if (fStateMutex.TryLock()) {
		// now we have locked the mutex
		isRunning = (fState == eRunning);
		fStateMutex.Unlock();
	}
	return isRunning;
}

//:Try to set Ready state
bool Thread::SetReady(ROAnything args)
{
	StartTrace(Thread.SetReady);
	return SetRunningState(eReady, args);
}

//:Try to set Working state
bool Thread::SetWorking(ROAnything args)
{
	StartTrace(Thread.SetWorking);
	return SetRunningState(eWorking, args);
}

bool Thread::Terminate(long timeout, ROAnything args)
{
	StartTrace1(Thread.Terminate, "Timeout: " << timeout << " ThrdId: " << MyId());

	SetState(eTerminationRequested, args);

	return CheckState(eTerminated, timeout);
}

void Thread::IntRun()
{
	// IntId might be wrong since thread starts in parallel with the forking thread
	StartTrace1(Thread.IntRun, "IntId: " << (long)GetId() << " ParId: " << fParentThreadId << " CallId: " << MyId());
	if (!CheckState(eStarted)) {
		// needs syslog messages
		if (!SetState(eTerminated)) {
			Trace("SetState(eTerminated) failed MyId(" << MyId() << ") GetId( " << (long)GetId() << ")" );
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
			SimpleMutexEntry me(fgNumOfThreadsMutex);
			fgNumOfThreads++;
		}
		// do the real work
		Run();
		{
			SimpleMutexEntry me(fgNumOfThreadsMutex);
			fgNumOfThreads--;
		}
	} else {
		Trace("SetState(eRunning) failed MyId(" << MyId() << ") GetId( " << (long)GetId() << ")" );
	}
#if !(defined(WIN32) && defined(_DLL))
	if (!SetState(eTerminated)) {
		Trace("SetState(eTerminated) failed MyId(" << MyId() << ") GetId( " << (long)GetId() << ")");
	}
	CleanupThreadStorage();
#endif
}

void Thread::Wait(long secs, long nanodelay)
{
	StartTrace(Thread.Wait);

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
	StartTrace(Thread.MyId);
	return THRID();
}

long Thread::NumOfThreads()
{
	StartTrace(Thread.NumOfThreads);
	SimpleMutexEntry me(fgNumOfThreadsMutex);
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
	StartTrace1(Thread.CleanupThreadStorage, "CallId: " << MyId());

	Anything *handlerList = 0;
	if (GETTLSDATA(fgCleanerKey, handlerList, Anything) && handlerList) {
		for (long i = (handlerList->GetSize() - 1); i >= 0; i--) {
			CleanupHandler *handler = (CleanupHandler *)((*handlerList)[i].AsIFAObject(0));
			if (handler) {
				handler->DoCleanup();
			}
		}
		delete handlerList;
		if (!SETTLSDATA(fgCleanerKey, 0)) {
			SysLog::Error( "Thread::CleanupThreadStorage(CleanupHandler *) could not cleanup handler" );
			return false;	// giving up..
		}
	}
	return true;
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
	StartTrace(Semaphore.Acquire);
	return LOCKSEMA(fSemaphore);
}

bool Semaphore::TryAcquire()
{
	StartTrace(Semaphore.TryAcquire);
	return TRYSEMALOCK(fSemaphore);
}

void Semaphore::Release()
{
	StartTrace(Semaphore.Release);
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
	if ( !CREATEMUTEX(fMutex) ) {
		SysLog::Error("CREATEMUTEX failed");
	}
}

SimpleMutex::~SimpleMutex()
{
	if ( !DELETEMUTEX(fMutex) ) {
		SysLog::Error("DELETEMUTEX failed");
	}
}

void SimpleMutex::Lock()
{
	if ( !LOCKMUTEX(fMutex) ) {
		SysLog::Error("LOCKMUTEX failed");
	}
}

void SimpleMutex::Unlock()
{
	if ( !UNLOCKMUTEX(fMutex) ) {
		SysLog::Error("LOCKMUTEX failed");
	}
}

bool SimpleMutex::TryLock()
{
	return TRYLOCK(fMutex);
}

//---- Mutex ------------------------------------------------------------
long Mutex::fgMutexId = 0;
#if defined(__sun)
MUTEX Mutex::fgMutexIdMutex = DEFAULTMUTEX;
#elif defined (__linux__) || defined(_AIX)
MUTEX Mutex::fgMutexIdMutex = PTHREAD_MUTEX_INITIALIZER;
#elif WIN32
MUTEX Mutex::fgMutexIdMutex = ::CreateMutex(NULL, false, NULL);
#endif

THREADKEY Mutex::fgCountTableKey = 0;	// WIN32 defined it 0xFFFFFFFF !!

//---- CleanupHandler ------------------------------------------------------------
MutexCountTableCleaner MutexCountTableCleaner::fgCleaner;
bool MutexCountTableCleaner::DoCleanup()
{
	StartTrace1(MutexCountTableCleaner.DoCleanup, "ThrdId: " << Thread::MyId());
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
	// allocation of class global ressources
	if ( !LOCKMUTEX(fgMutexIdMutex) ) {
		SysLog::Error("LOCKMUTEX failed");
	}
	if ( fgMutexId == 0 ) {
#if defined( __sun) || defined (__linux__) || defined(_AIX)
		if (THRKEYCREATE(fgCountTableKey, 0)) {
			SysLog::Error("Mutex::Mutex: TLS key create failed");
#elif WIN32
#	ifndef _DLL
		// Allocate a TLS index.
		//FIXME in case of DLL linkage, this needs to be done in DllMain
		if ((Mutex::fgCountTableKey = TlsAlloc()) == 0xFFFFFFFF)  {
			SysLog::Error("TlsAlloc failed");
#	else
			if (Mutex::fgCountTableKey == 0xFFFFFFFF)  {
				SysLog::Error( "TlsAlloc error for Mutex::fgCountTableKey");
#endif	// _DLL
#endif
			if (!UNLOCKMUTEX(fgMutexIdMutex)) {
				SysLog::Error("Mutex unlock failed");
			}
			SysLog::Error("Mutex creation failed");
			return;
		}
	}
	fMutexId.Append(fgMutexId++);
	if ( !UNLOCKMUTEX(fgMutexIdMutex) || !CREATEMUTEX(fMutex) ) {
		SysLog::Error("CREATEMUTEX failed");
	}

#ifdef TRACE_LOCKS_IMPL
	SysLog::WriteToStderr(String("[") << fName << " Id " << GetId() << "]<" << GetId() << "> A" << "\n");
#endif
}

Mutex::~Mutex() {
#ifdef TRACE_LOCKS_IMPL
	SysLog::WriteToStderr(String("[") << fName << " Id " << GetId() << "]<" << GetId() << "> D" << "\n");
#endif
	if ( !DELETEMUTEX(fMutex) ) {
		SysLog::Error("DELETEMUTEX failed");
	}
}

long Mutex::GetCount() {
	StartTrace1(Mutex.GetCount, "ThrdId: " << Thread::MyId());
	MetaThing *countarray = 0;
	GETTLSDATA(fgCountTableKey, countarray, MetaThing);
	if (countarray) {
		TraceAny((*countarray), "countarray");
		return ((ROAnything)(*countarray))[fMutexId].AsLong();
	}
	return 0;
}

bool Mutex::SetCount(long newCount) {
	StartTrace1(Mutex.SetCount, "ThrdId: " << Thread::MyId() << " newCount: " << newCount);
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
		TraceAny((*countarray), "countarray");
		if (newCount <= 0) {
			(*countarray).Remove(fMutexId);
		} else {
			(*countarray)[fMutexId] = newCount;
		}
	}
	return true;
}

String &Mutex::GetId() {
#if defined(WIN32) && defined(TRACE_LOCK_UNLOCK)
	fMutexIdTL = (long)fMutex;
	return fMutexIdTL;
#else
	return fMutexId;
#endif
}

void Mutex::Lock() {
	StartTrace1(Mutex.Lock, "Id: " <<  GetId() << " ThrdId: " << Thread::MyId());
	if (!TryLock()) {
		if ( !LOCKMUTEX(fMutex) ) {
			SysLog::Error("UNLOCKMUTEX failed");
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
		Trace("Count: " << GetCount());
	}
	TRACE_LOCK_ACQUIRE(fName);
}

void Mutex::Unlock() {
	StartTrace1(Mutex.Unlock, "Id: " <<  GetId() << " ThrdId: " << Thread::MyId());
	if ( fLocker == Thread::MyId() ) {
		long actCount = GetCount() - 1;
		SetCount(actCount);
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
	Trace("Count: " << GetCount());
	TRACE_LOCK_RELEASE(fName);
}

bool Mutex::TryLock() {
	StartTrace1(Mutex.TryLock, "Id: " <<  GetId() << " ThrdId: " << Thread::MyId() << " Locker: " << fLocker << " Count: " << GetCount());
	bool hasLocked = TRYLOCK(fMutex);
#if defined(WIN32)
	if (hasLocked) {
		// WIN32 allows same thread to acquire the mutex more than once without deadlocking
		if (GetCount() == 0) {
			// mutex acquired for the first time
			fLocker = Thread::MyId();
			SetCount(1);
#ifdef TRACE_LOCKS_IMPL
			SysLog::WriteToStderr(String("[") << fName << " Id " << GetId() << "]<" << GetCount() << "," << fLocker << "> TL" << "\n");
#endif
		} else {
			// consecutive acquire
			SetCount(GetCount() + 1);
			UNLOCKMUTEX(fMutex);
#ifdef TRACE_LOCKS_IMPL
			SysLog::WriteToStderr(String("[") << fName << " Id " << GetId() << "]<" << GetCount() << "," << fLocker << "> TLA" << "\n");
#endif
		}
	} else if ( fLocker == Thread::MyId() ) {
#ifdef TRACE_LOCKS_IMPL
		SysLog::WriteToStderr(String("[") << fName << " Id " << GetId() << "]<" << GetCount() << "," << fLocker << "> TLA" << "\n");
#endif
		SetCount(GetCount() + 1);
		hasLocked = true;
	}
#else
	if (hasLocked) {
		fLocker = Thread::MyId();
		SetCount(1);
		Trace("Count: " << GetCount());
#ifdef TRACE_LOCKS_IMPL
		SysLog::WriteToStderr(String("[") << fName << " Id " << GetId() << "]<" << GetCount() << "," << fLocker << "> TL" << "\n");
#endif
	} else if ( fLocker == Thread::MyId() ) {
		SetCount(GetCount() + 1);
		hasLocked = true;
		Trace("Count: " << GetCount());
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

void Mutex::ReleaseForWait() {
	StartTrace1(Mutex.ReleaseForWait, "Id: " <<  GetId() << " ThrdId: " << Thread::MyId());
#ifdef TRACE_LOCKS_IMPL
	SysLog::WriteToStderr(String("[") << fName << " Id " << GetId() << "]<" << GetCount() << "," << fLocker << "> RFW" << "\n");
#endif
	fLocker = -1;
}

void Mutex::AcquireAfterWait() {
	StartTrace1(Mutex.AcquireAfterWait, "Id: " <<  GetId() << " ThrdId: " << Thread::MyId() << " fLocker: " << fLocker);
	fLocker = Thread::MyId();
#ifdef TRACE_LOCKS_IMPL
	SysLog::WriteToStderr(String("[") << fName << " Id " << GetId() << "]<" << GetCount() << "," << fLocker << "> AAW" << "\n");
#endif
}

//---- RWLock ------------------------------------------------------------

RWLock::RWLock() {
	CREATERWLOCK(fLock);
}

RWLock::RWLock(const char * name, Allocator * a)
	: fName(name, -1, a) {
	CREATERWLOCK(fLock);
}

RWLock::RWLock(const RWLock &) {
	// private no op -> don't use this;
}

void RWLock::operator=(const RWLock &) {
	// private no op -> don't use this;
}

RWLock::~RWLock() {
	DELETERWLOCK(fLock);
}

void RWLock::Lock(bool reading) const {
	LOCKRWLOCK(fLock, reading);
	TRACE_LOCK_ACQUIRE(fName);
}

void RWLock::Unlock(bool reading) const {
	UNLOCKRWLOCK(fLock, reading);
	TRACE_LOCK_RELEASE(fName);
}

bool RWLock::TryLock(bool reading) const {
	bool hasLocked = TRYRWLOCK(fLock, reading);
#ifdef TRACE_LOCKS
	if (hasLocked) {
		TRACE_LOCK_ACQUIRE(fName);
	}
#endif
	return hasLocked;
}

//---- SimpleCondition ------------------------------------------------

SimpleCondition::SimpleCondition() {
	if ( !CREATECOND(fSimpleCondition) ) {
		SysLog::Error("CREATECOND failed");
	}
}

SimpleCondition::~SimpleCondition() {
	if ( !DELETECOND(fSimpleCondition) ) {
		SysLog::Error("DELETECOND failed");
	}
}

int SimpleCondition::Wait(SimpleMutex & m) {
	if (!CONDWAIT(fSimpleCondition, m.GetInternal())) {
		SysLog::Error("CONDWAIT failed");
	}
	return 0;
}

int SimpleCondition::TimedWait(SimpleMutex & m, long secs, long nanosecs) {
	return CONDTIMEDWAIT(fSimpleCondition, m.GetInternal(), secs, nanosecs);
}

int SimpleCondition::Signal() {
	return SIGNALCOND(fSimpleCondition);
}

int SimpleCondition::BroadCast() {
	return BROADCASTCOND(fSimpleCondition);
}

long SimpleCondition::GetId() {
	return (long)GetInternal();
}

//---- Condition ------------------------------------------------

Condition::Condition() {
	if ( !CREATECOND(fCondition) ) {
		SysLog::Error("CREATECOND failed");
	}
}

Condition::~Condition() {
	if ( !DELETECOND(fCondition) ) {
		SysLog::Error("DELETECOND failed");
	}
}

int Condition::Wait(Mutex & m) {
	StartTrace1(Condition.Wait, "Id: " << GetId() << " ThrdId: " << Thread::MyId());
	m.ReleaseForWait();
	if ( !CONDWAIT(fCondition, m.GetInternal()) ) {
		SysLog::Error("CONDWAIT failed");
	}
	m.AcquireAfterWait();
	return 0;
}

int Condition::TimedWait(Mutex & m, long secs, long nanosecs) {
	StartTrace1(Condition.TimedWait, "Id: " << GetId() << " ThrdId: " << Thread::MyId());
	m.ReleaseForWait();
	int ret = CONDTIMEDWAIT(fCondition, m.GetInternal(), secs, nanosecs);
	m.AcquireAfterWait();
	return ret;
}

int Condition::Signal() {
	StartTrace1(Condition.Signal, "Id: " << GetId() << " ThrdId: " << Thread::MyId());
	return SIGNALCOND(fCondition);
}

int Condition::BroadCast() {
	StartTrace1(Condition.BroadCast, "Id: " << GetId() << " ThrdId: " << Thread::MyId());
	return BROADCASTCOND(fCondition);
}

long Condition::GetId() {
	return (long)GetInternal();
}

////--- ThreadLocalData ------------------------------
//
//ThreadLocalData::ThreadLocalData()
//{
//	THRKEYCREATE(fKey, NULL);
//}
//
//ThreadLocalData::ThreadLocalData(int data)
//{
//	CreateKeyAndSetData(data);
//}
//
//ThreadLocalData::ThreadLocalData(long data)
//{
//	THRKEYCREATE(fKey, NULL);
//	SETTLSDATA(fKey, new Anything(data));
//}
//
//ThreadLocalData::ThreadLocalData(float data)
//{
//	CreateKeyAndSetData(data);
//}
//
//ThreadLocalData::ThreadLocalData(double data)
//{
//	CreateKeyAndSetData(data);
//}
//
//ThreadLocalData::ThreadLocalData(const char*, long len)
//{
//	CreateKeyAndSetData(data);
//}
//
//ThreadLocalData::ThreadLocalData(const String &data)
//{
//	CreateKeyAndSetData(data);
//}
//
//ThreadLocalData::ThreadLocalData(const void *data, long len)
//{
//	CreateKeyAndSetData(data);
//}
//
//ThreadLocalData::ThreadLocalData(IFAObject *data)
//{
//	CreateKeyAndSetData(data);
//}
//
//ThreadLocalData::ThreadLocalData(const Anything &data)
//{
//	CreateKeyAndSetData(data);
//}
//
//void ThreadLocalData::CreateKeyAndSetData(Anything data)
//{
//	if	( (THRKEYCREATE(fKey, NULL) != 0) ||
//		  (SETTLSDATA(fKey, &fData) != 0)
//		)
//	{
//		String msg("Failed to create Thread local data: <"); msg << data << ">" << SysLog::LastSysError();
//		SysLog::Error(msg);
//	}
//
//}
//
//
//ThreadLocalData::~ThreadLocalData()
//{
//	THRKEYDELETE(fKey);
//}
//
//
//ThreadLocalData &ThreadLocalData::operator=(int)
//{
//
//}
//
//ThreadLocalData &ThreadLocalData::operator=(long)
//{
//
//}
//
//ThreadLocalData &ThreadLocalData::operator=(float)
//{
//
//}
//
//ThreadLocalData &ThreadLocalData::operator=(double)
//{
//
//}
//
//ThreadLocalData &ThreadLocalData::operator=(const char*, long len=-1)
//{
//
//}
//
//ThreadLocalData &ThreadLocalData::operator=(const String&)
//{
//
//}
//
//ThreadLocalData &ThreadLocalData::operator=(const void*, long len)
//{
//
//}
//
//ThreadLocalData &ThreadLocalData::operator=(IFAObject *)
//{
//
//}
//
//ThreadLocalData &ThreadLocalData::operator=(const Anything &any)
//{
//
//}
//
//ThreadLocalData &ThreadLocalData::operator=(const ROAnything &any)
//{
//
//}
//
//
////: returns a const char * representation of the implementation if any is set else the default
//// this method doesn't copy memory
//const char *ThreadLocalData::AsCharPtr(const char *dflt) const
//{
//
//}
//
////: returns a String representation of the implementation if any is set else the default, the string copies memory
//// beware of temporaries
//String ThreadLocalData::AsString(const char *dflt) const
//{
//
//}
//
//
////: returns a long representation of the implementation if possible else the default
//long ThreadLocalData::AsLong(long dflt) const
//{
//
//}
//
//
////: returns a bool representation of the implementation if possible else the default
//bool ThreadLocalData::AsBool(bool dflt) const
//{
//
//}
//
//
////: returns a double representation of the implementation if possible else the default
//double ThreadLocalData::AsDouble(double dlft) const
//{
//
//}
//
//
////: returns a IFAObject representation of the implementation if possible else the default
//IFAObject *ThreadLocalData::AsIFAObject(IFAObject *dflt) const
//{
//
//}
