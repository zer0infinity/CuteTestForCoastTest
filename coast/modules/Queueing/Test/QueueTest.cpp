/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

//--- interface include --------------------------------------------------------
#include "QueueTest.h"

//--- test modules used --------------------------------------------------------
#include "TestSuite.h"

//--- module under test --------------------------------------------------------
#include "Queue.h"

//--- standard modules used ----------------------------------------------------
#include "PoolAllocator.h"
#include "Dbg.h"
#include "SystemBase.h"

using namespace Coast;

//--- c-modules used -----------------------------------------------------------

class DCDStateMachine
{
public:

	// constructors
	DCDStateMachine();

	// events
	struct ev_ReloadMarketCodeFilter {
		ev_ReloadMarketCodeFilter(Anything &anyElement)
			: fanyElement(anyElement)
			, froaElement(fanyElement)
		{}
		Anything fanyElement;
		ROAnything froaElement;
	};
};

template < class theDCDStateMachine >
class dcd_event
{
	template<typename ValueType>
	dcd_event &operator=(const ValueType &);
	dcd_event &operator=(const dcd_event &);

public: // structors

	dcd_event(const dcd_event &other)
		: content(other.content ? other.content->clone() : 0) {
	}
	dcd_event()
		: content(0) {
	}

	template<typename ValueType>
	dcd_event(const ValueType &value)
		: content(new holder<ValueType>(value)) {
	}

	~dcd_event() {
		delete content;
	}

public: // queries

	bool empty() const {
		return !content;
	}

	const std::type_info &type() const {
		return content ? content->type() : typeid(void);
	}

#ifndef BOOST_NO_MEMBER_TEMPLATE_FRIENDS
private: // types
#else
public: // types (public so any_cast can be non-friend)
#endif

	class placeholder
	{
	public: // structors

		virtual ~placeholder() {
		}

	public: // queries

		virtual bool process(theDCDStateMachine &rStateMachine) = 0;

		virtual const std::type_info &type() const = 0;

		virtual placeholder *clone() const = 0;

	};

	template<typename ValueType>
	class holder : public placeholder
	{
	public: // structors

		holder(const ValueType &value)
			: held(value) {
		}

	public: // queries
		virtual bool process(theDCDStateMachine &rStateMachine) {
//				rStateMachine.process_event(held);
//				return rStateMachine.successfull_state_transition();
			return false;
		}

		virtual const std::type_info &type() const {
			return typeid(ValueType);
		}

		virtual placeholder *clone() const {
			return new holder(held);
		}

	public: // representation

		ValueType held;

	};

	placeholder *content;
};

class TestConsumer : public Thread
{
	friend class QueueTest;
public:
	typedef AnyQueueType &AnyQueueTypeRef;

	TestConsumer(AnyQueueTypeRef aQueue, long lHowManyConsumes, long lWaitTimeMicroSec = 0L)
		: Thread("TestConsumer"), fQueue(aQueue), fWaitTimeMicroSec(lWaitTimeMicroSec), fToConsume(lHowManyConsumes), fConsumed(0L)
	{}
	~TestConsumer()
	{}
	bool DoStartRequestedHook(ROAnything roaWork) {
		fWork = roaWork.DeepClone();
		return true;
	}
	void Run() {
		StartTrace(TestConsumer.Run);
		long lProductCount = 0L;
		bool bTryLock = fWork["TryLock"].AsBool(false);
		while ( lProductCount < fToConsume ) {
			Anything anyProduct;
			if ( fQueue.Get(anyProduct, bTryLock) == AnyQueueType::eSuccess ) {
				TraceAny(anyProduct, "consumed product");
				fProducts[lProductCount] = anyProduct;
				if (fWaitTimeMicroSec > 0L) {
					System::MicroSleep(fWaitTimeMicroSec);
				}
				lProductCount++;
			}
		}
	}

private:
	AnyQueueTypeRef fQueue;
	long fWaitTimeMicroSec, fToConsume, fConsumed;
	Anything fWork, fProducts;
};

class TestProducer : public Thread
{
public:
	typedef AnyQueueType &AnyQueueTypeRef;

	TestProducer(AnyQueueTypeRef aQueue, long lHowManyProduces, long lWaitTimeMicroSec = 0L)
		: Thread("TestProducer"), fQueue(aQueue), fWaitTimeMicroSec(lWaitTimeMicroSec), fToProduce(lHowManyProduces), fProduced(0L)
	{}
	~TestProducer()
	{}
	bool DoStartRequestedHook(ROAnything roaWork) {
		fWork = roaWork.DeepClone();
		return true;
	}
	void Run() {
		StartTrace(TestProducer.Run);
		long lProductCount = 0L;
		bool bTryLock = fWork["TryLock"].AsBool(false);
		while ( lProductCount < fToProduce ) {
			Anything anyProduct = fWork["Product"].DeepClone();
			anyProduct["ThreadId"] = GetId();
			anyProduct["ProductNumber"] = lProductCount;
			if ( fQueue.Put(anyProduct, bTryLock) == AnyQueueType::eSuccess ) {
				TraceAny(anyProduct, "produced product");
				if (fWaitTimeMicroSec) {
					System::MicroSleep(fWaitTimeMicroSec);
				}
				lProductCount++;
			}
		}
	}

private:
	AnyQueueTypeRef fQueue;
	long fWaitTimeMicroSec, fToProduce, fProduced;
	Anything fWork;
};

class ConsumerTerminationThread : public Thread
{
	friend class QueueTest;
public:
	typedef AnyQueueType &AnyQueueTypeRef;

	ConsumerTerminationThread(AnyQueueTypeRef aQueue)
		: Thread("ConsumerTerminationThread"), fQueue(aQueue), fConsumed(0L)
	{}
	~ConsumerTerminationThread()
	{}
	void Run() {
		StartTrace(ConsumerTerminationThread.Run);
		// signal start using working state
		Trace("before CheckRunningState(eWorking)");
		while ( CheckState( eRunning, 0, 1 ) ) {
			CheckRunningState(eWorking);
			Trace("now working");
			Anything anyProduct;
			if ( fQueue.Get(anyProduct, false) == AnyQueueType::eSuccess ) {
				TraceAny(anyProduct, "consumed product");
				fConsumed++;
			}
		}
	}

private:
	AnyQueueTypeRef fQueue;
	long fConsumed;
};

typedef AnyQueueType &AnyQueueTypeRef;

//---- QueueTest ----------------------------------------------------------------
QueueTest::QueueTest(TString tstrName) : TestCaseType(tstrName)
{
	StartTrace(QueueTest.Ctor);
}

QueueTest::~QueueTest()
{
	StartTrace(QueueTest.Dtor);
}

void QueueTest::BlockingSideTest()
{
	StartTrace(QueueTest.BlockingSideTest);
	AnyQueueType Q1("Q1", 1);
	assertEqualm(AnyQueueType::eNone, Q1.feBlocked, "expected unblocked put and get side");
	t_assertm(!Q1.IsBlocked(), "expected queue not to be blocked");
	t_assertm(!Q1.IsBlocked(AnyQueueType::eBothSides), "expected queue not to be blocked");
	t_assertm(!Q1.IsBlocked(AnyQueueType::ePutSide), "expected put side not to be blocked");
	t_assertm(!Q1.IsBlocked(AnyQueueType::eGetSide), "expected get side not to be blocked");
	Q1.Block(AnyQueueType::ePutSide);
	assertEqual(AnyQueueType::ePutSide, Q1.feBlocked);
	t_assertm(Q1.IsBlocked(AnyQueueType::ePutSide), "expected put side to be blocked");
	t_assertm(!Q1.IsBlocked(AnyQueueType::eGetSide), "expected get side not to be blocked");
	t_assertm(!Q1.IsBlocked(AnyQueueType::eBothSides), "expected not both sides to be blocked");
	Q1.UnBlock(AnyQueueType::eBothSides);
	t_assertm(!Q1.IsBlocked(), "expected queue not to be blocked");
	Q1.Block(AnyQueueType::eGetSide);
	assertEqual(AnyQueueType::eGetSide, Q1.feBlocked);
	t_assertm(Q1.IsBlocked(AnyQueueType::eGetSide), "expected get side to be blocked");
	t_assertm(!Q1.IsBlocked(AnyQueueType::ePutSide), "expected put side not to be blocked");
	t_assertm(!Q1.IsBlocked(AnyQueueType::eBothSides), "expected not both sides to be blocked");
	Q1.Block();
	assertEqual(AnyQueueType::eBothSides, Q1.feBlocked);
	t_assertm(Q1.IsBlocked(), "expected queue to be blocked");
	t_assertm(Q1.IsBlocked(AnyQueueType::eBothSides), "expected both sides to be blocked");
	t_assertm(Q1.IsBlocked(AnyQueueType::ePutSide), "expected put side to be blocked");
	t_assertm(Q1.IsBlocked(AnyQueueType::eGetSide), "expected get side to be blocked");
}

void QueueTest::PutGetStatusTest()
{
	StartTrace(QueueTest.PutGetStatusTest);
	AnyQueueType Q1("Q1", 1);
	t_assert(Q1.GetSize() == 0L);
	Anything anyTest1, anyTest2;
	anyTest1["Guguseli"] = 1;
	assertEqualm(AnyQueueType::eSuccess, Q1.DoPut(anyTest1), "first put should succeed");
	t_assert(Q1.GetSize() == 1L);
	Anything anyOut;
	assertEqualm(AnyQueueType::eSuccess, Q1.DoGet(anyOut), "first get should succeed");
	assertAnyEqual(anyTest1, anyOut);
	t_assert(Q1.GetSize() == 0L);
	anyOut = Anything();
	assertEqualm((AnyQueueType::eEmpty | AnyQueueType::eError), Q1.DoGet(anyOut), "second get should fail because of empty queue");
}

void QueueTest::SimplePutGetTest()
{
	StartTrace(QueueTest.SimplePutGetTest);
	AnyQueueType Q1("Q1", 1);
	t_assert(Q1.GetSize() == 0L);
	Anything anyTest1, anyTest2;
	anyTest1["Guguseli"] = 1;
	// first call should not block
	assertEqual(AnyQueueType::eSuccess, Q1.Put(anyTest1));
	anyTest2["Guguseli"] = 2;
	// second call should block so use a trylock and check return code
	assertEqual(AnyQueueType::eFull, Q1.Put(anyTest2, true));
	t_assert(Q1.GetSize() == 1L);
	Anything anyOut;
	// first get should work without blocking
	assertEqual(AnyQueueType::eSuccess, Q1.Get(anyOut));
	assertAnyEqual(anyTest1, anyOut);
	t_assert(Q1.GetSize() == 0L);
	// second get should block because of empty queue, so use trylock and check return code
	anyOut = Anything();
	assertEqual(AnyQueueType::eEmpty, Q1.Get(anyOut, true));
	Q1.GetStatistics(anyOut);
	assertEqual(1L, anyOut["QueueSize"].AsLong(0L));
	assertEqual(1L, anyOut["MaxLoad"].AsLong(0L));
	TraceAny(anyOut, "statistics");
}

void QueueTest::DoMultiProducerSingleConsumerTest(long lQueueSize)
{
	StartTrace1(QueueTest.DoMultiProducerSingleConsumerTest, "QueueSize:" << lQueueSize);
	{
		AnyQueueType aProductQueue("aProductQueue", lQueueSize);
		TestProducer aProd4(aProductQueue, 4L, 1500L), aProd5(aProductQueue, 5L, 2000L), aProd10(aProductQueue, 10L, 1000L);
		TestConsumer aConsumer(aProductQueue, 19L);
		Anything anyCons, anyProd;
		anyCons["TryLock"] = false;
		anyProd["TryLock"] = false;
		aConsumer.Start(Storage::Global(), anyCons);
		anyProd["Product"] = "Prod10";
		aProd10.Start(Storage::Global(), anyProd);
		anyProd["Product"] = "Prod4";
		aProd4.Start(Storage::Global(), anyProd);
		anyProd["Product"] = "Prod5";
		aProd5.Start(Storage::Global(), anyProd);
		// wait for 10s on consumer to terminate
		t_assert(aConsumer.CheckState(Thread::eTerminated, 10));
		TraceAny(aConsumer.fProducts, "produced items");
		assertEqual(19L, aConsumer.fProducts.GetSize());
		Anything anyStat;
		aProductQueue.GetStatistics(anyStat);
		TraceAny(anyStat, "statistics");
	}
	{
		AnyQueueType aProductQueue("aProductQueue", lQueueSize);
		TestProducer aProd4(aProductQueue, 4L), aProd5(aProductQueue, 5L), aProd10(aProductQueue, 10L);
		TestConsumer aConsumer(aProductQueue, 19L);
		Anything anyCons, anyProd;
		anyCons["TryLock"] = false;
		anyProd["TryLock"] = false;
		aConsumer.Start(Storage::Global(), anyCons);
		anyProd["Product"] = "Prod10";
		aProd10.Start(Storage::Global(), anyProd);
		anyProd["Product"] = "Prod4";
		aProd4.Start(Storage::Global(), anyProd);
		anyProd["Product"] = "Prod5";
		aProd5.Start(Storage::Global(), anyProd);
		// wait for 10s on consumer to terminate
		t_assert(aConsumer.CheckState(Thread::eTerminated, 10));
		TraceAny(aConsumer.fProducts, "produced items");
		assertEqual(19L, aConsumer.fProducts.GetSize());
		Anything anyStat;
		aProductQueue.GetStatistics(anyStat);
		TraceAny(anyStat, "statistics");
	}
}

void QueueTest::MultiProducerSingleConsumerTest()
{
	StartTrace(QueueTest.MultiProducerSingleConsumerTest);
	DoMultiProducerSingleConsumerTest(1L);
	DoMultiProducerSingleConsumerTest(25L);
	DoMultiProducerSingleConsumerTest(5L);
}

void QueueTest::DoSingleProducerMultiConsumerTest(long lQueueSize)
{
	StartTrace(QueueTest.DoSingleProducerMultiConsumerTest);
	{
		AnyQueueType aProductQueue("aProductQueue", lQueueSize);
		TestConsumer aCons4(aProductQueue, 4L, 1500L), aCons5(aProductQueue, 5L, 2000L), aCons10(aProductQueue, 10L, 1000L);
		TestProducer aProducer(aProductQueue, 19L);
		Anything anyCons, anyProd;
		anyCons["TryLock"] = false;
		anyProd["TryLock"] = false;
		aCons4.Start(Storage::Global(), anyCons);
		aCons5.Start(Storage::Global(), anyCons);
		aCons10.Start(Storage::Global(), anyCons);
		anyProd["Product"] = "Gugus";
		aProducer.Start(Storage::Global(), anyProd);
		// wait for 10s on consumer to terminate
		t_assert(aCons4.CheckState(Thread::eTerminated, 10));
		t_assert(aCons5.CheckState(Thread::eTerminated, 10));
		t_assert(aCons10.CheckState(Thread::eTerminated, 10));
		TraceAny(aCons4.fProducts, "consumer4 items");
		TraceAny(aCons5.fProducts, "consumer5 items");
		TraceAny(aCons10.fProducts, "consumer10 items");
		assertEqual(4L, aCons4.fProducts.GetSize());
		assertEqual(5L, aCons5.fProducts.GetSize());
		assertEqual(10L, aCons10.fProducts.GetSize());
		Anything anyStat;
		aProductQueue.GetStatistics(anyStat);
		TraceAny(anyStat, "statistics");
	}
	{
		AnyQueueType aProductQueue("aProductQueue", lQueueSize);
		TestConsumer aCons4(aProductQueue, 4L), aCons5(aProductQueue, 5L), aCons10(aProductQueue, 10L);
		TestProducer aProducer(aProductQueue, 19L);
		Anything anyCons, anyProd;
		anyCons["TryLock"] = false;
		anyProd["TryLock"] = false;
		aCons4.Start(Storage::Global(), anyCons);
		aCons5.Start(Storage::Global(), anyCons);
		aCons10.Start(Storage::Global(), anyCons);
		anyProd["Product"] = "Gugus";
		aProducer.Start(Storage::Global(), anyProd);
		// wait for 10s on consumer to terminate
		t_assert(aCons4.CheckState(Thread::eTerminated, 10));
		t_assert(aCons5.CheckState(Thread::eTerminated, 10));
		t_assert(aCons10.CheckState(Thread::eTerminated, 10));
		TraceAny(aCons4.fProducts, "consumer4 items");
		TraceAny(aCons5.fProducts, "consumer5 items");
		TraceAny(aCons10.fProducts, "consumer10 items");
		assertEqual(4L, aCons4.fProducts.GetSize());
		assertEqual(5L, aCons5.fProducts.GetSize());
		assertEqual(10L, aCons10.fProducts.GetSize());
		Anything anyStat;
		aProductQueue.GetStatistics(anyStat);
		TraceAny(anyStat, "statistics");
	}
}

void QueueTest::SingleProducerMultiConsumerTest()
{
	StartTrace(QueueTest.SingleProducerMultiConsumerTest);
	DoSingleProducerMultiConsumerTest(1L);
	DoSingleProducerMultiConsumerTest(5L);
	DoSingleProducerMultiConsumerTest(25L);
}

void QueueTest::ConsumerTerminationTest()
{
	StartTrace(QueueTest.ConsumerTerminationTest);
	{
		AnyQueueType *pProductQueue = new (Storage::Global()) AnyQueueType("pProductQueue", 2);
		ConsumerTerminationThread aConsumer(*pProductQueue);
		{
			Anything anyProduct;
			anyProduct["What"] = "Yippieh";
			assertEqual(AnyQueueType::eSuccess, pProductQueue->Put(anyProduct));
		}
		// start activity of thread
		aConsumer.Start();
		assertEqualm(1L, pProductQueue->GetSize(), "expected queue to still contain 1 element");
		t_assert(aConsumer.CheckState(Thread::eRunning, 10));
		// allow thread to consume element
		aConsumer.SetWorking();
		aConsumer.CheckRunningState(Thread::eWorking);
		System::MicroSleep(5000);
		assertEqualm(0L, pProductQueue->GetSize(), "expected element to be consumed");
		t_assert(aConsumer.CheckState(Thread::eRunning, 10));
		aConsumer.Terminate(1);
		t_assert(aConsumer.CheckState(Thread::eTerminationRequested, 10));
		delete pProductQueue;
		t_assert(aConsumer.CheckState(Thread::eTerminated, 10));
	}
	{
		AnyQueueType *pProductQueue = new (Storage::Global()) AnyQueueType("pProductQueue", 2);
		ConsumerTerminationThread aConsumer(*pProductQueue);
		{
			Anything anyProduct;
			anyProduct["What"] = "Yippieh";
			assertEqual(AnyQueueType::eSuccess, pProductQueue->Put(anyProduct));
		}
		// start activity of thread
		aConsumer.Start();
		assertEqualm(1L, pProductQueue->GetSize(), "expected queue to still contain 1 element");
		t_assert(aConsumer.CheckState(Thread::eRunning, 10));
		// allow thread to consume element
		aConsumer.SetWorking();
		aConsumer.CheckRunningState(Thread::eWorking);
		System::MicroSleep(5000);
		assertEqualm(0L, pProductQueue->GetSize(), "expected element to be consumed");
		t_assert(aConsumer.CheckState(Thread::eRunning, 10));
		aConsumer.Terminate(1);
		t_assert(aConsumer.CheckState(Thread::eTerminationRequested, 10));
		assertEqualm(1L, pProductQueue->fBlockingGetCount, "expected 1 blocking getter");
		// force termination of thread through queue blocking
		pProductQueue->Block(AnyQueueType::eGetSide);
		t_assertm(pProductQueue->IsBlocked(AnyQueueType::eGetSide), "expected get side to be blocked");
		t_assert(aConsumer.CheckState(Thread::eTerminated, 10));
		assertEqualm(0L, pProductQueue->fBlockingGetCount, "expected 0 blocking getter");
		delete pProductQueue;
	}
}

/*
//TODO - future: change DoGet() method, so that it's not restricted only for Anything's (see Queue.h)
*/
typedef Queue<long, Anything> LongAnyQueueType;
void QueueTest::SimpleTypeAnyStorageQueueTest()
{
	StartTrace(QueueTest.SimpleTypeAnyStorageQueueTest);

	LongAnyQueueType Q1("Q1");

//	long lValue = 0;
//	assertEqualm(0L, Q1.GetSize(), "expected queue to be empty");
//	Q1.Put( ( lValue = 20L ) );
//	assertEqualm(1L, Q1.GetSize(), "expected queue to contain 1 element");
//	Q1.Put( ( lValue = 50L ) );
//	assertEqualm(2L, Q1.GetSize(), "expected queue to contain 2 elements");
//	Q1.Get(lValue);
//	assertEqualm(1L, Q1.GetSize(), "expected queue to contain 1 element");
//	Q1.Get(lValue);
//	assertEqualm(0L, Q1.GetSize(), "expected queue to be empty");
}

typedef	dcd_event< DCDStateMachine > EventType;
typedef	EventType *EventTypePtr;
#if defined(__GNUG__)  && ( __GNUC__ >= 4 )
typedef STLStorage::fast_pool_allocator_global< EventTypePtr > EvtAllocType;
typedef Queue< EventTypePtr, std::list<EventTypePtr, EvtAllocType > > EventQueueType;
#else
typedef Queue< EventTypePtr, std::list<EventTypePtr > > EventQueueType;
#endif
void QueueTest::SimpleTypeListStorageQueueTest()
{
	StartTrace(QueueTest.SimpleTypeListStorageQueueTest);

	Anything anyMsg, anyMsg2;
	anyMsg["Content"] = "DummyContent1";
	anyMsg2["Content"] = "DummyContent2";

	EventTypePtr pEventWrapper = new EventType( DCDStateMachine::ev_ReloadMarketCodeFilter(anyMsg) );
	EventTypePtr pEventWrapper2 = new EventType( DCDStateMachine::ev_ReloadMarketCodeFilter(anyMsg2) );
	EventTypePtr pEventWrapperOut2 = NULL;

	EventQueueType Q1("Q1");

	assertEqualm(0L, Q1.GetSize(), "expected queue to be empty");
	Q1.Put(pEventWrapper);
	assertEqualm(1L, Q1.GetSize(), "expected queue to contain 1 element");
	Q1.Put(pEventWrapper2);
	assertEqualm(2L, Q1.GetSize(), "expected queue to contain 2 elements");
	Q1.Get(pEventWrapperOut2);
	assertEqualm(1L, Q1.GetSize(), "expected queue to contain 1 element");
	Q1.Get(pEventWrapperOut2);
	assertEqualm(0L, Q1.GetSize(), "expected queue to be empty");

	delete pEventWrapper;
	pEventWrapper = NULL;
	delete pEventWrapperOut2;
	pEventWrapperOut2 = NULL;
}

void QueueTest::QueueWithAllocatorTest()
{
	StartTrace(QueueTest.QueueWithAllocatorTest);
	{
		PoolAllocator aPoolAlloc(1234, 1234, 18);

		l_long lAllocMark = aPoolAlloc.CurrentlyAllocated();
		{
			AnyQueueType Q1("Q1", 1, &aPoolAlloc);
			t_assert(Q1.GetSize() == 0L);
			Anything anyTest1, anyTest2;
			anyTest1["Guguseli"] = 1;
			// first call should not block
			assertEqual(AnyQueueType::eSuccess, Q1.Put(anyTest1));
			anyTest2["Guguseli"] = 2;
			// second call should block so use a trylock and check return code
			assertEqual(AnyQueueType::eFull, Q1.Put(anyTest2, true));
			t_assert(Q1.GetSize() == 1L);

			if ( Storage::GetStatisticLevel() >= 1 ) {
				assertComparem(lAllocMark, less, aPoolAlloc.CurrentlyAllocated(), "expected PoolAllocator having had some allocations");
			}

			Anything anyOut;
			// first get should work without blocking
			assertEqual(AnyQueueType::eSuccess, Q1.Get(anyOut));
			assertAnyEqual(anyTest1, anyOut);
			t_assert(Q1.GetSize() == 0L);
			// second get should block because of empty queue, so use trylock and check return code
			anyOut = Anything();
			assertEqual(AnyQueueType::eEmpty, Q1.Get(anyOut, true));
			Q1.GetStatistics(anyOut);
			assertEqual(1L, anyOut["QueueSize"].AsLong(0L));
			assertEqual(1L, anyOut["MaxLoad"].AsLong(0L));
			TraceAny(anyOut, "statistics");
		}
		if ( Storage::GetStatisticLevel() >= 1 ) {
			assertComparem(lAllocMark, equal_to, aPoolAlloc.CurrentlyAllocated(), "expected PoolAllocator to have allocated its memory on Storage::Global()");
		}
	}
	{
		PoolAllocator aPoolAlloc(2345, 2345, 18);
		Anything anyMsg, anyMsg2;
		anyMsg["Content"] = "DummyContent1";
		anyMsg2["Content"] = "DummyContent2";

		EventTypePtr pEventWrapper = new EventType( DCDStateMachine::ev_ReloadMarketCodeFilter(anyMsg) );
		EventTypePtr pEventWrapper2 = new EventType( DCDStateMachine::ev_ReloadMarketCodeFilter(anyMsg2) );
		EventTypePtr pEventWrapperOut2 = NULL;

		l_long lAllocMark = aPoolAlloc.CurrentlyAllocated();

		{
			EventQueueType Q1("Q1", 20, &aPoolAlloc);
			assertEqualm(0L, Q1.GetSize(), "expected queue to be empty");
			Q1.Put(pEventWrapper);
			assertEqualm(1L, Q1.GetSize(), "expected queue to contain 1 element");
			Q1.Put(pEventWrapper2);
			assertEqualm(2L, Q1.GetSize(), "expected queue to contain 2 elements");

			if ( Storage::GetStatisticLevel() >= 1 ) {
				assertComparem(lAllocMark, less, aPoolAlloc.CurrentlyAllocated(), "expected PoolAllocator having had some allocations");
			}

			Q1.Get(pEventWrapperOut2);
			assertEqualm(1L, Q1.GetSize(), "expected queue to contain 1 element");
			delete pEventWrapperOut2;

			Q1.Get(pEventWrapperOut2);
			assertEqualm(0L, Q1.GetSize(), "expected queue to be empty");
			delete pEventWrapperOut2;
		}
		if ( Storage::GetStatisticLevel() >= 1 ) {
			assertComparem(lAllocMark, equal_to, aPoolAlloc.CurrentlyAllocated(), "expected PoolAllocator to have allocated its memory on Storage::Global()");
		}
	}
}

// builds up a suite of testcases, add a line for each testmethod
Test *QueueTest::suite ()
{
	StartTrace(QueueTest.suite);
	TestSuite *testSuite = new TestSuite;

	ADD_CASE(testSuite, QueueTest, BlockingSideTest);
	ADD_CASE(testSuite, QueueTest, PutGetStatusTest);
	ADD_CASE(testSuite, QueueTest, SimplePutGetTest);
	ADD_CASE(testSuite, QueueTest, MultiProducerSingleConsumerTest);
	ADD_CASE(testSuite, QueueTest, SingleProducerMultiConsumerTest);
	ADD_CASE(testSuite, QueueTest, ConsumerTerminationTest);
	ADD_CASE(testSuite, QueueTest, SimpleTypeAnyStorageQueueTest);
	ADD_CASE(testSuite, QueueTest, SimpleTypeListStorageQueueTest);
	ADD_CASE(testSuite, QueueTest, QueueWithAllocatorTest);

	return testSuite;
}
