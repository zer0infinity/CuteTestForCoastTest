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
#include "ObserverIf.h"

//---- ThreadPoolManager ------------------------------------------------
//! abstract class which handles initialization, starting and termination of threads in a pool
/*!
A thread pool can be used in cases where we do not need to know what each thread does or when it does anything. Important is, that we can have an amount of parallel workers running and waiting on something to do.
<p>The thing is that we do not need an element of control which gets work from somewhere, selects a free Thread and lets it process the work. The Thread can do this on its own. Just implement a special kind of Thread which does exactly this. An excellent usage could be processing elements from a queue.
*/
class EXPORTDECL_MTFOUNDATION ThreadPoolManager : public StatGatherer, public Observable<Thread, ROAnything>::Observer
{
protected:
	typedef Observable<Thread, ROAnything> tBaseClass;
	typedef tBaseClass::tObservedPtr tObservedPtr;
	typedef tBaseClass::tArgsRef tArgsRef;
public:
	//! does nothing; real work is done in init, object is unusable without call of Init() method!
	ThreadPoolManager(const char *name);

	//! deletes thread pool; be careful if you overwrite DoDeletePool
	virtual ~ThreadPoolManager();

	/*! initialization of thread pool (may be safely used also for reinit)
		\param maxParallelRequests number of threads to manage in the pool, DoAllocPool() will be called to allocate the threads
		\param roaThreadArgs ROAnything carrying thread specific information
		\return true if allocation of threads was successful, false otherwise */
	virtual bool Init(int maxParallelRequests, ROAnything roaThreadArgs);

	/*! starts threads with or without using pool storage
		\param usePoolStorage set to true if you want to use special pool memory
		\param poolStorageSize size in kB of the total allocated pool memory
		\param numOfPoolBucketSizes how many different pool buckets to reserve
		\param roaThreadArgs ROAnything carrying thread specific information
		\return 0 when all threads could be started, negative number either when there is no thread in the pool or not all of the threads could be started */
	virtual int Start(bool usePoolStorage, int poolStorageSize, int numOfPoolBucketSizes, ROAnything roaThreadArgs = ROAnything());

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
		\param pObserved thread which wants to signal a state change
		\param roaUpdateArgs arguments the thread can pass us */
	virtual void Update(tObservedPtr pObserved, tArgsRef roaUpdateArgs);

	/*! blocks the caller, waiting for threads to go in Ready-state. It waits at most 'secs' seconds
		\param secs how long to wait in seconds
		\return true in case all threads entered Ready-state, false otherwise */
	bool AwaitReady(long secs);

	/*! access the pool size which reflects the number of threads really allocated and accessible to use
		\return number of allocated and usable threads in the pool */
	virtual long GetPoolSize();

	//! Get my name
	String GetName();

	//! Set my name
	void SetName(const char *str)	{
		fName = str;
	}

	bool InTermination() const {
		return fTerminated;
	}

protected:
	/*! implements the StatGatherer interface used by StatObserver
		\param statistics Anything to get statistics data */
	void DoGetStatistic(Anything &statistics);

	/*! allocate the thread pool
		\param poolSize number of threads to allocate and use
		\param roaThreadArgs ROAnything carrying thread specific information
		\return true if thread allocation was successful, false in case of a allocation failure or poolSize=0 */
	bool AllocPool(long poolSize, ROAnything roaThreadArgs);

	/*! create an array of the required workers
		\param poolSize number of threads to allocate and use
		\param roaThreadArgs ROAnything carrying thread specific information
		\return true if all threads could be allocated */
	virtual bool DoAllocPool(long poolSize, ROAnything roaThreadArgs);

	/*! abstract method to allocate one single thread given its specific params
		\param i index of the thread to allocate
		\param roaThreadArgs ROAnything carrying thread specific information
		\return pointer to the newly created Thread object */
	virtual Thread *DoAllocThread(long i, ROAnything roaThreadArgs) = 0;

	/*! cleanup hook for pool, removes threads from pool and deletes them
		\note The threads must already be terminated or a runtime error will occur! */
	virtual void DoDeletePool();

	/*! Initialize all pool threads with their specific parameters and register them as subject for observing state changes
		\param roaThreadArgs ROAnything carrying thread specific information
		\return true if all threads could be initialized */
	virtual bool InitPool(ROAnything roaThreadArgs);

	/*! get the ith thread out of the pool
		\param i index of the thread, poolSize > i >=0
		\return pointer to the Thread object if it is valid */
	virtual Thread *DoGetThread(long i);

	/*! access configuration for the ith thread, overwrite this method if you do not want to use roaThreadArgs[i] as the threads specific arguments when initializing it
		\param i index of the thread to get the configuration for, poolSize > i >=0
		\param roaThreadArgs ROAnything carrying thread specific information
		\return roaThreadArgs[i] or an empty ROAnything */
	virtual ROAnything DoGetInitConfig(long i, ROAnything roaThreadArgs);

	/*! access configuration for the ith thread, overwrite this method if you do not want to use roaThreadArgs[i] as the threads specific arguments when starting it
		\param i index of the thread to get the configuration for, poolSize > i >=0
		\param roaThreadArgs ROAnything carrying thread specific information
		\return roaThreadArgs[i] or an empty ROAnything */
	virtual ROAnything DoGetStartConfig(long i, ROAnything roaThreadArgs);

	/*! Terminate all pool threads
		\param lWaitToTerminate how many seconds should we wait until a thread is in terminated state
		\return number of threads not already terminated, 0 in case of success, negative value signals an internal problem concerning the pool */
	virtual int DoTerminate(long lWaitToTerminate);

	/*! terminated flag is true after the pool was initialized using the Init-function until the Terminate function was called */
	bool fTerminated;

protected:
	//! guard for synchronisation
	SimpleMutex fMutex;
	//! condition for synchronisation
	SimpleMutex::ConditionType fCond;

private:
	//!string containing the PoolManagers name
	String fName;

	//! counts the still running threads
	long fRunningThreads;
	//! counts the started threads, this value gets reset when the pool is initialized again
	long fStartedThreads;
	//! counts the terminated threads, this value gets reset when the pool is initialized again
	long fTerminatedThreads;

	typedef std::auto_ptr<StatEvtHandler> StatEvtHandlerPtrType;

	//! statistic event handler
	StatEvtHandlerPtrType fpStatEvtHandler;

	//! holds the allocated threads
	Anything fThreadPool;

	friend class TestThreadPool;
};

//---- WorkerThread -----------------------------------------------
//! abstraction for processing some work on demand
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
	int Init(ROAnything workerInit);

protected:
	//! implements the worker cycle<br><ul><li>waiting for work<li>processing assigned work<li>checking back and waiting again</ul><br> until terminated
	void Run();

	/*! used for general initialization of the worker when it is created. Arbitrary parameters may be passed using the ROAnything...
		\note Make sure to copy whatever you need from the ROAnything into some instance variable because the values of workerInit are only guaranteed to live during the invocation of DoInit
		\param workerInit ROAnything containing initialization params for the WorkerThread */
	virtual void DoInit(ROAnything workerInit) {};

	/*! PoolManager interface to pass working arguments - called by Thread::SetWorking()
		\param roaWorkerArgs pass work to do and store it for later use by DoProcessWorkload()
		\note If the worker is configured to use its own PoolAllocator, do NOT try to use the Workers allocator within this function especially if Traces could be enabled (opt-wddbg or dbg)! Imagine an outer Thread using WorkerPoolManager::Enter() to pass its work. Further assume that the worker just left the SetReady() method but is not yet able to check its RunningState because of a OS-ThreadSwitch. Now the caller will get active again and Thread::SetWorking() will be called to pass the workload. Now this function will get active and receive the workload to be internally stored. If the workload is an Anything which we fully need later we could store it in an Anything internally by DeepCloning the passed work. If we do this using an unmodified member-Anything and a default-param DeepClone() everything will work fine because the internal member-Anything will hav an Allocator of Storage::Global(). But if we try to optimize our member-Anything and call SetAllocator(fAllocator) on it - because we want unlocked fast access - we have more troubles than benefits. But why? Because in the process of DeepCloning the given ROAnything, another context-switch might occur and the WorkerThread becomes active again. As stated above, it has to check his RunningState and should process the work when it switched to eWorking. But in the call-chain there is a Mutex which - if Traces are compiled in and optionally enabled - wants to tell the console that a Mutex has to be accessed and also additional things like who it was and so on. But this String needs memory - in our case PoolAllocator memory - to hold the content to print out. Now we have the case where two concurrent accessors try to Alloc/Free PoolAllocator memory and this can lead to nice BusErrors because PoolAllocators do not use locking on internal memory accesses. */
	virtual void DoWorkingHook(ROAnything roaWorkerArgs) {};

	//! checks back after processing a work package to cleanup things if required
	virtual void DoReadyHook(ROAnything);

	//! do the work (using the informations you stored in the instance variables)
	virtual void DoProcessWorkload() = 0;

	/*! if we use pool allocators to allocate memory they will be refreshed in DoReadyHook(). Take care in implementing Workers which optimize for memory speed!
		set to true if a refresh is possible, to false if we should not refresh it */
	bool fRefreshAllocator;

private:
	//!prohibit the use of the copy constructor
	WorkerThread(const WorkerThread &);
	//!prohibit the use of th assignement operator
	WorkerThread &operator=(const WorkerThread &);
	//!the master of the worker
};

//---- PoolManager ------------------------------------------------
template
<
typename TObservedType,
		 typename TArgs
		 >
class EXPORTDECL_MTFOUNDATION PoolManager
	: public StatGatherer
	, public Observable<TObservedType, TArgs>::Observer
{
public:
	typedef Observable<TObservedType, TArgs> tBaseClass;
	typedef typename tBaseClass::tObservedPtr tObservedPtr;
	typedef typename tBaseClass::tArgsRef tArgsRef;

	/*! does nothing, real work is done in init, object is still unusable without call of Init() method!
		\param name Give the pool Manager a name, gets printed in statistics */
	PoolManager(const char *name) {};

	/*! prints some statistics and terminates
		\note can't delete pool here because representation of subobject has already gone */
	virtual ~PoolManager() {};

	virtual void Update(tObservedPtr pObserved, tArgsRef aUpdateArgs) = 0;

//	/*! access the pool size which reflects the number of resources really allocated and accessible to use
//		\return number of allocated and usable resources in the pool */
//	long GetPoolSize();
//
//	/*! access the pool and return the number of resources currently in use
//		\return number of resources in use */
//	long ResourcesUsed();
//
//	/*! access the pool and return the number of currently free resources
//		\return number of free resources */
//	long ResourcesFree();

protected:
//	/*! allocate the resource pool
//		\param poolSize number of resources to allocate and use
//		\param roaResourceArgs ROAnything carrying resource specific information
//		\return true if resource allocation was successful, false in case of a allocation failure or poolSize=0 */
//	bool AllocPool(long poolSize, ROAnything roaResourceArgs);
//
//	/*! create an array of the required workers
//		\param poolSize number of resources to allocate and use
//		\param roaResourceArgs ROAnything carrying resource specific information
//		\return true if all resources could be allocated */
//	virtual bool DoAllocPool(long poolSize, ROAnything roaResourceArgs);
//
//	/*! abstract method to allocate one single resource given its specific params
//		\param i index of the thread to allocate
//		\param roaResourceArgs ROAnything carrying resource specific information
//		\return pointer to the newly created Thread object */
//	virtual Thread *DoAllocThread(long i, ROAnything roaResourceArgs) = 0;
//
//	/*! cleanup hook for pool, removes resources from pool and deletes them
//		\note The resources must already be terminated or a runtime error will occur! */
//	virtual void DoDeletePool();
//
//	/*! Initialize all pool resources with their specific parameters and register them as subject for observing state changes
//		\param roaResourceArgs ROAnything carrying resource specific information
//		\return true if all resources could be initialized */
//	virtual bool InitPool(ROAnything roaResourceArgs);
//
//	/*! get the ith resource out of the pool
//		\param i index of the resource, poolSize > i >=0
//		\return pointer to the resource object if it is valid */
//	virtual Thread *DoGetThread(long i);
//
//	/*! access configuration for the ith resource, overwrite this method if you do not want to use roaResourceArgs[i] as the resources specific arguments when initializing it
//		\param i index of the resource to get the configuration for, poolSize > i >=0
//		\param roaResourceArgs ROAnything carrying resource specific information
//		\return roaResourceArgs[i] or an empty ROAnything */
//	virtual ROAnything DoGetInitConfig(long i, ROAnything roaResourceArgs);
//
//	/*! access configuration for the ith resource, overwrite this method if you do not want to use roaResourceArgs[i] as the resources specific arguments when starting it
//		\param i index of the resource to get the configuration for, poolSize > i >=0
//		\param roaResourceArgs ROAnything carrying resource specific information
//		\return roaResourceArgs[i] or an empty ROAnything */
//	virtual ROAnything DoGetStartConfig(long i, ROAnything roaResourceArgs);
//
//	/*! Terminate all pool resources
//		\param lWaitToTerminate how many seconds should we wait until a resource is in terminated state
//		\return number of resources not already terminated, 0 in case of success, negative value signals an internal problem concerning the pool */
//	virtual int DoTerminate(long lWaitToTerminate);

private:
//	bool		fbInitialized;
//	Anything	fanyListOfResources;
//	Mutex		fStructureMutex;
//	Semaphore	*fpResourcesSema;

	//!prohibit the use of the copy constructor
	PoolManager(const PoolManager &);
	//!prohibit the use of the assignement operator
	PoolManager &operator=(const PoolManager &);
};

//---- WorkerPoolManager ------------------------------------------------
//! manages a pool of worker threads
/*! this abstract class handles the critical region of parallel active requests
it uses a semaphore which is set by the parameter numMaxParallelRequests
see SamplePoolManager for an example on how to use this class */
class EXPORTDECL_MTFOUNDATION WorkerPoolManager : public StatGatherer, public Observable<Thread, ROAnything>::Observer
{
protected:
	typedef Observable<Thread, ROAnything> tBaseClass;
	typedef tBaseClass::tObservedPtr tObservedPtr;
	typedef tBaseClass::tArgsRef tArgsRef;
public:
	/*! does nothing, real work is done in init, object is still unusable without call of Init() method!
		\param name Give the pool Manager a name, gets printed in statistics */
	WorkerPoolManager(const char *name = "WorkerPoolManager");

	//!prints some statistics; can't delete pool here because representation of subobject is already gone
	virtual ~WorkerPoolManager();

	/*! react to state changes of pool threads
		\param pObserved thread which wants to signal a state change
		\param roaUpdateArgs arguments the thread can pass us */
	virtual void Update(tObservedPtr pObserved, tArgsRef roaUpdateArgs);

	//!initialisation of  thread pool (may be safely used also for reinit)
	int Init(int maxParallelRequests, int usePoolStorage, int poolStorageSize, int numOfPoolBucketSizes, ROAnything roaWorkerArgs);

	/*! Use this method to signal final termination by deleting the internal threads. A later (Re)Init can safely be done
		\param lWaitTime time in seconds to wait on internal threads termination
		\return true in case of success, eg. internal threads were terminated, false otherwise */
	bool Finis(long lWaitTime);

	/*! critical region entry to process the next work package.
		This method blocks the caller (e.g. the server accept-loop) if the pool has no worker ready,  i.e. ResourcesUsed() == GetPoolSize()
		\pre needs "request" slot in workload, containing a long which gives a hint which worker to take
		\param workload arguments passed to WorkerThread
		\param lFindWorkerHint hint to select correct WorkerThread for processing
		\return true in case a worker could be found and is processing the work, false otherwise, which means we should try again */
	template< class WorkerParamType >
	bool Enter( WorkerParamType workload, long lFindWorkerHint );

	//!blocks the caller waiting for the running requests to terminate. It waits at most 'secs' seconds
	bool AwaitEmpty(long secs);

	/*! waits for termination of all workers which are still running
		\param secs waits for threads secs seconds after that it starts killing threads; if secs == 0 waits uncoditionally
		\return returns true if all threads reached the state eTerminated */
	bool Terminate(long secs = 0);

	//!returns the number of active work packages that are being processed at the moment
	long ResourcesUsed();

	//! returns the number of max active requests running that could run in parallel
	long GetPoolSize();

	//! blocking requests on request of the admin server
	void BlockRequests();

	//! unblocking requests on request of the admin server
	void UnblockRequests();

	//! Get my name
	String GetName();

	//! Set my name
	void SetName(const char *str)	{
		fName = str;
	}

protected:
	/*! implements the StatGatherer interface used by StatObserver
		\param statistics Anything to get statistics data */
	void DoGetStatistic(Anything &statistics);

	//! create an array of the required workers
	virtual void DoAllocPool(ROAnything roaWorkerArgs) = 0;

	//! implementers should return ith WorkerThread
	virtual WorkerThread *DoGetWorker(long i) = 0;

	//!cleanup hook for re-initialization of pool
	virtual void DoDeletePool(ROAnything roaWorkerArgs) = 0;

	//!check if the presently pool may be re-initalized
	virtual bool CanReInitPool();

	//!allocate the thread pool
	bool AllocPool(long poolSize, ROAnything roaWorkerArgs);

	//!Init pool members with request processor and pool storage if necessary
	int InitPool(bool usePoolStorage, long poolStorageSize, int numOfPoolBucketSizes, ROAnything roaWorkerArgs);

	//!handles misconfiguration
	int PreparePool(int usePoolStorage, int poolStorageSize, int numOfPoolBucketSizes, ROAnything roaWorkerArgs);

	/*! finds a runnable WorkerThread object. Since there are fPoolSize WorkerThread objects and also fPoolSize allowed in the critical region there should always at least one be runnable
		\param lFindWorkerHint a number giving a hint which worker to select
		\return the next available WorkerThread */
	virtual WorkerThread *FindNextRunnable(long lFindWorkerHint);

protected:
	//!mutex guarding access to this objects variables
	SimpleMutex fMutex;
	//!condition synchronizing the use of the worker pool
	SimpleMutex::ConditionType fCond;

private:
	//!string containing the PoolManagers name
	String fName;
	//! number of allowed request threads that can run in parallel
	long fPoolSize;
	//! flag to control request handling on demand by admin server
	bool fBlockRequestHandling;
	//!termination flag
	bool fTerminated;

	typedef std::auto_ptr<StatEvtHandler> StatEvtHandlerPtrType;

	//! statistic event handler
	StatEvtHandlerPtrType fpStatEvtHandler;

private:
	//!prohibit the use of the copy constructor
	WorkerPoolManager(const WorkerPoolManager &);
	//!prohibit the use of the assignement operator
	WorkerPoolManager &operator=(const WorkerPoolManager &);
};

#include "ThreadPools.ipp"

#endif
