/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

//--- interface include --------------------------------------------------------
#include "SystemAPI.h"

//--- standard modules used ----------------------------------------------------
#include "Threads.h"
#include "SystemLog.h"
#include "Dbg.h"

//--- c-library modules used ---------------------------------------------------
#if !defined(WIN32)
#include <sys/time.h>
#endif

#if defined(WIN32)

THREADWRAPPERFUNCDECL(ThreadWrapper, thread)
{
	Thread *t = (Thread *) thread;	// we should better check this....
	if (t) {
		// need to set thread id here because no other possibility when using _beginthread
		t->fThreadId = (void *)THRID();
#if defined(_DLL)
		if (!TlsSetValue(fgThreadPtrKey, (void *)t)) {
			SysLog::Error( String("ThreadWrapper[") << (long)t->fThreadId << "] failed to store Thread* into TLS");
		}
		Anything aAny;
		aAny["id"] = (long)t->fThreadId;
		aAny["Name"] = t->GetName();
		aAny["Addr"] = (IFAObject *)t;
		{
			LockUnlockEntry sm(fgThreadsMutex);
			fgThreads[Anything((long)t).AsString()] = aAny.DeepClone();
		}
#endif
		t->IntRun();
#if !defined(_DLL)
		t->CleanupThreadStorage();
#endif
		t->Exit(0);
	} else {
		DELETETHREAD();
	}
}

int doTimedWait(HANDLE c, HANDLE m, long s, long n /* nanoseconds...*/)
{
	int ret = doWaitObject(c, m, (s == INFINITE) ? s : (s * 1000L + (n / 1000000)));
	doWaitObject(m, 0, INFINITE);
	return ret;
}

int doWaitObject(HANDLE h, HANDLE m, long time /* milliseconds...*/)
{
	int ret = -1;
	if (m) {
		fooRELOBJECT(m);
		fooWAITOBJECT(h);
		ret = ::SignalObjectAndWait(m, h, time, false);
	} else {
		fooWAITOBJECT(h);
		ret = ::WaitForSingleObject(h, time);
	}

	switch (ret) {
		case WAIT_FAILED: {
			String strErr("Wait failed!! (");
			strErr << (long)GetLastError() << ")\n";
			SysLog::WriteToStderr(strErr);
			break;
		}
		case WAIT_ABANDONED: {
			String strErr("Wait abandoned!!\n");
			SysLog::WriteToStderr(strErr);
			break;
		}
		case WAIT_TIMEOUT:
			//#ifdef DEBUG
			//		cerr << "##ID: " << (long)h << " Wait timeout!! ##" << endl;
			//#endif
			break;
		case WAIT_OBJECT_0:
			fooLOCKOBJECT(h);
			break;
		default:
			break;
	}
	return ret;
}

#else
static void AdjustTimeToAbsolut(long &s, long &n)
{
	const long ONESECOND = 1000 * 1000 * 1000; // 1e9 nanosecs
	timeval tv;
	if (0 == gettimeofday(&tv, 0)) {
		n += tv.tv_usec * 1000; // make it nanoseconds
		s += tv.tv_sec;
	} else {
		s += time(0); // fall back towards using time
	}
	while (n > ONESECOND) {
		s++, n -= ONESECOND;    // might fail for large ns, be careful
	}
}

#if defined(__sun) && !defined(_POSIX_THREADS)
THREADWRAPPERFUNCDECL(ThreadWrapper, thread)
{
	Thread *t = (Thread *) thread;	// we should better check this....
	if (t) {
		t->IntRun();
		t->CleanupThreadStorage();
		t->Exit(0);
	} else {
		DELETETHREAD();
	}
	return 0;
}

bool doStartThread(void *obj, bool *b, thread_t *thrId, void * (*wrapper)(void *))
{
	int flags = 0;

	if (b[0]) {
		flags |= THR_BOUND;
	}
	if (b[1]) {
		flags |= THR_DETACHED;
	}
	if (b[2]) {
		flags |= THR_DAEMON;
	}
	if (b[3]) {
		flags |= THR_SUSPENDED;
	}

	flags |= THR_NEW_LWP;	// set concurrency

	int retVal;
	if ( (retVal = thr_create(NULL, 0, wrapper, obj, flags, thrId)) != 0 ) {
		String logMsg;
		SysLog::Error(logMsg << "Thread::Start: error in thr_create" << SysLog::SysErrorMsg(retVal));
		return false;
	}
	return true;
}

bool doTryLock(mutex_t *m, int &ret)
{
	ret = mutex_trylock(m);
	String logMsg;
	if ( ret == EFAULT ) {
		SysLog::Error(logMsg << "doTryLock" << SysLog::SysErrorMsg(ret));
		StartTrace1(Mutex.TryLock, "EFAULT");	// reuse Mutex.TryLock switch
		return false;
	}
	if ( ret == EINVAL ) {
		SysLog::Error(logMsg << "doTryLock" << SysLog::SysErrorMsg(ret));
		StartTrace1(Mutex.TryLock, "EINVAL");	// reuse Mutex.TryLock switch
		return false;
	}
	if ( ret == EBUSY ) {
		StartTrace1(Mutex.TryLock, "EBUSY");	// reuse Mutex.TryLock switch
		return false;
	}
	return true;
}

void doRWLock(rwlock_t *rw, bool reading)
{
	if (reading) {
		if (rw_rdlock((rwlock_t *) rw) != 0) {
			perror("RWLock::ReadLock");
		}
	} else {
		if (rw_wrlock((rwlock_t *) rw) != 0) {
			perror("RWLock::WriteLock");
		}
	}
}

bool doTryRWLock(rwlock_t *rw, bool reading)
{
	int ret;
	if (reading) {
		ret = rw_tryrdlock((rwlock_t *) rw);
	} else {
		ret = rw_trywrlock((rwlock_t *) rw);
	}

	if ( ret == EFAULT ) {
		perror("RWLock::TryLock");
		return false;

	}
	if ( ret == EINVAL ) {
		perror("RWLock::TryLock");
		return false;
	}
	if ( ret == EBUSY ) {
		return false;
	}
	return true;
}

int doTimedWait(cond_t *c, mutex_t *m, long s, long n)
{
	timestruc_t t;
	AdjustTimeToAbsolut(s, n);
	t.tv_sec = s;
	t.tv_nsec = n;
	return cond_timedwait(c,  m, (timestruc_t *) &t);
}

#else // assume posix e.g. Linux

THREADWRAPPERFUNCDECL(ThreadWrapper, thread)
{
	Thread *t = (Thread *) thread;	// we should better check this....
	if (t) {
		t->IntRun();
		t->CleanupThreadStorage();
		t->Exit(0);
	} else {
		DELETETHREAD();
	}
	return 0;
}

bool doStartThread(void *obj, bool *b, pthread_t *thrId, void * (*wrapper)(void *))
{
	pthread_attr_t attr;
	pthread_attr_init(&attr);

	if (b[0]);
	//flags|= THR_BOUND;
	if (b[1]) {
		pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
	}
	//flags|= THR_DETACHED;
	if (b[2]) {
		pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
	}
	//flags|= THR_DAEMON;
	if (b[3]);
	//flags|= THR_SUSPENDED;

	int retVal;
	if ( (retVal = pthread_create(thrId, &attr, wrapper, obj)) != 0 ) {

		switch (retVal) {
			case EAGAIN:
				SysLog::Error("EAGAIN");
				break;
			case EINVAL:
				SysLog::Error("EINVAL" );
				break;
			case ENOMEM:
				SysLog::Error("ENOMEM");
				break;
			default:
				break;
		};

		String logMsg;
		SysLog::Error(logMsg << "Thread::Start: error in thr_create " << (long)retVal); //SysLog::SysErrorMsg(errno));
		return false;
	}
	return true;
}

bool doTryLock(pthread_mutex_t *m, int &ret)
{
	ret = pthread_mutex_trylock(m);
	String logMsg;
	if ( ret == EFAULT ) {
		SysLog::Error(logMsg << "doTryLock" << SysLog::SysErrorMsg(ret));
		return false;
	}
	if ( ret == EINVAL ) {
		SysLog::Error(logMsg << "doTryLock" << SysLog::SysErrorMsg(ret));
		return false;
	}
	if ( ret == EBUSY ) {
		return false;
	}

	return 0 == ret;
}

void doRWLock(rwlock_t *rw, bool reading)
{
	if (reading) {
		if (rwl_readlock(rw) != 0) { //if (rw_rdlock((rwlock_t*) rw) != 0)
			perror("RWLock::ReadLock");
		}
	} else {
		if (rwl_writelock(rw) != 0) { //if (rw_wrlock((rwlock_t*) rw) != 0)
			perror("RWLock::WriteLock");
		}
	}
}

bool doRWTrylock(rwlock_t *rw, bool reading)
{
	int ret;
	if (reading) {
		ret = rwl_readtrylock(rw); //rw_tryrdlock((rwlock_t*) rw);
	} else {
		ret = rwl_writetrylock(rw); //ret= rw_trywrlock((rwlock_t*) rw);
	}

	if ( ret == EFAULT ) {
		perror("RWLock::TryLock");
		return false;
	}
	if ( ret == EINVAL ) {
		perror("RWLock::TryLock");
		return false;
	}
	if ( ret == EBUSY ) {
		return false;
	}

	return true;

}

bool doRWUnlock(rwlock_t *rw, bool reading)
{
	int ret;
	if (reading) {
		ret = rwl_readunlock(rw); //rw_tryrdlock((rwlock_t*) rw);
	} else {
		ret = rwl_writeunlock(rw); //ret= rw_trywrlock((rwlock_t*) rw);
	}

	if ( ret == EFAULT ) {
		perror("RWLock::TryLock");
		return false;
	}
	if ( ret == EINVAL ) {
		perror("RWLock::TryLock");
		return false;
	}
	if ( ret == EBUSY ) {
		return false;
	}

	return true;
}

int doTimedWait(pthread_cond_t *c, pthread_mutex_t *m, long s, long n)
{
	timespec t;
	AdjustTimeToAbsolut(s, n);
	t.tv_sec = s;
	t.tv_nsec = n;
	return pthread_cond_timedwait(c,  m,  &t);
}

#define RWLOCK_VALID    0xfacade

/*
* Support static initialization of barriers
*/
#define RWL_INITIALIZER \
{PTHREAD_MUTEX_INITIALIZER, PTHREAD_COND_INITIALIZER, \
PTHREAD_COND_INITIALIZER, RWLOCK_VALID, 0, 0, 0, 0}
/*
* Initialize a read-write lock
*/
int rwl_init (rwlock_t *rwl)
{
	int status;

	rwl->r_active = 0;
	rwl->r_wait = rwl->w_wait = 0;
	rwl->w_active = 0;
	status = pthread_mutex_init (&rwl->mutex, NULL);
	if (status != 0) {
		return status;
	}
	status = pthread_cond_init (&rwl->read, NULL);
	if (status != 0) {
		/* if unable to create read CV, destroy mutex */
		pthread_mutex_destroy (&rwl->mutex);
		return status;
	}
	status = pthread_cond_init (&rwl->write, NULL);
	if (status != 0) {
		/* if unable to create write CV, destroy read CV and mutex */
		pthread_cond_destroy (&rwl->read);
		pthread_mutex_destroy (&rwl->mutex);
		return status;
	}
	rwl->valid = RWLOCK_VALID;
	return 0;
}

/*
* Destroy a read-write lock
*/
int rwl_destroy (rwlock_t *rwl)
{
	int status, status1, status2;

	if (rwl->valid != RWLOCK_VALID) {
		return EINVAL;
	}
	status = pthread_mutex_lock (&rwl->mutex);
	if (status != 0) {
		return status;
	}

	/*
	* Check whether any threads own the lock; report "BUSY" if
	* so.
	*/
	if (rwl->r_active > 0 || rwl->w_active) {
		pthread_mutex_unlock (&rwl->mutex);
		return EBUSY;
	}

	/*
	* Check whether any threads are known to be waiting; report
	* EBUSY if so.
	*/
	if (rwl->r_wait != 0 || rwl->w_wait != 0) {
		pthread_mutex_unlock (&rwl->mutex);
		return EBUSY;
	}

	rwl->valid = 0;
	status = pthread_mutex_unlock (&rwl->mutex);
	if (status != 0) {
		return status;
	}
	status = pthread_mutex_destroy (&rwl->mutex);
	status1 = pthread_cond_destroy (&rwl->read);
	status2 = pthread_cond_destroy (&rwl->write);
	return (status == 0 ? status : (status1 == 0 ? status1 : status2));
}

/*
* Handle cleanup when the read lock condition variable
* wait is cancelled.
*
* Simply record that the thread is no longer waiting,
* and unlock the mutex.
*/
static void rwl_readcleanup (void *arg)
{
	rwlock_t    *rwl = (rwlock_t *)arg;

	rwl->r_wait--;
	pthread_mutex_unlock (&rwl->mutex);
}

/*
* Lock a read-write lock for read access.
*/
int rwl_readlock (rwlock_t *rwl)
{
	int status;

	if (rwl->valid != RWLOCK_VALID) {
		return EINVAL;
	}
	status = pthread_mutex_lock (&rwl->mutex);
	if (status != 0) {
		return status;
	}
	if (rwl->w_active) {
		rwl->r_wait++;
		pthread_cleanup_push (rwl_readcleanup, (void *)rwl);
		while (rwl->w_active) {
			status = pthread_cond_wait (&rwl->read, &rwl->mutex);
			if (status != 0) {
				break;
			}
		}
		pthread_cleanup_pop (0);
		rwl->r_wait--;
	}
	if (status == 0) {
		rwl->r_active++;
	}
	pthread_mutex_unlock (&rwl->mutex);
	return status;
}

/*
* Attempt to lock a read-write lock for read access (don't
* block if unavailable).
*/
int rwl_readtrylock (rwlock_t *rwl)
{
	int status, status2;

	if (rwl->valid != RWLOCK_VALID) {
		return EINVAL;
	}
	status = pthread_mutex_lock (&rwl->mutex);
	if (status != 0) {
		return status;
	}
	if (rwl->w_active) {
		status = EBUSY;
	} else {
		rwl->r_active++;
	}
	status2 = pthread_mutex_unlock (&rwl->mutex);
	return (status2 != 0 ? status2 : status);
}

/*
* Unlock a read-write lock from read access.
*/
int rwl_readunlock (rwlock_t *rwl)
{
	int status, status2;

	if (rwl->valid != RWLOCK_VALID) {
		return EINVAL;
	}
	status = pthread_mutex_lock (&rwl->mutex);
	if (status != 0) {
		return status;
	}
	rwl->r_active--;
	if (rwl->r_active == 0 && rwl->w_wait > 0) {
		status = pthread_cond_signal (&rwl->write);
	}
	status2 = pthread_mutex_unlock (&rwl->mutex);
	return (status2 == 0 ? status : status2);
}

/*
* Handle cleanup when the write lock condition variable
* wait is cancelled.
*
* Simply record that the thread is no longer waiting,
* and unlock the mutex.
*/
static void rwl_writecleanup (void *arg)
{
	rwlock_t *rwl = (rwlock_t *)arg;

	rwl->w_wait--;
	pthread_mutex_unlock (&rwl->mutex);
}

/*
* Lock a read-write lock for write access.
*/
int rwl_writelock (rwlock_t *rwl)
{
	int status;

	if (rwl->valid != RWLOCK_VALID) {
		return EINVAL;
	}
	status = pthread_mutex_lock (&rwl->mutex);
	if (status != 0) {
		return status;
	}
	if (rwl->w_active || rwl->r_active > 0) {
		rwl->w_wait++;
		pthread_cleanup_push (rwl_writecleanup, (void *)rwl);
		while (rwl->w_active || rwl->r_active > 0) {
			status = pthread_cond_wait (&rwl->write, &rwl->mutex);
			if (status != 0) {
				break;
			}
		}
		pthread_cleanup_pop (0);
		rwl->w_wait--;
	}
	if (status == 0) {
		rwl->w_active = 1;
	}
	pthread_mutex_unlock (&rwl->mutex);
	return status;
}

/*
* Attempt to lock a read-write lock for write access. Don't
* block if unavailable.
*/
int rwl_writetrylock (rwlock_t *rwl)
{
	int status, status2;

	if (rwl->valid != RWLOCK_VALID) {
		return EINVAL;
	}
	status = pthread_mutex_lock (&rwl->mutex);
	if (status != 0) {
		return status;
	}
	if (rwl->w_active || rwl->r_active > 0) {
		status = EBUSY;
	} else {
		rwl->w_active = 1;
	}
	status2 = pthread_mutex_unlock (&rwl->mutex);
	return (status != 0 ? status : status2);
}

/*
* Unlock a read-write lock from write access.
*/
int rwl_writeunlock (rwlock_t *rwl)
{
	int status;

	if (rwl->valid != RWLOCK_VALID) {
		return EINVAL;
	}
	status = pthread_mutex_lock (&rwl->mutex);
	if (status != 0) {
		return status;
	}
	rwl->w_active = 0;
	if (rwl->r_wait > 0) {
		status = pthread_cond_broadcast (&rwl->read);
		if (status != 0) {
			pthread_mutex_unlock (&rwl->mutex);
			return status;
		}
	} else if (rwl->w_wait > 0) {
		status = pthread_cond_signal (&rwl->write);
		if (status != 0) {
			pthread_mutex_unlock (&rwl->mutex);
			return status;
		}
	}
	status = pthread_mutex_unlock (&rwl->mutex);
	return status;
}

#if defined(__APPLE__)
#include <sstream>
bool sem_initialize(sem_t **sema, unsigned int count)
{
	static int sema_num = 0;
	std::stringstream ss;
	std::stringstream ss_init_sema;
	ss_init_sema << "/init_sema_" << getpid();
	sem_t *init_sema = sem_open(ss_init_sema.str().c_str(), O_CREAT | O_EXCL, 700, 1);
	sem_wait(init_sema);
	ss << "/" << getpid() << ":" << ++sema_num;
	*sema = (sem_open(ss.str().c_str(), O_CREAT | O_EXCL, 700, count));
	sem_post(init_sema);
	return ((long) sema != (long) SEM_FAILED);
}
#endif

#endif

#endif
