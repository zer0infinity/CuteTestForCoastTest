/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _THREADS_H
#define _THREADS_H

#include "IFAObject.h"
#include "MT_Storage.h"
#include "Anything.h"

class Thread;

/*! Implementation of Dijkstra semaphore
This is a simple wrapper for native semaphore implementations
*/
class EXPORTDECL_MTFOUNDATION Semaphore
{
public:
	Semaphore(unsigned i_nCount);
	~Semaphore();

	/*! Acquires the semaphore, eg. decreases the internal counter if it is greater than 0. This is equivalent to
		Dijkstras DOWN method
		This is a blocking call, if the the count is already 0, it blocks until someone releases it
		\return true if count could be decreased */
	bool Acquire();

	/*! tries to acquire the semaphore.
		Does the same as the Acquire() function except blocking if the count could not be decreased
		\return true if count could be decreased, false if we could not or an error occured */
	bool TryAcquire();

	/*! Releases the semaphore, eg. increases the internal count and signals a waiting thread. This is equivalent to
		Dijkstras UP method */
	void Release();

	/*! Returns the actual value of the semaphore without altering its value. If the seamphore is locked, the return value
		is either zero or negative. The absolute value of the methods return code indicates the threads waiting for the
		semaphore. */
	int GetCount(int &svalue);

	//!dummy method to prevent optimizing compilers from optimizing away unused variables
	void Use() const { }

private:
	SEMA fSemaphore;

private:
	// disabled standard functions (move to public section if implemented)
	Semaphore (const Semaphore &rSemaphore);  // copy constructor
	void operator= (const Semaphore &rSemaphore);  // assignment
};

//!helper class to Acquire and Release Semaphores automatically in scope
class EXPORTDECL_MTFOUNDATION SemaphoreEntry
{
public:
	/*! Acquires the semaphore
		\param i_aSemaphore the semaphore object to acquire and release */
	SemaphoreEntry(Semaphore &i_aSemaphore);

	//! Releases the semaphore.
	~SemaphoreEntry();

private:
	//! A reference to the related semaphore.
	Semaphore &fSemaphore;
};

//---- Mutex ------------------------------------------------------------
/*! <b>mutual exclusion lock, wrapper for nativ system service</b>
recursive call from the same thread means deadlock! */
class EXPORTDECL_MTFOUNDATION SimpleMutex
{
	friend class SimpleCondition;
public:
	/*! create mutex with names to ease debugging of locking problems
		\param name a name to identify the mutex when tracing locking problems
		\param a allocator used to allocate the storage for the name */
	SimpleMutex(const char *name, Allocator *a = Storage::Global());
	~SimpleMutex();

	/*! tries to acquire the mutex for this thread, blocks the caller if already locked (by another thread)
		acquires the mutex for the calling thread; this is NOT recursive, if you call lock twice for the same thread it deadlocks */
	void Lock();

	/*! releases the mutex */
	void Unlock();

	/*! tries to acquire the mutex if possible; bails out without locking if already locked
		\return false when already locked */
	bool TryLock();

	/*! returns native reprentation of mutex hidden by macro MUTEXPTR */
	MUTEXPTR GetInternal() const {
		return GETMUTEXPTR(fMutex);
	}

private:
	//! internal representation of mutex
	MUTEX fMutex;

	//! the mutexs name, this is handy for tracing locking problems
	String fName;

	//!standard constructor prohibited
	SimpleMutex();
	//!standard copy constructor prohibited
	SimpleMutex(const SimpleMutex &);
	//!standard assignement operator prohibited
	void operator=(const SimpleMutex &);
};

/*! <b>mutual exclusion lock, wrapper for nativ system service adding recursive lock feature</b>
it is possible to call lock from the same thread without deadlock.
\note you have to call unlock as many times as you have called lock */
class EXPORTDECL_MTFOUNDATION Mutex
{
	friend class Condition;
	friend class MutexInitializer;
	friend class ThreadsTest;
	friend class ContextTest;
public:
	/*! create mutex with names to ease debugging of locking problems
		\param name a name to identify the mutex when tracing locking problems
		\param a allocator used to allocate the storage for the name */
	Mutex(const char *name, Allocator *a = Storage::Global());
	~Mutex();

	/*! tries to acquire the mutex for this thread, blocks the caller if already locked (by another thread)
		acquires the mutex for the calling thread; this is recursive, if you call lock twice for the same thread
		a count is incremented
		\note you should call unlock as many times as you called lock */
	void Lock();

	/*! releases the mutex */
	void Unlock();

	/*! tries to acquire the mutex if possible; bails out without locking if already locked
		\return false when already locked */
	bool TryLock();
	/*! return status of lock without locking overhead, useful for
		releasing, and reaquiring when lock */
	bool IsLockedByMe() {
		return GetCount() > 0;
	}
	/*! returns native representation of mutex hidden by macro MUTEXPTR
		\return native representation of mutex */
	MUTEXPTR GetInternal() const {
		return GETMUTEXPTR(fMutex);
	}

	//! resets thread id before Wait on a condition is called
	void ReleaseForWait();
	//! restores thread id after returning from a wait call
	void AcquireAfterWait();

	static THREADKEY fgCountTableKey;

protected:
	//!returns thread specific mutex lock count
	long GetCount();
	//!sets thread specific mutex lock count
	bool SetCount(long);

	//!get mutex 'Id' (name) mainly for Tracing
	const String &GetId();

	//!global mutex id counter
	static long fgMutexId;
	//!guard for global mutex id counter
	static SimpleMutex *fgpMutexIdMutex;

	//!this mutex unique id
	String fMutexId;
#if defined(WIN32) && defined(TRACE_LOCK_UNLOCK)
	String fMutexIdTL;
#endif

private:
	//! internal representation of mutex
	MUTEX fMutex;
	//!thread id of the lock holder
	long fLocker;

	//! the mutexs name, this is handy for tracing locking problems
	String fName;

	//!standard constructor prohibited
	Mutex();
	//!standard copy constructor prohibited
	Mutex(const Mutex &);
	//!standard assignement operator prohibited
	void operator=(const Mutex &);
};

//---- SimpleCondition --------------------------------------------------------------
//!native condition variable api wrapper implementing the common condition variable operations wait, signal and broadcast
class EXPORTDECL_MTFOUNDATION SimpleCondition
{
public:
	//!creates a system dependent condition variable
	SimpleCondition();
	//!destroys the system dependent condition variable
	~SimpleCondition();

	/*! waits for the condition to be signaled atomicly releasing the mutex while waiting and reaquiring it when returning from the wait
		\param m the mutex which is used with the condition; it must be locked, since it will be unlocked
		\return system dependent return value; zero if call was ok */
	int Wait(SimpleMutex &m);

	/*! waits for the condition to be signaled at most secs seconds and nanosecs nanoseconds if available on the plattform. After this time we block on the SimpleMutex until we can lock it before leaving the function
		\param m the mutex which is used with the condition; it must be locked, since it will be unlocked
		\param secs timeout to wait in seconds
		\param nanosecs timeout to wait in nanoseconds (10e-9)
		\return system dependent return value; zero if call was ok */
	int TimedWait(SimpleMutex &m, long secs, long nanosecs = 0);
	//!access the system dependent handle to the condition variable
	CONDPTR GetInternal() {
		return GETCONDPTR(fSimpleCondition);
	}
	//!signal the condition; do it while protected through the associated mutex, then unlock the mutex; zero or one waiting thread will be woken up
	int Signal();
	//!broadcasts the condition; do it while protected through the associated mutex, then unlock the mutex; all waiting thread will eventually be woken up
	int BroadCast();

private:
	//!get internal 'Id' mainly for Tracing
	long GetId();

	//!the system dependent condition variable handle
	COND fSimpleCondition;
};

//---- Condition --------------------------------------------------------------
//!native condition variable api wrapper implementing the common condition variable operations wait, signal and broadcast
class EXPORTDECL_MTFOUNDATION Condition
{
public:
	//!creates a system dependent condition variable
	Condition();
	//!destroys the system dependent condition variable
	~Condition();

	/*! waits for the condition to be signaled atomicly releasing the mutex while waiting and reaquiring it when returning from the wait
		\param m the mutex which is used with the condition; it must be locked, since it will be unlocked
		\return system dependent return value; zero if call was ok */
	int Wait(Mutex &m);
	/*! waits for the condition to be signaled at most secs seconds and nanosecs nanoseconds if available on the plattform
		\param m the mutex which is used with the condition; it must be locked, since it will be unlocked
		\param secs timeout to wait in seconds
		\param nanosecs timeout to wait in nanoseconds (10e-9)
		\return system dependent return value; zero if call was ok */
	int TimedWait(Mutex &m, long secs, long nanosecs = 0);
	//!access the system dependent handle to the condition variable
	CONDPTR GetInternal() {
		return GETCONDPTR(fCondition);
	}
	//!signal the condition; do it while protected through the associated mutex, then unlock the mutex; zero or one waiting thread will be woken up
	int Signal();
	//!broadcasts the condition; do it while protected through the associated mutex, then unlock the mutex; all waiting thread will eventually be woken up
	int BroadCast();

private:
	//!get internal 'Id' mainly for Tracing
	long GetId();

	//!the system dependent condition variable handle
	COND fCondition;
};

//---- SimpleMutexEntry -------------------------------------------------------------
//!syntactic sugar to ease acquiring and releasing a mutex in one scope
class EXPORTDECL_MTFOUNDATION SimpleMutexEntry
{
public:
	//!acquires the mutex in the constructor
	SimpleMutexEntry(SimpleMutex &m, bool = true) : fMutex(m) {
		fMutex.Lock();
	}
	//!realeases the mutex in destructor
	~SimpleMutexEntry() {
		fMutex.Unlock();
	}
	//!dummy method to prevent optimizing compilers from optimizing away unused variables
	void Use() const
	{ }
private:
	//!reference to Mutex Object used by this object
	SimpleMutex	&fMutex;
};

//---- MutexEntry -------------------------------------------------------------
//!syntactic sugar to ease acquiring and releasing a mutex in one scope
class EXPORTDECL_MTFOUNDATION MutexEntry
{
public:
	//!acquires the mutex in the constructor
	MutexEntry(Mutex &m) : fMutex(m) {
		fMutex.Lock();
	}
	//!realeases the mutex in destructor
	~MutexEntry() {
		fMutex.Unlock();
	}
	//!dummy method to prevent optimizing compilers from optimizing away unused variables
	void Use() const
	{ }
private:
	//!reference to Mutex Object used by this object
	Mutex	&fMutex;
};

//---- CleanupHandler ------------------------------------------------------------
//!subclasses may be defined to perform cleanup in thread specific storage while thread is still alive. CleanupHandlers are supposed to be singletons..
class EXPORTDECL_MTFOUNDATION CleanupHandler
{
public:
	//!Constructor does nothing
	CleanupHandler() {}
	virtual ~CleanupHandler() {}

	bool Cleanup() {
		return DoCleanup();
	}

protected:
	//!subclasses implement cleanup of thread specific storage
	virtual bool DoCleanup() = 0;
};

//---- AllocatorUnref ------------------------------------------------------------
//!<b>utility class used for proper destruction of thread local storage</b>
/*!destruction of thread local store belonging to a thread has to take place at the very
last moment possible; but it has to be done; it is the only task of this class */
class EXPORTDECL_MTFOUNDATION AllocatorUnref
{
public:
	//!stores away the thread the object is working for
	AllocatorUnref(Thread *t);
	//!destroys the thread local store of the fThread
	~AllocatorUnref();
protected:
	//!the thread which is deleted
	Thread *fThread;
};

//---- ThreadObserver ------------------------------------------------------------
//!<b>observer interface for observing Thread objects</b>
class EXPORTDECL_MTFOUNDATION ThreadObserver
{
public:
	//!does nothing
	ThreadObserver() { }
	//!does nothing
	virtual ~ThreadObserver() { }

	/*! interface which must be implemented if you want to be notified when thread t's state changes
		\param t the thread that changes its state
		\param roaStateArgs arguments that describe the change; EThreadState in subslot "ThreadState" or ERunningState in subslot "RunningState". "Old" and "New" slots tell you about corresponding states. */
	virtual void Update(Thread *t, ROAnything roaStateArgs) = 0;
};

//---- Thread ------------------------------------------------------------
//!<b>thread abstraction implementing its own thread state model using EThreadState and the available nativ thread api</b>
/*!
this class implements the thread abstraction ( its own thread of control ) using the system dependent thread api available.<br>
To ease its use we have defined a state machine which let clients query a thread object about the state.<br>
With this means it is possible to reliably control starting and stopping of a thread */
class EXPORTDECL_MTFOUNDATION Thread : public NamedObject
{
public:
	/*! possible thread states; there is an implicit ordering in these states<br> eThreadCreated < eStartRequested < eStarted < eRunning < eTerminationRequested < eTerminated. Only well defined transitions are possible
		These states exist:<br>
		<b>eCreated</b><br>thread exists; but Start has not yet been called<p>
		<b>eStartRequested</b><br>Thread::Start has been called, thread is beeing initialized for start<p>
		<b>eStarted</b><br>Thread::Start has been called, thread was successfully initialized; the new thread of control is forked<p>
		<b>eRunning</b><br>Thread::Run has been entered in its own thread of control<p>
		<b>eTerminationRequested</b><br>thread is still in Thread::Run but somebody has termination signaled; thread will eventually terminate its own thread of control<p>
		<b>eTerminated</b><br>Thread::Run has been left and its own thread of control is terminated. The object still exists and can be restarted now<p>
		The following <b>state transitions</b> from current to next state are possible:<br>
		<table border=1 align=center>
		<tr><th align=center>CurrentState\NextState</th><th>eCreated</th><th>eStartRequested</th><th>eStarted</th><th>eRunning</th><th>eTerminationRequested</th><th>eTerminated</th></tr>
		<tr><td>eCreated</td><td align=center>1</td><td align=center>1</td><td align=center>0</td><td align=center>0</td><td align=center>0</td><td align=center>1</td></tr>
		<tr><td>eStartRequested</td><td align=center>0</td><td align=center>1</td><td align=center>1</td><td align=center>0</td><td align=center>0</td><td align=center>0</td></tr>
		<tr><td>eStarted</td><td align=center>0</td><td align=center>0</td><td align=center>1</td><td align=center>1</td><td align=center>0</td><td align=center>0</td></tr>
		<tr><td>eRunning</td><td align=center>0</td><td align=center>0</td><td align=center>0</td><td align=center>1</td><td align=center>1</td><td align=center>0</td></tr>
		<tr><td>eTerminationRequested</td><td align=center>0</td><td align=center>0</td><td align=center>0</td><td align=center>0</td><td align=center>1</td><td align=center>1</td></tr>
		<tr><td>eTerminated</td><td align=center>0</td><td align=center>1</td><td align=center>0</td><td align=center>0</td><td align=center>0</td><td align=center>1</td></tr>
		</table> */
	enum EThreadState {
		eCreated,
		eStartRequested,
		eStartInProgress,
		eStarted,
		eRunning,
		eTerminationRequested,
		eTerminatedRunMethod,
		eTerminated
	};

	//!predefined possible running states for specialized application
	enum ERunningState {
		//! Thread is running and ready to do sthg.
		eReady,
		//! Thread is doing sthg.
		eWorking
	};

	/*! Constructor geared towards solaris thread api
		\param name identify the thread with a more or less unique name, used for debugging etc
		\param daemon sets the daomon flag
		\param detached if set to true the thread is not joinable by other threads
		\param suspended creates this thread in suspended mode
		\param bound bounds this thread to a lwp */
	Thread(const char *name, bool daemon = false, bool detached = true, bool suspended = false, bool bound = false, Allocator *a = 0);

	/*! since it is not possible to terminate a thread in the superclass destructor, because most of the object has gone by that time,
		we provide some infrastructure through the CheckState API to check and wait for certain events to happen */
	virtual ~Thread();

	/*! naming support getting name
		\param str string which gets back the threads name
		\return true in case the name was set */
	virtual bool GetName(String &str) const;

	/*! naming support getting name */
	virtual const char *GetName() const {
		return fThreadName;
	}

	/*! naming support setting name
		\param name name to set */
	virtual void SetName(const char *name);

	//! custom initialization for array allocated threads
	virtual int Init(ROAnything args);

	/*! starts a thread, when this method returns the thread might or might not already be running
		\param wdallocator the storage allocator used in the thread to be started
		\param args the arguments given to the DoStartRequestedHook
		\return true if the thread is about to start and fState == eStarted; false if DoStartRequestedHook returns false and fState == eTerminated */
	bool Start(Allocator *wdallocator = Storage::Global(), ROAnything args = ROAnything());

	/*! blocks the caller until thread has reached the requested or a higher state
		\param state EThreadState to check if the thread has reached or passed it
		\param timeout timeout for wait in seconds
		\param nanotimeout timeout to wait in nanoseconds (10e-9)
		\return true if state was reached otherwise false; caution if you check for e.g. eThreadStarted and the state is already eThreadRunning returns also true */
	bool CheckState(EThreadState state, long timeout = 0, long nanotimeout = 0);

	/*! blocks the caller until thread has reached the requested state
		\param state ERunningState to check if the thread has reached or passed it
		\param timeout timeout for wait in seconds
		\param nanotimeout timeout to wait in nanoseconds (10e-9)
		\return true if state was reached otherwise false; */
	bool CheckRunningState(ERunningState state, long timeout = 0, long nanotimeout = 0);

	/*! returns the threads current state; if using trylock and lock is already set returns dfltState
		\param trylock set to true if you want to use TryLock instead of blocking on the state mutex
		\param dfltState set value which should be returned in case the TryLock fails
		\return current state of thread or dfltState when TryLock fails */
	EThreadState GetState(bool trylock = false, EThreadState dfltState = eRunning);

	//!tests if the thread object is still valid
	bool IsAlive() {
		return (fSignature == 0x0f0055AA);
	}

	//!tests if in state eReady; returns false if mutex locked
	bool IsReady();

	//!tests if in state eWorking; returns true if mutex locked - assuming something is going on
	bool IsWorking();

	//!tests if in state eRunning == fState
	bool IsRunning();

	//!Try to set Ready state
	bool SetReady(ROAnything args = ROAnything());

	//!Try to set Working state
	bool SetWorking(ROAnything args = ROAnything());

	/*! terminates the thread of control; blocks its caller
		\param timeout the caller is blocked until the state eTerminated is reached or the timeout period has elapsed. If timeout==0 the timeout period is ignored
		\param args the arguments to the termination
		\return true if eTerminated was reached; false if timeout period has elapsed without reaching the state */
	bool Terminate(long timeout = 0, ROAnything args = ROAnything());

	//!adds ThreadObserver that is notifyied in DoSignalThreadEnd
	virtual void AddObserver(ThreadObserver *to);

	//!returns the id of this thread
	int  GetId()	{
		return (int)fThreadId;
	}

	//! blocks the calling thread for delay secs and nanodelay nanosecs
	static void Wait(long delay, long nanodelay = 0);

	//! returns id of the calling thread
	static long MyId();

	//! returns the number of active thread objects - still in IntRun() method
	static long NumOfThreads();

	//! allows clients to register an additional cleanup handler, (which is put into a list)
	static bool RegisterCleaner(CleanupHandler *);

	static THREADKEY fgCleanerKey;
#if defined(WIN32)
	friend void  ThreadWrapper(void *);
#else
	friend void *ThreadWrapper(void *);
#endif
	friend class ThreadInitializer;

protected:
	//!sets internal states and calls Run
	void IntRun();

	bool IntCheckState(EThreadState state, long timeout = 0, long nanotimeout = 0);

	//!this method gets called from the threads callback function
	virtual void Run() = 0;

	//! hook to let the observers of the thread know that a state has changed
	void NotifyAll(Anything evt);

	//! broadcast an state change to the Observers and broadcast the fStateCond for clients initiating the state change
	void BroadCastEvent(Anything evt);

	//!dispatch to subclass hooks for thread state changes
	bool CallStateHooks(EThreadState state, ROAnything args);

	/*! Use current list of cleanup handlers to cleanup the thread specific storage and then reset to an empty list. This method is the last point where we can modify state values of the thread terminating. After TLS cleanup is done, the state is set to eTerminated and fThreadId will be reset to 0.
		\return true in case of success, false otherwise */
	bool CleanupThreadStorage();

	/*! This hook gets called before an operating system thread will be started. One can decide - returning true or false - if an operating system thread should really be started or not.
		This method is useful when we want to initialize memory dependant things because the fAllocator member is now initialized. The GetId() function will not return a valid id because no real thread was started yet.
		\param args arguments passed when calling the Start() method
		\return true in case the Start() method should continue to attach us to a real operating system thread, false otherwises */
	virtual bool DoStartRequestedHook(ROAnything args);

	/*! This hook gets called when an operating system thread could be started. The IntRun() method and its internal call to Run() will appear after we leave this function so it is guaranteed that we do not need to lock anything which could be used already in the Run() method.
		This method is useful when we want to initialize memory dependant things because the fAllocator member is now initialized. In addition - to DoStartRequestedHook - the GetId() function also returns a valid threadid.
		\param args arguments passed when calling the Start() method */
	virtual void DoStartedHook(ROAnything args);

	//!subclass hook
	virtual void DoTerminationRequestHook(ROAnything args);

	/*! subclass hook to catch event when thread specific (derived) Run() method returns */
	virtual void DoTerminatedRunMethodHook();

	/*! subclass hook to catch event when IntRun() method returns, eg. OS specific thread terminates
		\note From now on, every Run() related code has finished executing */
	virtual void DoTerminatedHook();

	//!dispatch to subclass hooks for running state changes
	void CallRunningStateHooks(ERunningState state, ROAnything args);

	//!subclass hook
	virtual void DoReadyHook(ROAnything);

	//!subclass hook
	virtual void DoWorkingHook(ROAnything);

	//! 1st instance var (i.e. is destroyed last) used to unregister allocator *after* its no longer required
	AllocatorUnref fAllocCleaner;

	//!allocator for thread local storage
	Allocator *fAllocator;

	//!guard that locks observer list
	SimpleMutex fObserversMutex;
	//!this threads observers
	Anything fObservers;
	//!system dependent thread id
	THREAD fThreadId;
	//! thread id of parent which constructed us
	long fParentThreadId;

	//!solaris like thread flags; not very useful on all systems
	bool fDaemon, fDetached, fSuspended, fBound;

	// deletes running thread (but not the object)
	void Exit(int code);

	// some synchronization variables to make life easier for thread users guard for fState flag
	SimpleMutex fStateMutex;

	//!synchronization condition for checking thread states
	SimpleCondition fStateCond;

	//!thread count variable
	static long fgNumOfThreads;

	//!guard of thread count
	static SimpleMutex *fgpNumOfThreadsMutex;

private:
	/*! internal method implementing the state transition matrix
		\param state the next state wished
		\param args the arguments for the transition
		\return true if transition was possible; else false
		\note this internal method is not to be touched, it implements the state transition matrix and calls IntSetState if ok */
	bool SetState(EThreadState state, ROAnything args = ROAnything());

	/*! internal method setting the state and calling the hook methods
		\param state the next state accepted
		\param args the arguments for the transition forwarded to the hook methods
		\return returns always true apart from the DoStartRequestHook that can abort the transition
		\note internal method; it calls the state hook with the new state and the args<br>if successful it changes the state to the new state and Broadcasts the state change to observers and waiters of the state condition */
	bool IntSetState(EThreadState state, ROAnything args = ROAnything());

	/*! internal method setting the running state
		\param state the running state wished
		\param args the arguments to the state change
		\return returns true if the state change was possible; returns false if not in EThreadState::eRunning or the running state is already the state wished
		\note internal method setting the running state; if successful it broadcasts the state change to observers and waiters of the state condition */
	bool SetRunningState(ERunningState state, ROAnything args);

	/*! cloning interface of object, disallow clones for now
		\return NULL in any case */
	virtual IFAObject *Clone() const {
		return (IFAObject *)0;
	}

	//!state variable defining the threads current running state
	ERunningState fRunningState;
	//!state variable defining the threads state
	EThreadState fState;

	//! signature to check alive state of thread even the object itself got destructed
	long fSignature;

	//! the name of the thread
	String fThreadName;

	// a temporary placeholder for passing arguments to DoStartedHook
	Anything fanyArgTmp;

	friend class AllocatorUnref;
#if defined(WIN32)
	friend BOOL WINAPI DllMain(HANDLE, DWORD, LPVOID);
	friend void EXPORTDECL_MTFOUNDATION TerminateKilledThreads();
#endif
};

//---- RWLock ------------------------------------------------------------
//! read/write lock, wrapper for nativ system service
class EXPORTDECL_MTFOUNDATION RWLock
{
public:
	/*! creates system dependent read/write lock
		\param name a name to identify the mutex when tracing locking problems
		\param a allocator used to allocate the storage for the name */
	RWLock(const char *name, Allocator *a = Storage::Global());
	//!deletes system dependent read/write lock
	~RWLock();
	//!locks system dependent lock for reading or writing according to the reading flag
	void Lock(bool reading = true) const;
	//!releases system dependent lock for reading or writing according to the reading flag
	void Unlock(bool reading = true) const;
	//!tries to lock system dependent lock for reading or writing according to the reading flag; bails out returning false without blocking if not possible
	bool TryLock(bool reading = true) const;
	//!access the internal system dependent representation of the read/write lock
	RWLOCKPTR GetInternal() const {
		return GETRWLOCKPTR(fLock);
	}
private:
	//!handle to system dependent read/write lock
	RWLOCK	fLock;
	//!the name of the read/write lock to ease debugging locking problems
	String fName;

	//!standard constructor prohibited
	RWLock();
	//!standard copy constructor prohibited
	RWLock(const RWLock &);
	//!standard assignment operator prohibited
	void operator=(const RWLock &);
};

//---- RWLockEntry -------------------------------------------------------------
//!syntactic sugar to ease acquiring and releasing a read/write lock in one scope
class EXPORTDECL_MTFOUNDATION RWLockEntry
{
public:
	//!acquires the read/write lock in the constructor for reading or writing
	RWLockEntry(const RWLock &m, bool reading = true) : fLock(m), fReading(reading) {
		fLock.Lock(fReading);
	}
	//!realeases the read/write lock in destructor
	~RWLockEntry() {
		fLock.Unlock(fReading);
	}
	//!dummy method to prevent optimizing compilers from optimizing away unused variables
	void Use() const
	{ }
private:
	//!reference to Read/Write Lock Object used by this object
	const RWLock	&fLock;
	//!remembers if this lock was for reading or writing
	bool	fReading;
};

#endif
