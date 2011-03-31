/*
 * Copyright (c) 2007, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#include "BoostPoolTest.h"
#include "TestSuite.h"
#include <boost/pool/pool_alloc.hpp>
#include <boost/pool/object_pool.hpp>
#include "SystemLog.h"
#define COAST_DISABLE_TRACE
#include "ThreadPools.h"
#include "MT_Storage.h"
#include <boost/bind.hpp>
#include <boost/function.hpp>
#include <vector>
#include <set>

//---- BoostPoolTest ----------------------------------------------------------------
BoostPoolTest::BoostPoolTest(TString tstrName)
	: TestCaseType(tstrName)
{
	StartTrace(BoostPoolTest.Ctor);
}

BoostPoolTest::~BoostPoolTest()
{
	StartTrace(BoostPoolTest.Dtor);
}

// uncomment if something special needs to be done which isnt already done in base class
//void BoostPoolTest::setUp ()
//{
//	StartTrace(BoostPoolTest.setUp);
//}
//
//void BoostPoolTest::tearDown ()
//{
//	StartTrace(BoostPoolTest.tearDown);
//}

// Each "tester" object below checks into and out of the "cdtor_checker",
//  which will report any problems related to the construction/destruction of
//  "tester" objects.
class cdtor_checker
{
private:
	// Each constructed object registers its "this" pointer into "objs"
	std::set<void *> objs;

public:
	~cdtor_checker() {
		// At end of program, print out memory leaks
		//  (assuming no static "tester"s)
		for (std::set<void *>::const_iterator i = objs.begin(); i != objs.end(); ++i) {
			SYSERROR("Memory leak: " << (long)*i);
		}
	}

	void check_in(void *const This) {
		if (objs.find(This) != objs.end()) {
			SYSERROR("Double-constructed memory: " << (long)This);
		}
		objs.insert(This);
	}
	void check_out(void *const This) {
		std::set<void *>::iterator i = objs.find(This);
		if (i == objs.end()) {
			SYSERROR("Destroyed non-constructed memory: " << (long)This);
		}
		objs.erase(This);
	}

	// true iff all objects that have checked in have checked out
	bool ok() const {
		return objs.empty();
	}
};
static cdtor_checker mem;

struct tester {
	tester(int arg1, int arg2) {
		StartTrace1(tester.tester, "arg1:" << arg1 << " arg2:" << arg2);
		if (arg1 == 17 && arg2 == 17) {
			Trace("tester not constructed");
			throw std::logic_error("No construction allowed!");
		}
		mem.check_in(this);
	}
	tester(const tester &) {
		StartTrace1(tester.tester, "copy ctor");
		mem.check_in(this);
	}
	~tester() {
		StartTrace(tester.~tester);
		mem.check_out(this);
	}
};

void BoostPoolTest::BasicsTest()
{
	StartTrace(BoostPoolTest.BasicsTest);
	MemChecker aChecker("BoostPoolTest.BasicsTest", Coast::Storage::Global());
	{
		Trace("empty pool test");
		boost::object_pool<tester> pool;
		assertComparem(0LL, equal_to, aChecker.CheckDelta(), "expected no memory being allocated through our Storage");
	}

	{
		// Construct several tester objects.  Don't delete them (i.e.,
		//  test pool's garbage collection).
		Trace("Testing pool. . .");
		boost::object_pool<tester> pool;
		for (int i = 0; i < 10; ++i) {
			pool.construct(13, 13);
		}
		assertComparem(0LL, equal_to, aChecker.CheckDelta(), "expected no memory being allocated through our Storage");
	}

	{
		// Construct several tester objects.  Delete some of them.
		Trace("Testing pool with some removed. . .");
		boost::object_pool<tester> pool;
		std::vector<tester *> v;
		for (int i = 0; i < 10; ++i) {
			v.push_back(pool.construct(13, 13));
		}
		std::random_shuffle(v.begin(), v.end());
		for (int j = 0; j < 5; ++j) {
			pool.destroy(v[j]);
		}
		assertComparem(0LL, equal_to, aChecker.CheckDelta(), "expected no memory being allocated through our Storage");
	}

	{
		// Test how pool reacts with constructors that throw exceptions.
		//  Shouldn't have any memory leaks.
		Trace("Testing with exceptional constructors :). . .");
		boost::object_pool<tester> pool;
		for (int i = 0; i < 5; ++i) {
			pool.construct(13, 13);
		}
		long lExceptCount = 0L;
		for (int j = 0; j < 5; ++j) {
			try {
				// The following constructor will raise an exception.
				pool.construct(17, 17);
			} catch (const std::logic_error &) {
				++lExceptCount;
			}
		}
		assertCompare(5L, equal_to, lExceptCount);
		assertComparem(0LL, equal_to, aChecker.CheckDelta(), "expected no memory being allocated through our Storage");
	}
}

void BoostPoolTest::AllocationTest()
{
	StartTrace(BoostPoolTest.AllocationTest);
	{
		// Allocate several tester objects.  Delete one.
		Trace("with vector. . .");
		std::vector<tester, boost::pool_allocator<tester> > l;
		for (int i = 0; i < 10; ++i) {
			l.push_back(tester(13, 13));
		}
		l.pop_back();
	}

	{
		// Allocate several tester objects.  Delete two.
		Trace("with deque. . .");
		std::deque<tester, boost::pool_allocator<tester> > l;
		for (int i = 0; i < 10; ++i) {
			l.push_back(tester(13, 13));
		}
		l.pop_back();
		l.pop_front();
	}

	{
		// Allocate several tester objects.  Delete two.
		Trace("with list. . .");
		std::list<tester, boost::fast_pool_allocator<tester> > l;
		// lists rebind their allocators, so dumping is useless
		for (int i = 0; i < 10; ++i) {
			l.push_back(tester(13, 13));
		}
		l.pop_back();
		l.pop_front();
	}

	tester *tmp;
	{
		// Create a memory leak on purpose.  (Allocator doesn't have
		//  garbage collection)
		Trace("Testing allocator cleanup. . .");
		// (Note: memory leak)
		boost::pool_allocator<tester> a;
		tmp = a.allocate(1, 0);
		new (tmp) tester(13, 13);
	}
	if ( !t_assertm( !mem.ok(), "memory count should be different because of memory leak" ) ) {
		SYSERROR("Pool allocator cleaned up, but it shouldnt!");
	}
	// Remove memory checker entry (to avoid error later) and
	//  clean up memory leak
	tmp->~tester();
	boost::pool_allocator<tester>::deallocate(tmp, 1);
}

// This is a wrapper around a UserAllocator.  It just registers alloc/dealloc
//  to/from the system memory.  It's used to make sure pool's are allocating
//  and deallocating system memory properly.
// Do NOT use this class with static or singleton pools.
template <typename UserAllocator>
struct TrackAlloc {
	typedef typename UserAllocator::size_type size_type;
	typedef typename UserAllocator::difference_type difference_type;

	static std::set<char *> allocated_blocks;

	static char *malloc(const size_type bytes) {
		char *const ret = UserAllocator::malloc(bytes);
		StatTrace(TrackAlloc.malloc, "size:" << (long)bytes << " @" << (long)ret << " blocks@" << (long)&allocated_blocks << " size:" << (long)allocated_blocks.size(), Coast::Storage::Global());
		allocated_blocks.insert(ret);
		return ret;
	}
	static void free(char *const block) {
		StatTrace(TrackAlloc.free, "@" << (long)block << " blocks@" << (long)&allocated_blocks << " size:" << (long)allocated_blocks.size(), Coast::Storage::Global());
		if (allocated_blocks.find(block) == allocated_blocks.end()) {
			SYSERROR("Free'd non-malloc'ed block: " << (long)(void *) block);
		}
		allocated_blocks.erase(block);
		UserAllocator::free(block);
	}

	static bool ok() {
		return allocated_blocks.empty();
	}
};
template <typename UserAllocator>
std::set<char *> TrackAlloc<UserAllocator>::allocated_blocks;

typedef TrackAlloc<boost::default_user_allocator_new_delete> track_alloc;

void BoostPoolTest::MemUsageTest()
{
	StartTrace(BoostPoolTest.MemUsageTest);

	typedef boost::pool<track_alloc> pool_type;
	{
		// Constructor should do nothing; no memory allocation
		pool_type pool(sizeof(int));
		t_assertm(track_alloc::ok(), "Memory error");
		t_assertm(!pool.release_memory(), "Pool released memory");
		t_assertm(!pool.purge_memory(), "Pool purged memory");

		// Should allocate from system
		pool.free(pool.malloc());
		Trace("bla");
		t_assertm(!track_alloc::ok(), "Memory error");

		// Ask pool to give up memory it's not using; this should succeed
		t_assertm(pool.release_memory(), "Pool didn't release memory");
		t_assertm(track_alloc::ok(), "Memory error");

		// Should allocate from system again
		pool.malloc(); // loses the pointer to the returned chunk (*A*)

		// Ask pool to give up memory it's not using; this should fail
		t_assertm(!pool.release_memory(), "Pool released memory");

		// Force pool to give up memory it's not using; this should succeed
		//  This will clean up the memory leak from (*A*)
		t_assertm(pool.purge_memory(), "Pool didn't purge memory");
		t_assertm(track_alloc::ok(), "Memory error");

		// Should allocate from system again
		pool.malloc(); // loses the pointer to the returned chunk (*B*)

		// pool's destructor should purge the memory
		//  This will clean up the memory leak from (*B*)
	}

	t_assertm(track_alloc::ok(), "Memory error");
}

typedef TrackAlloc<ITOStorage::BoostPoolUserAllocatorGlobal> track_allocWD;

void BoostPoolTest::GlobalStorageMemUsageTest()
{
	StartTrace(BoostPoolTest.GlobalStorageMemUsageTest);

	typedef boost::pool<track_allocWD> pool_typeWD;
	MemChecker aChecker("BoostPoolTest.GlobalStorageMemUsageTest", Coast::Storage::Global());
	{
		// Constructor should do nothing; no memory allocation
		pool_typeWD pool(sizeof(int));
		t_assertm(track_allocWD::ok(), "Memory error");
		t_assertm(!pool.release_memory(), "Pool released memory");
		t_assertm(!pool.purge_memory(), "Pool purged memory");

		// Should allocate from system
		pool.free(pool.malloc());
		assertComparem(32LL * sizeof(track_allocWD), less, aChecker.CheckDelta(), "expected no unfreed memory");
		t_assertm(!track_allocWD::ok(), "Memory error");

		// Ask pool to give up memory it's not using; this should succeed
		t_assertm(pool.release_memory(), "Pool didn't release memory");
		t_assertm(track_allocWD::ok(), "Memory error");
		assertComparem(0LL, equal_to, aChecker.CheckDelta(), "expected no unfreed memory");

		// Should allocate from system again
		pool.malloc(); // loses the pointer to the returned chunk (*A*)

		// Ask pool to give up memory it's not using; this should fail
		t_assertm(!pool.release_memory(), "Pool released memory");

		// Force pool to give up memory it's not using; this should succeed
		//  This will clean up the memory leak from (*A*)
		t_assertm(pool.purge_memory(), "Pool didn't purge memory");
		t_assertm(track_allocWD::ok(), "Memory error");

		// Should allocate from system again
		pool.malloc(); // loses the pointer to the returned chunk (*B*)

		// pool's destructor should purge the memory
		//  This will clean up the memory leak from (*B*)
	}

	t_assertm(track_allocWD::ok(), "Memory error");
	assertComparem(0LL, equal_to, aChecker.CheckDelta(), "expected no unfreed memory");
}

typedef boost::function<void ()> TestCaseFunc;

class CurrentStorageThread : public Thread
{
	TestCaseFunc &fFunc;
public:
	CurrentStorageThread(TestCaseFunc &f)
		: Thread("CurrentStorageThread")
		, fFunc(f)
	{}
	void Run() {
		fFunc();
	}
};

void BoostPoolTest::CurrentStorageMemUsageTest()
{
	StartTrace(BoostPoolTest.CurrentStorageMemUsageTest);
	{
		Trace("doing Test with global storage again");
		TestCaseFunc f(boost::bind(&BoostPoolTest::TestFuncCurrent, this));
		CurrentStorageThread aThread(f);
		aThread.Start(Coast::Storage::Global());
		aThread.CheckState(Thread::eTerminated);
	}
	{
		Trace("doing Test with pool storage");
		TestCaseFunc f(boost::bind(&BoostPoolTest::TestFuncCurrent, this));
		CurrentStorageThread aThread(f);
		aThread.Start(MT_Storage::MakePoolAllocator(1024, 10, 0));
		aThread.CheckState(Thread::eTerminated);
	}
}

typedef TrackAlloc<ITOStorage::BoostPoolUserAllocatorCurrent> track_allocWDCurr;

void BoostPoolTest::TestFuncCurrent()
{
	StartTrace(BoostPoolTest.TestFuncCurrent);

	typedef boost::pool<track_allocWDCurr> pool_typeWDCurr;
	MemChecker aCChecker("BoostPoolTest.TestFuncCurrent", Coast::Storage::Current());
	{
		// Constructor should do nothing; no memory allocation
		pool_typeWDCurr pool(sizeof(int));
		t_assertm(track_allocWDCurr::ok(), "Memory error");
		t_assertm(!pool.release_memory(), "Pool released memory");
		t_assertm(!pool.purge_memory(), "Pool purged memory");

		// Should allocate from system
		pool.free(pool.malloc());
		assertComparem(32LL * sizeof(track_allocWDCurr), less, aCChecker.CheckDelta(), "expected no unfreed memory");
		t_assertm(!track_allocWDCurr::ok(), "Memory error");

		// Ask pool to give up memory it's not using; this should succeed
		t_assertm(pool.release_memory(), "Pool didn't release memory");
		t_assertm(track_allocWDCurr::ok(), "Memory error");
		assertComparem(0LL, greater_equal, aCChecker.CheckDelta(), TString("expected no unfreed memory ") << (Coast::Storage::Current() == Coast::Storage::Global() ? "G" : "C"));

		// Should allocate from system again
		pool.malloc(); // loses the pointer to the returned chunk (*A*)

		// Ask pool to give up memory it's not using; this should fail
		t_assertm(!pool.release_memory(), "Pool released memory");

		// Force pool to give up memory it's not using; this should succeed
		//  This will clean up the memory leak from (*A*)
		t_assertm(pool.purge_memory(), "Pool didn't purge memory");
		t_assertm(track_allocWDCurr::ok(), "Memory error");

		// Should allocate from system again
		pool.malloc(); // loses the pointer to the returned chunk (*B*)

		// pool's destructor should purge the memory
		//  This will clean up the memory leak from (*B*)
	}

	t_assertm(track_allocWDCurr::ok(), "Memory error");
	assertComparem(0LL, greater_equal, aCChecker.CheckDelta(), TString("expected no unfreed memory ") << (Coast::Storage::Current() == Coast::Storage::Global() ? "G" : "C"));
}

// ----------------------------------------------------------------------------

template<unsigned int N>
class ThisIsASmallObject
{
	char data[N];
};

template<unsigned int N, class T>
struct Base : public ThisIsASmallObject<N>, public T
	{};

template<unsigned int N>
struct Base<N, void> : public ThisIsASmallObject<N>
	{};

// ----------------------------------------------------------------------------
template <
typename Tag,
		 unsigned int N,
		 typename UserAllocator = boost::default_user_allocator_new_delete
		 >
class BoostSingletonPool : public Base<N, void>
{
public:
	typedef boost::singleton_pool< Tag, N, UserAllocator > BoostPoolType;

private:
	static BoostPoolType BoostPool;

public:
	/// Throwing single-object new throws bad_alloc when allocation fails.
	static void *operator new ( std::size_t ) throw ( std::bad_alloc )
	{
		return BoostPoolType::malloc();
	}

	/// Non-throwing single-object new returns NULL if allocation fails.
	static void *operator new ( std::size_t, const std::nothrow_t & ) throw () {
		return BoostPoolType::malloc();
	}

	/// Placement single-object new merely calls global placement new.
	inline static void *operator new ( std::size_t size, void *place ) {
		return ::operator new( size, place );
	}

	/// Single-object delete.
	static void operator delete ( void *p ) throw () {
		BoostPoolType::free( reinterpret_cast< BoostSingletonPool * >( p ) );
	}

	/** Non-throwing single-object delete is only called when nothrow
	    new operator is used, and the constructor throws an exception.
	    */
	static void operator delete ( void *p, const std::nothrow_t & ) throw() {
		BoostPoolType::free( reinterpret_cast< BoostSingletonPool * >( p ) );
	}

	/// Placement single-object delete merely calls global placement delete.
	inline static void operator delete ( void *p, void *place ) {
		::operator delete ( p, place );
	}

	/** @note This class does not provide new [] and delete [] operators since
	 the Boost.Pool allocator only works for memory requests of the same size.
	 */
};

template <
typename Tag,
		 unsigned int N,
		 typename UserAllocator
		 >
typename BoostSingletonPool<Tag, N, UserAllocator>::BoostPoolType BoostSingletonPool<Tag, N, UserAllocator>::BoostPool;

template <
typename tPoolType
>
void cleanupBoostPool()
{
	tPoolType::BoostPoolType::release_memory();
}

// ----------------------------------------------------------------------------

typedef boost::function<void (void **, int)> theTestFunc;

template <
int loop
>
void BasicTestMethod(theTestFunc tstFunc)
{
	int Narr = 500 * 1000;

	void **a = new void*[Narr];

	tstFunc(a, loop);

	delete [] a;
}

template <class T>
struct dummyAlloc {};

#define _SMALLOBJ_BENCH(FUNC, CODE_DECL, CODE_LOOP)                                 \
template <class T, const int N, template <class> class tAlloc>                                                    \
void FUNC(void**, int loop)             \
{                                                                            \
	CODE_DECL                                                            \
    for (int i=0; i<loop; ++i)                                               \
    {                                                                        \
        CODE_LOOP                                                            \
    }                                                                        \
}

#define _SMALLOBJ_BENCH_ARRAY(FUNC, CODE_DECL, CODE_NEW, CODE_DELETE)                                 \
template <class T, const int N, template <class> class tAlloc>                                                    \
void FUNC(void**arrv, int loop)             \
{                                                                            \
    CODE_DECL                                                               \
    T** arr = reinterpret_cast<T**>(arrv);                                   \
    for (int i=0; i<loop; ++i)                                               \
    {                                                                        \
        CODE_NEW                                                             \
    }                                                                        \
    for (int i=0; i<loop; ++i)                                               \
    {                                                                        \
        CODE_DELETE                                                          \
    }                                                                        \
}

// first set of methods
_SMALLOBJ_BENCH(delete_new          , , delete new T;)
_SMALLOBJ_BENCH(delete_new_mal      , , std::free(std::malloc(sizeof(T)));)
_SMALLOBJ_BENCH(delete_new_all      , tAlloc<T> st; , st.deallocate(st.allocate(1), 1);)

// second set of methods
_SMALLOBJ_BENCH(delete_new_array    , , delete[] new T[N];)
_SMALLOBJ_BENCH(delete_new_array_mal, , std::free(std::malloc(sizeof(T[N])));)
_SMALLOBJ_BENCH(delete_new_array_all, tAlloc<T[N]> st; , st.deallocate(st.allocate(1), 1);)

_SMALLOBJ_BENCH_ARRAY(new_del_on_arr    , , arr[i] = new T; ,
					  delete arr[i];)
_SMALLOBJ_BENCH_ARRAY(new_del_on_arr_mal, , arr[i] = static_cast<T *>(std::malloc(sizeof(T))); ,
					  std::free(arr[i]);)
_SMALLOBJ_BENCH_ARRAY(new_del_on_arr_all,    tAlloc<T> st; ,
					  arr[i] = st.allocate(1); ,
					  st.deallocate(arr[i], 1);)

_SMALLOBJ_BENCH_ARRAY(new_del_a_on_a    , , arr[i] = new T[N]; ,
					  delete[] arr[i];)
_SMALLOBJ_BENCH_ARRAY(new_del_a_on_a_mal, , arr[i] = static_cast<T *>(std::malloc(sizeof(T[N]))); ,
					  std::free(arr[i]);)
_SMALLOBJ_BENCH_ARRAY(new_del_a_on_a_all, tAlloc<T[N]> st; ,
					  arr[i] = reinterpret_cast<T *>(st.allocate(1)); ,
					  st.deallocate(reinterpret_cast<T( *)[N]>(arr[i]), 1);)

typedef boost::function<void (theTestFunc)> WorkerFunc;
typedef boost::function<void ()> MemCleanupFunc;

void cleanupDummy() {}

template <
typename funcType,
		 typename innerFunc,
		 typename cleanupFunc
		 >
class TestMethodWorkerThread : public Thread
{
	funcType fFunc;
	innerFunc fInnerFunc;
	cleanupFunc fCleanupFunc;
public:
	TestMethodWorkerThread(funcType f, innerFunc fi, cleanupFunc cf)
		: Thread("TestMethodWorkerThread")
		, fFunc(f)
		, fInnerFunc(fi)
		, fCleanupFunc(cf) {
	}
	void Run() {
		SetWorking();
		fFunc(fInnerFunc);
		fCleanupFunc();
		SetReady();
	}
};

template <
typename funcType,
		 typename innerFunc,
		 typename cleanupFunc
		 >
class TestMethodWorkerThreadPool : public ThreadPoolManager
{
	funcType fFunc;
	innerFunc fInnerFunc;
	cleanupFunc fCleanupFunc;
public:
	TestMethodWorkerThreadPool(const char *pTPName, funcType f, innerFunc fi, cleanupFunc cf)
		: ThreadPoolManager( pTPName )
		, fFunc(f)
		, fInnerFunc(fi)
		, fCleanupFunc(cf)
	{}

	~TestMethodWorkerThreadPool()
	{}

	Thread *DoAllocThread(long i, ROAnything args) {
		StartTrace(TestMethodWorkerThreadPool.DoAllocThread);
		TraceAny(args, "Init-Arguments");
		return new (Coast::Storage::Global()) TestMethodWorkerThread<funcType, innerFunc, cleanupFunc>(fFunc, fInnerFunc, fCleanupFunc);
	}

	ROAnything DoGetInitConfig(long i, ROAnything args) {
		StartTrace(TestMethodWorkerThreadPool.DoGetInitConfig);
		// use same config for all threads
		return args;
	}

	ROAnything DoGetStartConfig(long i, ROAnything args) {
		StartTrace(TestMethodWorkerThreadPool.DoGetStartConfig);
		// use same config for all threads
		return args;
	}
};

// builds up a suite of testcases, add a line for each testmethod
Test *BoostPoolTest::suite ()
{
	StartTrace(BoostPoolTest.suite);
	TestSuite *testSuite = new TestSuite;
	ADD_CASE(testSuite, BoostPoolTest, BasicsTest);
	ADD_CASE(testSuite, BoostPoolTest, AllocationTest);
	ADD_CASE(testSuite, BoostPoolTest, MemUsageTest);
	ADD_CASE(testSuite, BoostPoolTest, GlobalStorageMemUsageTest);
	ADD_CASE(testSuite, BoostPoolTest, CurrentStorageMemUsageTest);
	ADD_CASE(testSuite, BoostPoolTest, ExportCsvStatistics);
	return testSuite;
}
