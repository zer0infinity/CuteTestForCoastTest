/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _SYSTEMAPI_H
#define _SYSTEMAPI_H

#include "config_mtfoundation.h"
#include "foundation.h"

#if defined(WIN32)
#include <climits>
#include <process.h>
#include <winbase.h>
#include <time.h>

#define TIMEOUTCODE WAIT_TIMEOUT
// define how long to wait in trylocks
#define	TRYLOCKTIMEOUT	0L

int EXPORTDECL_MTFOUNDATION doTimedWait(HANDLE c, HANDLE m, long s, long n);
int EXPORTDECL_MTFOUNDATION doWaitObject(HANDLE h, HANDLE m, long time);

//#define TRACE_LOCK_UNLOCK	1
#ifdef TRACE_LOCK_UNLOCK
#define fooRELOBJECT(hh)	cerr << "##ID: " << (long)hh << " --##" << endl;
#define fooWAITOBJECT(hh)	cerr << "##ID: " << (long)hh << " ..##" << endl;
#define fooLOCKOBJECT(hh)	cerr << "##ID: " << (long)hh << " ++##" << endl;
#else
#define fooRELOBJECT(hh)
#define fooWAITOBJECT(hh)
#define fooLOCKOBJECT(hh)
#endif

//--- mutex macros
#define MUTEX						HANDLE
#define MUTEXPTR					HANDLE*
#define GETMUTEXPTR(mutex)			(MUTEXPTR)&mutex
#define CREATEMUTEX(mutex, retcode)	((mutex= ::CreateMutex(NULL, false, NULL)) != NULL)
#define DELETEMUTEX(mutex, retcode)	(::CloseHandle(mutex) != 0)
#define LOCKMUTEX(mutex, retcode)	(doWaitObject(mutex, 0, INFINITE) == WAIT_OBJECT_0)
#ifdef TRACE_LOCK_UNLOCK
#define UNLOCKMUTEX(mutex, retcode)	if (!::ReleaseMutex(mutex)) cerr << "error in releasing mutex" << endl;	fooRELOBJECT(mutex);
#else
#define UNLOCKMUTEX(mutex, retcode)	(::ReleaseMutex(mutex) != 0)
#endif
#define TRYLOCK(mutex, retcode)		(doWaitObject(mutex, 0, TRYLOCKTIMEOUT) == WAIT_OBJECT_0)

//--- sema macros
#define SEMA						HANDLE
#define SEMAPTR						HANDLE*
#define GETSEMAPTR(sema)			(HANDLE*)&sema
#define CREATESEMA(sema, count)		((sema= ::CreateSemaphore(NULL, count, LONG_MAX, NULL)) != NULL)
#define DELETESEMA(sema)			(::CloseHandle(sema) != 0)
#define LOCKSEMA(sema)				(doWaitObject(sema, 0, INFINITE) == WAIT_OBJECT_0)
#ifdef TRACE_LOCK_UNLOCK
#define UNLOCKSEMA(sema)			if (!::ReleaseSemaphore(sema, 1, NULL)) cerr << "error in releasing semaphore" << endl; fooRELOBJECT(sema);
#else
#define UNLOCKSEMA(sema)			(::ReleaseSemaphore(sema, 1, NULL) != 0)
#endif
#define TRYSEMALOCK(sema)			(doWaitObject(sema, 0, TRYLOCKTIMEOUT) == WAIT_OBJECT_0)
#define GETSEMACOUNT(sema, count)	(sem_getvalue((sem_t*) &sema, &count))

//--- rwlock macros
#define RWLOCK						HANDLE
#define RWLOCKPTR					HANDLE*
#define GETRWLOCKPTR(rwlock)		(HANDLE*)&rwlock
#define CREATERWLOCK(rwlock)		((rwlock= ::CreateMutex(NULL, false, NULL)) != NULL)
#define DELETERWLOCK(rwlock)		(::CloseHandle(rwlock) != 0)
#define LOCKRWLOCK(rwlock, r)		(doWaitObject(rwlock, 0, INFINITE) == WAIT_OBJECT_0)
#ifdef TRACE_LOCK_UNLOCK
#define UNLOCKRWLOCK(rwlock, r)		if (!::ReleaseMutex(rwlock)) cerr << "error in releasing rwlock(mutex)" << endl; fooRELOBJECT(rwlock);
#else
#define UNLOCKRWLOCK(rwlock, r)		(::ReleaseMutex(rwlock) != 0)
#endif
#define TRYRWLOCK(rwlock, reading)	(doWaitObject(rwlock, 0, TRYLOCKTIMEOUT) == WAIT_OBJECT_0)

//--- thread macros
#define THREAD						HANDLE
#define STARTTHREAD(o, f, id, wrapper)		(_beginthread( wrapper, 0, (void *) o  ) != -1)
#define DELETETHREAD()				_endthread()
#define THRID()						GetCurrentThreadId()
#define THRKILL(tid, sig)			0
#define THRDETACH(id)
#define THRSETSIGMASK(A,B,C)
#define THRSETCONCURRENCY(a)

//--- condition macros
#define COND						HANDLE
#define CONDPTR						HANDLE*
#define GETCONDPTR(cond)			(HANDLE*)&cond
#define CREATECOND(cond)			((cond= ::CreateEvent(NULL, true, false, NULL)) != NULL)
#define DELETECOND(cond)			(::CloseHandle(cond) != 0)
#define SIGNALCOND(cond)			(::PulseEvent(cond) != 0)
#define BROADCASTCOND(cond)			(::PulseEvent(cond) != 0)
#define CONDWAIT(cond, m)			(doTimedWait(cond, *m, INFINITE, 0) == WAIT_OBJECT_0)
#define CONDTIMEDWAIT(c, m, s, n)	doTimedWait(c, *m, s, n)

extern "C" {
	void ThreadWrapper(void *thread);
}

#define THREADWRAPPERFUNCPROTO(wrapper)	void (*wrapper)(void *)
#define THREADWRAPPERFUNCDECL(name, param) void name(void *param)
#define THREADWRAPPERFUNC(thread)	THREADWRAPPERFUNCDECL(ThreadWrapper, thread)
#define THWRAPRETURNVAL

#define THREADKEY 							DWORD
#define THRKEYCREATE(key, destFct)			((key=TlsAlloc())==0xFFFFFFFF)
#define THRKEYDELETE(key) 					(TlsFree(key)!=0L)
#define SETTLSDATA(key, data)				(TlsSetValue(key, (void*)data) != 0)
#define GETTLSDATA(key, data, dataType)		((data = (dataType*)TlsGetValue(key)), (GetLastError()!=NO_ERROR?(data=0):data))

#else
#if defined(__sun) && !defined(_POSIX_THREADS)

#include <thread.h>

#define TIMEOUTCODE ETIME
extern "C" {
	void *ThreadWrapper(void *thread);
	bool doTryLock(mutex_t *, int &);
	bool doTryRWLock(rwlock_t *, bool);
	int  doTimedWait(cond_t *, mutex_t *, long s, long n);
	bool doStartThread(void *obj, bool *b, unsigned int *thrId, void * (*wrapper)(void *));
	void doRWLock(rwlock_t *rw, bool reading);
}
#define THREADWRAPPERFUNCPROTO(wrapper)	void *(*wrapper)(void *)
#define THREADWRAPPERFUNCDECL(name, param)	void *name(void *param)
#define THREADWRAPPERFUNC(thread)	THREADWRAPPERFUNCDECL(ThreadWrapper, thread)
#define THWRAPRETURNVAL				return 0;

//--- thread macros
#define THREAD						thread_t
#define STARTTHREAD(o, f, id, wrapper)		doStartThread(o,f, id, wrapper)
#define DELETETHREAD()				thr_exit((void*)0)
#define THRID()						thr_self()
#define THRKILL(tid, sig)			thr_kill(tid, sig)
#define THRDETACH(id)				pthread_detach(id)
#define THRSETSIGMASK(A,B,C)		thr_sigsetmask((A),(B),(C))
#define THRSETCONCURRENCY(a)		thr_setconcurrency(a)

//--- mutex macros
#define MUTEX						mutex_t
#define MUTEXPTR					mutex_t*
#define GETMUTEXPTR(mutex)			(mutex_t*)&mutex
#define CREATEMUTEX(mutex, retcode)	( ( retcode = mutex_init(&mutex, USYNC_THREAD, 0) ) == 0 )
#define DELETEMUTEX(mutex, retcode)	( ( retcode = mutex_destroy(&mutex) ) == 0 )
#define LOCKMUTEX(mutex, retcode)	( ( retcode = mutex_lock((mutex_t*) &mutex) ) == 0 )
#define UNLOCKMUTEX(mutex, retcode)	( ( retcode = mutex_unlock((mutex_t*) &mutex) ) == 0 )
#define TRYLOCK(mutex, retcode)		doTryLock((mutex_t*) &mutex, retcode)

//--- sema macros
#define SEMA						sema_t
#define SEMAPTR						sema_t*
#define GETSEMAPTR(sema)			(sema_t*)&sema
#define CREATESEMA(sema, count)		(sema_init(&sema, count, USYNC_THREAD, 0) == 0)
#define DELETESEMA(sema)			(sema_destroy(&sema) == 0)
#define LOCKSEMA(sema)				(sema_wait((sema_t*) &sema) == 0)
#define UNLOCKSEMA(sema)			(sema_post((sema_t*) &sema) == 0)
#define TRYSEMALOCK(sema)			(sema_trywait((sema_t*)&sema) == 0)
#define GETSEMACOUNT(sema, count)	(sema_getvalue((sem_t*) &sema, &count))

//--- read write locks
#define RWLOCK						rwlock_t
#define RWLOCKPTR					rwlock_t*
#define GETRWLOCKPTR(rwlock)		(rwlock_t*)&rwlock
#define CREATERWLOCK(rwlock)		rwlock_init((rwlock_t*)&rwlock, USYNC_THREAD, 0)
#define DELETERWLOCK(rwlock)		rwlock_destroy((rwlock_t*)&rwlock)
#define LOCKRWLOCK(rwlock, r)		doRWLock((rwlock_t*)&rwlock, r)
#define UNLOCKRWLOCK(rwlock, r)		rw_unlock((rwlock_t*) &rwlock)
#define TRYRWLOCK(rwlock, r)		doTryRWLock((rwlock_t*) &rwlock, reading)

//--- condition macros
#define COND						cond_t
#define CONDPTR						cond_t *
#define GETCONDPTR(cond)			(cond_t*)&cond
#define CREATECOND(cond)			(cond_init(&cond, USYNC_THREAD, 0) == 0)
#define DELETECOND(cond)			(cond_destroy(&cond) == 0)
#define SIGNALCOND(cond)			(cond_signal(&cond) == 0)
#define BROADCASTCOND(cond)			(cond_broadcast(&cond) == 0)
#define CONDWAIT(cond, m)			(cond_wait(&cond, m) == 0)
#define CONDTIMEDWAIT(c, m, s, n)	doTimedWait(&c, m, s, n)

#define THREADKEY							thread_key_t
#define THRKEYCREATE(key, destFct)			thr_keycreate(&key, destFct)
#define THRKEYDELETE(key) 					(0)
#define SETTLSDATA(key, data)				(thr_setspecific(key, (void*)data) == 0)
#define GETTLSDATA(key, data, dataType)		(thr_getspecific(key, (void**)&data)?(data=0):data)
#else
// assume posix api (linux)
#include <pthread.h>
#include <semaphore.h>

#define TIMEOUTCODE  ETIMEDOUT
/*
 * Structure describing a read-write lock.
 */
typedef struct rwlock_tag {
	pthread_mutex_t     mutex;
	pthread_cond_t      read;           /* wait for read */
	pthread_cond_t      write;          /* wait for write */
	int                 valid;          /* set when valid */
	int                 r_active;       /* readers active */
	int                 w_active;       /* writer active */
	int                 r_wait;         /* readers waiting */
	int                 w_wait;         /* writers waiting */
} rwlock_t;

extern "C" {
	void *ThreadWrapper(void *thread);
	bool doTryLock(pthread_mutex_t *, int &);
	int  doTimedWait(pthread_cond_t *, pthread_mutex_t *, long s, long n);
	bool doStartThread(void *obj, bool *b, pthread_t *thrId, void * (*wrapper)(void *));
	void doRWLock(rwlock_t *rw, bool reading);
	bool doRWTrylock(rwlock_t *, bool);
	bool doRWUnlock(rwlock_t *, bool);
	/*
	 * Define read-write lock functions
	 */
	int rwl_init (rwlock_t *rwlock);
	int rwl_destroy (rwlock_t *rwlock);
	int rwl_readlock (rwlock_t *rwlock);
	int rwl_readtrylock (rwlock_t *rwlock);
	int rwl_readunlock (rwlock_t *rwlock);
	int rwl_writelock (rwlock_t *rwlock);
	int rwl_writetrylock (rwlock_t *rwlock);
	int rwl_writeunlock (rwlock_t *rwlock);
}
#define THREADWRAPPERFUNCPROTO(wrapper)	void *(*wrapper)(void *)
#define THREADWRAPPERFUNCDECL(name, param)	void *name(void *param)
#define THREADWRAPPERFUNC(thread)	THREADWRAPPERFUNCDECL(ThreadWrapper, thread)
#define THWRAPRETURNVAL				return 0;

//--- thread macros
#define THREAD						pthread_t
#define STARTTHREAD(o, f, id, wrapper)		doStartThread(o,f, id, wrapper)
#define DELETETHREAD()				pthread_exit((void*)0)
#define THRID()						(long)pthread_self()
#define THRKILL(tid, sig)			pthread_kill(tid, sig)
#define THRDETACH(id)				pthread_detach(id)
#define THRSETSIGMASK(A,B,C)		pthread_sigmask(A,B,C)

// workaround on SunOS 5.6: pthread library dos not define pthread_setconcurrency
// to use thr_setconcurrency we need to forward declare it
// > we can not include thread.h for its definition because it would double define things
#if defined(__sun) && (OS_RELMINOR == 6)
#ifdef __cplusplus
extern "C" {
#endif
	int thr_setconcurrency(int);
#ifdef __cplusplus
};
#endif
#define THRSETCONCURRENCY(a)	thr_setconcurrency(a)
#else
#define THRSETCONCURRENCY(a)	pthread_setconcurrency(a)
#endif

//--- mutex macros
#define MUTEX						pthread_mutex_t
#define MUTEXPTR					pthread_mutex_t*
#define GETMUTEXPTR(mutex)			(pthread_mutex_t*)&mutex
#define CREATEMUTEX(mutex, retcode)	( ( retcode = pthread_mutex_init(&mutex, 0) ) == 0 )
#define DELETEMUTEX(mutex, retcode)	( ( retcode = pthread_mutex_destroy(&mutex) ) == 0 )
#define LOCKMUTEX(mutex, retcode)	( ( retcode = pthread_mutex_lock((pthread_mutex_t*) &mutex) ) == 0 )
#define UNLOCKMUTEX(mutex, retcode)	( ( retcode = pthread_mutex_unlock((pthread_mutex_t*) &mutex) ) == 0 )
#define TRYLOCK(mutex, retcode)		doTryLock((pthread_mutex_t*)&mutex, retcode)

//--- sema macros
#define SEMA						sem_t
#define SEMAPTR						sem_t*
#define GETSEMAPTR(sema)			(sem_t*)&sema
#define CREATESEMA(sema, count)		(sem_init(&sema, 0, count) == 0)
#define DELETESEMA(sema)			(sem_destroy(&sema) == 0)
#define LOCKSEMA(sema)				(sem_wait((sem_t*) &sema) == 0)
#define UNLOCKSEMA(sema)			(sem_post((sem_t*) &sema) == 0)
#define TRYSEMALOCK(sema)			(sem_trywait((sem_t*)&sema) == 0)
#define GETSEMACOUNT(sema, count)	(sem_getvalue((sem_t*) &sema, &count))

//--- read write locks
#define RWLOCK						rwlock_t
#define RWLOCKPTR					rwlock_t*
#define GETRWLOCKPTR(rwlock)		(rwlock_t*)&rwlock
#define CREATERWLOCK(rwlock)		rwl_init((rwlock_t*)&rwlock)
#define DELETERWLOCK(rwlock)		rwl_destroy((rwlock_t*)&rwlock)
#define LOCKRWLOCK(rwlock, reading)	doRWLock((rwlock_t*)&rwlock, reading)
#define UNLOCKRWLOCK(rwlock, read)	doRWUnlock((rwlock_t*)&rwlock, read)
#define TRYRWLOCK(rwlock, reading)	doRWTrylock((rwlock_t*)&rwlock, reading)

//--- condition macros
#define COND						pthread_cond_t
#define CONDPTR						pthread_cond_t *
#define GETCONDPTR(cond)			(pthread_cond_t*)&cond
#define CREATECOND(cond)			(pthread_cond_init(&cond, 0) == 0)
#define DELETECOND(cond)			(pthread_cond_destroy(&cond) == 0)
#define SIGNALCOND(cond)			(pthread_cond_signal(&cond) == 0)
#define BROADCASTCOND(cond)			(pthread_cond_broadcast(&cond) == 0)
#define CONDWAIT(cond, m)			(pthread_cond_wait(&cond, m) == 0)
#define CONDTIMEDWAIT(c, m, s, n)	doTimedWait(&c, m, s, n)

#define THREADKEY							pthread_key_t
#define THRKEYCREATE(key, destFct)			pthread_key_create(&key, destFct)
#define THRKEYDELETE(key) 					pthread_key_delete(key)
#define SETTLSDATA(key, data)				(pthread_setspecific(key, (const void*)data) == 0)
#define GETTLSDATA(key, data, dataType)		(data = (dataType*)pthread_getspecific(key))

#endif

#include <errno.h>
#include <signal.h>
#include <stdio.h>
#endif

#endif
