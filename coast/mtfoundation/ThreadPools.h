/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _THREADPOOLS_H
#define _THREADPOOLS_H

#include "Threads.h"
#include "StatUtils.h"

//---- ThreadPoolManager ------------------------------------------------
//! <b>abstract class which handles initialization, starting and termination of threads in a pool</b>
/*!
A thread pool can be used in cases where we do not need to know what each thread does or when it does anything. Important is, that we can have an amount of parallel workers running and waiting on something to do.
<p>The thing is that we do not need an element of control which gets work from somewhere, selects a free Thread and lets it process the work. The Thread can do this on its own. Just implement a special kind of Thread which does exactly this. An excellent usage could be processing elements from a queue.
*/
class EXPORTDECL_MTFOUNDATION ThreadPoolManager : public ThreadObserver
{
public:
	//! does nothing; real work is done in init, object is unusable without call of Init() method!
	ThreadPoolManager();

	//! deletes thread pool; be careful if you overwrite DoDeletePool
	virtual ~ThreadPoolManager();

	/*! initialization of thread pool (may be safely used also for reinit)
		\param maxParallelRequests number of threads to manage in the pool, DoAllocPool() will be called to allocate the threads
		\param args ROAnything carrying pool and thread specific information
		\return true if allocation of threads was successful, false otherwise */
	virtual bool Init(int maxParallelRequests, ROAnything args);

	/*! starts threads with or without using pool storage
		\param usePoolStorage set to true if you want to use special pool memory
		\param poolStorageSize size in kB of the total allocated pool memory
		\param numOfPoolBucketSizes how many different pool buckets to reserve
		\return 0 when all threads could be started, negative number either when there is no thread in the pool or not all of the threads could be started */
	virtual int Start(bool usePoolStorage, int poolStorageSize, int numOfPoolBucketSizes);

	/*! blocks until termination of all pool threads, waits at least 1 second if threads are running
		\param lMaxSecsToWait how many seconds to wait until all threads are not running anymore, specify 0 to wait until all Threads have terminated
		\return number of threads still running */
	virtual int Join(long lMaxSecsToWait = 0);

	/*! implementers should stop all pool threads and cleanup the pool
		\param lWaitToTerminate how many seconds should we wait until a thread is in terminated state
		\param lWaitOnJoin how many seconds to wait until all threads are not running anymore
		\return number of threads not already terminated, 0 in case of success, negative value signals an internal problem concerning the pool
		\note If threads are still running or in termination sequence but not already terminated the threads do not get deleted with DoDeletePool()! In this case, given that you want to re-use the pool, you have to call Terminate again to ensure all threads are done! */
	virtual int Terminate(long lWaitToTerminate, long lWaitOnJoin);

	/*! react to state changes of pool threads
		\param t thread which wants to signal a state change
		\param args arguments the thread can pass us */
	virtual void Update(Thread *t, const Anything &args);

	/*! access the pool size which reflects the number of threads really allocated and accessible to use
		\return number of allocated and usable threads in the pool */
	virtual long GetPoolSize();

protected:
	/*! allocate the thread pool
		\param poolSize number of threads to allocate and use
		\param args ROAnything carrying pool and thread specific information
		\return true if thread allocation was successful, false in case of a allocation failure or poolSize=0 */
	bool AllocPool(long poolSize, ROAnything args);

	/*! create an array of the required workers
		\param poolSize number of threads to allocate and use
		\param args ROAnything carrying pool and thread specific information
		\return true if all threads could be allocated */
	virtual bool DoAllocPool(long poolSize, ROAnything args);

	/*! abstract method to allocate one single thread given its specific params
		\param args ROAnything carrying thread specific information
		\return pointer to the newly created Thread object */
	virtual Thread *DoAllocThread(ROAnything args) = 0;

	/*! cleanup hook for pool, removes threads from pool and deletes them
		\note The threads must already be terminated or a runtime error will occur! */
	virtual void DoDeletePool();

	/*! Initialize all pool threads with their specific parameters and register them as subject for observing state changes
		\param args ROAnything carrying pool and thread specific information
		\return true if all threads could be initialized */
	virtual bool InitPool(ROAnything args);

	/*! get the ith thread out of the pool
		\param i index of the thread, poolSize > i >=0
		\return pointer to the Thread object if it is valid */
	virtual Thread *DoGetThread(long i);

	/*! access configuration for the ith thread, overwrite this method if you do not want to use args[i] as the threads specific arguments
		\param i index of the thread to get the configuration for, poolSize > i >=0
		\param args ROAnything carrying pool and thread specific information
		\return args[i] or an empty ROAnything */
	virtual ROAnything DoGetConfig(long i, ROAnything args);

	/*! Terminate all pool threads
		\param lWaitToTerminate how many seconds should we wait until a thread is in terminated state
		\return number of threads not already terminated, 0 in case of success, negative value signals an internal problem concerning the pool */
	virtual int DoTerminate(long lWaitToTerminate);

	/*! terminated flag is true after the pool was initialized using the Init-function until the Terminate function was called */
	bool fTerminated;

	//! guard for synchronisation
	Mutex fMutex;
	//! condition for synchronisation
	Condition fCond;

	//! counts the still running threads
	long fRunningThreads;
	//! counts the started threads, this value gets reset when the pool is initialized again
	long fStartedThreads;

	//! holds the allocated threads
	Anything fThreadPool;
};

class WorkerPoolManager;

//---- WorkerThread -----------------------------------------------
//!<b>abstraction for processing some work on demand</b>
/*! The WorkerThread processes the workload assigned by its WorkerPoolManager<br>
After processing the workload it sets its running state back to ERunningState::eReady and waits until it gets selected by the WorkerPoolManager again
*/
class EXPORTDECL_MTFOUNDATION WorkerThread : public Thread
{
public:
	//!needs to implement standard constructor for array allocation
	WorkerThread(const char *name = "WorkerThread");
	//!does nothing
	~WorkerThread();

	//! init thread object after construction of elements with array constructor
	void Init(WorkerPoolManager *manager, ROAnything workerInit);

protected:
	//! implements the worker cycle<br><ul><li>waiting for work<li>processing assigned work<li>checking back and waiting again</ul><br> until terminated
	void Run();

	/*! used for general initialization of the worker when it is created. Arbitrary parameters may be passed using the ROAnything...
		\note Make sure to copy whatever you need from the ROAnything into some instance variable because the values of workerInit are only guaranteed to live during the invocation of DoInit
		\param workerInit ROAnything containing initialization params for the WorkerThread */
	virtual void DoInit(ROAnything workerInit);
	//!checks back after processing a work package to announce its readyness after the state change
	virtual void DoReadyHook(ROAnything);
	//! do the work (using the informations you stored in the instance variables)
	virtual void DoProcessWorkload() = 0;

private:
	//!prohibit the use of the copy constructor
	WorkerThread(const WorkerThread &);
	//!prohibit the use of th assignement operator
	WorkerThread &operator=(const WorkerThread &);
	//!the master of the worker
	WorkerPoolManager *fPoolManager;
};

//---- WorkerPoolManager ------------------------------------------------
//!<b>manages a pool of worker threads</b>
/*! this abstract class handles the critical region of parallel active requests
it uses a semaphore which is set by the parameter numMaxParallelRequests
see SamplePoolManager for an example on how to use this class */
class EXPORTDECL_MTFOUNDATION WorkerPoolManager : public StatGatherer
{
public:
	/*! does nothing, real work is done in init, object is still unusable without call of Init() method!
		\param name Give the pool Manager a name, gets printed in statistics */
	WorkerPoolManager(const char *name = "WorkerPoolManager");

	//!prints some statistics; can't delete pool here because representation of subobject is already gone
	virtual ~WorkerPoolManager();

	//!initialisation of  thread pool (may be safely used also for reinit)
	int Init(int maxParallelRequests, int usePoolStorage, int poolStorageSize, int numOfPoolBucketSizes, ROAnything args);

	/*! critical region entry to process the next work package.
		This method blocks the caller (e.g. the server accept-loop) if the pool has no worker ready,  i.e. NumOfRequestsRunning() == MaxRequests2Run()
		\pre needs "request" slot in workload, containing a long which gives a hint which worker to take
		\param workload arguments passed to WorkerThread */
	void Enter(ROAnything workload);

	//!critical region leave, this method allows a blocked caller of Enter to run if NumOfRequests() < MaxRequests2Run()
	void Leave();

	//!blocks the caller waiting for the running requests to terminate. It waits at most 'secs' seconds
	bool AwaitEmpty(long secs);

	/*! waits for termination of all workers which are still running
		\param secs waits for threads secs seconds after that it starts killing threads; if secs == 0 waits uncoditionally
		\return returns true if all threads reached the state eTerminated */
	bool Terminate(long secs = 0);

	//!returns the number of active work packages that are being processed at the moment
	long NumOfRequestsRunning();

	//! returns the number of max active requests running that could run in parallel
	long MaxRequests2Run();

	//! blocking requests on request of the admin server
	void BlockRequests();

	//! unblocking requests on request of the admin server
	void UnblockRequests();

	//! implements the StatGatherer interface used by StatObserver
	void Statistic(Anything &item);

protected:
	//!allocate the thread pool
	bool AllocPool(long poolSize, ROAnything args);

	//! create an array of the required workers
	virtual void DoAllocPool(ROAnything args) = 0;

	//!Init pool members with request processor and pool storage if necessary
	int InitPool(bool usePoolStorage, long poolStorageSize, int numOfPoolBucketSizes, ROAnything args);

	//!handles misconfiguration
	int PreparePool(int usePoolStorage, int poolStorageSize, int numOfPoolBucketSizes, ROAnything args);

	//!cleanup hook for re-initialization of pool
	virtual void DoDeletePool(ROAnything args) = 0;

	//!check if the presently pool may be re-initalized
	virtual bool CanReInitPool();

	/*! finds a runnable WorkerThread object. Since there are fPoolSize WorkerThread objects and also fPoolSize allowed in the critical region there should always at least one be runnable
		\param requestNr a number
		\return the next available WorkerThread */
	WorkerThread *FindNextRunnable(long requestNr);

	//! implementers should return ith WorkerThread
	virtual WorkerThread *DoGetWorker(long i) = 0;

	//! number of allowed request threads that can run in parallel
	long fPoolSize;
	//! number of currently running requests
	long fCurrentParallelRequests;
	//! flag to control request handling on demand by admin server
	bool fBlockRequestHandling;
	//!termination flag
	bool fTerminated;
	//!condition synchronizing the use of the worker pool
	Condition fCond;
	//!mutex guarding access to this objects variables
	Mutex fMutex;
	//!string containing the PoolManagers name
	String fName;
	//! statistic event handler
	StatEvtHandler *fStatEvtHandler;

private:
	//!prohibit the use of the copy constructor
	WorkerPoolManager(const WorkerPoolManager &);
	//!prohibit the use of the assignement operator
	WorkerPoolManager &operator=(const WorkerPoolManager &);
};

#endif
