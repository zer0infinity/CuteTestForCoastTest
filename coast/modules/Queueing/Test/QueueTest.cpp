/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#include "QueueTest.h"
#include "TestSuite.h"
#include "FoundationTestTypes.h"
#include "Queue.h"
#include "PoolAllocator.h"
#include "SystemBase.h"
#include <typeinfo>
#include "DiffTimer.h"
#include "ThreadPools.h"
#include "StringStream.h"
#include "MT_Storage.h"

using namespace Coast;

class DCDStateMachine {
public:
	DCDStateMachine();

	// events
	struct ev_ReloadMarketCodeFilter {
		ev_ReloadMarketCodeFilter(Anything &anyElement) :
			fanyElement(anyElement), froaElement(fanyElement) {
		}
		Anything fanyElement;
		ROAnything froaElement;
	};
};

template<class theDCDStateMachine>
class dcd_event {
	template<typename ValueType>
	dcd_event &operator=(const ValueType &);
	dcd_event &operator=(const dcd_event &);

public:
	dcd_event(const dcd_event &other) :
		content(other.content ? other.content->clone() : 0) {
	}
	dcd_event() :
		content(0) {
	}

	template<typename ValueType>
	dcd_event(const ValueType &value) :
		content(new holder<ValueType> (value)) {
	}

	~dcd_event() {
		delete content;
	}

public:
	// queries

	bool empty() const {
		return !content;
	}

	const std::type_info &type() const {
		return content ? content->type() : typeid(void);
	}

#ifndef BOOST_NO_MEMBER_TEMPLATE_FRIENDS
private:
	// types
#else
public: // types (public so any_cast can be non-friend)
#endif

	class placeholder {
	public:
		// structors

		virtual ~placeholder() {
		}

	public:
		// queries

		virtual bool process(theDCDStateMachine &rStateMachine) = 0;

		virtual const std::type_info &type() const = 0;

		virtual placeholder *clone() const = 0;

	};

	template<typename ValueType>
	class holder: public placeholder {
	public:
		// structors

		holder(const ValueType &value) :
			held(value) {
		}

	public:
		// queries
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

	public:
		// representation

		ValueType held;

	};

	placeholder *content;
};

template<typename QueueType, bool StoreConsumedProducts = true>
class TestConsumer : public Thread
{
	friend class QueueTest;
public:
	typedef QueueType& QueueTypeRef;
	typedef typename QueueType::ElementType QElement;

	TestConsumer(QueueTypeRef aQueue, long lHowManyConsumes, long lWaitTimeMicroSec = 0L)
	: Thread("TestConsumer"), fQueue(aQueue), fWaitTimeMicroSec(lWaitTimeMicroSec), fToConsume(lHowManyConsumes), fConsumed(0L)
	{}
	bool DoStartRequestedHook(ROAnything roaWork)
	{
		fWork = roaWork.DeepClone();
		return true;
	}
	void Run() {
		StartTrace(TestConsumer.Run);
		long lProductCount = 0L;
		bool bTryLock = fWork["TryLock"].AsBool(false);
		while ( lProductCount < fToConsume ) {
			QElement aProduct;
			if ( fQueue.Get(aProduct, bTryLock) == QueueType::eSuccess )
			{
				TraceAny(aProduct,"consumed product");
				if ( StoreConsumedProducts )
					fProducts[lProductCount] = aProduct;
				if (fWaitTimeMicroSec > 0L)
					System::MicroSleep(fWaitTimeMicroSec);
				++lProductCount;
			}
		}
	}

private:
	QueueTypeRef fQueue;
	long fWaitTimeMicroSec, fToConsume, fConsumed;
	Anything fWork, fProducts;
};

template<typename QueueType>
class TestProducer : public Thread
{
public:
	typedef QueueType& QueueTypeRef;
	typedef typename QueueType::ElementType QElement;

	TestProducer(QueueTypeRef aQueue, long lHowManyProduces, long lWaitTimeMicroSec = 0L)
	: Thread("TestProducer"), fQueue(aQueue), fWaitTimeMicroSec(lWaitTimeMicroSec), fToProduce(lHowManyProduces), fProduced(0L)
	{}
	bool DoStartRequestedHook(ROAnything roaWork)
	{
		fWork = roaWork.DeepClone();
		return true;
	}
	void Run() {
		StartTrace(TestProducer.Run);
		long lProductCount = 0L;
		bool bTryLock = fWork["TryLock"].AsBool(false);
		while ( lProductCount < fToProduce ) {
			QElement aProduct = fWork["Product"].DeepClone();
			aProduct["ThreadId"] = GetId();
			aProduct["ProductNumber"] = lProductCount;
			if ( fQueue.Put(aProduct, bTryLock) == QueueType::eSuccess )
			{
				TraceAny(aProduct, "produced product");
				if (fWaitTimeMicroSec)
					System::MicroSleep(fWaitTimeMicroSec);
				++lProductCount;
			}
		}
	}

private:
	QueueTypeRef fQueue;
	long fWaitTimeMicroSec, fToProduce, fProduced;
	Anything fWork;
};

template<typename QueueType>
class ConsumerTerminationThread : public Thread
{
	friend class QueueTest;
public:
	typedef QueueType& QueueTypeRef;
	typedef typename QueueType::ElementType QElement;

	ConsumerTerminationThread(QueueTypeRef aQueue)
	: Thread("ConsumerTerminationThread"), fQueue(aQueue), fConsumed(0L)
	{}
	void Run()
	{
		StartTrace(ConsumerTerminationThread.Run);
		// signal start using working state
		Trace("before CheckRunningState(eWorking)");
		while (CheckState(eRunning, 0, 1)) {
			CheckRunningState(eWorking);
			Trace("now working");
			QElement aProduct;
			if ( fQueue.Get(aProduct, false) == QueueType::eSuccess )
			{
				TraceAny(aProduct,"consumed product");
				++fConsumed;
			}
		}
	}

private:
	QueueTypeRef fQueue;
	long fConsumed;
};

//---- QueueTest ----------------------------------------------------------------
QueueTest::QueueTest(TString tstrName) :
	TestCaseType(tstrName) {
	StartTrace(QueueTest.Ctor)
	;
}

QueueTest::~QueueTest() {
	StartTrace(QueueTest.Dtor)
	;
}

void QueueTest::BlockingSideTest() {
	StartTrace(QueueTest.BlockingSideTest);
	typedef AnyQueueType QueueType;
	QueueType Q1("Q1", 1);
	assertEqualm(QueueType::eNone, Q1.feBlocked, "expected unblocked put and get side");
	t_assertm(!Q1.IsBlocked(), "expected queue not to be blocked");
	t_assertm(!Q1.IsBlocked(QueueType::eBothSides), "expected queue not to be blocked");
	t_assertm(!Q1.IsBlocked(QueueType::ePutSide), "expected put side not to be blocked");
	t_assertm(!Q1.IsBlocked(QueueType::eGetSide), "expected get side not to be blocked");
	Q1.Block(QueueType::ePutSide);
	assertEqual(QueueType::ePutSide, Q1.feBlocked);
	t_assertm(Q1.IsBlocked(QueueType::ePutSide), "expected put side to be blocked");
	t_assertm(!Q1.IsBlocked(QueueType::eGetSide), "expected get side not to be blocked");
	t_assertm(!Q1.IsBlocked(QueueType::eBothSides), "expected not both sides to be blocked");
	Q1.UnBlock(QueueType::eBothSides);
	t_assertm(!Q1.IsBlocked(), "expected queue not to be blocked");
	Q1.Block(QueueType::eGetSide);
	assertEqual(QueueType::eGetSide, Q1.feBlocked);
	t_assertm(Q1.IsBlocked(QueueType::eGetSide), "expected get side to be blocked");
	t_assertm(!Q1.IsBlocked(QueueType::ePutSide), "expected put side not to be blocked");
	t_assertm(!Q1.IsBlocked(QueueType::eBothSides), "expected not both sides to be blocked");
	Q1.Block();
	assertEqual(QueueType::eBothSides, Q1.feBlocked);
	t_assertm(Q1.IsBlocked(), "expected queue to be blocked");
	t_assertm(Q1.IsBlocked(QueueType::eBothSides), "expected both sides to be blocked");
	t_assertm(Q1.IsBlocked(QueueType::ePutSide), "expected put side to be blocked");
	t_assertm(Q1.IsBlocked(QueueType::eGetSide), "expected get side to be blocked");
}

void QueueTest::PutGetStatusTest() {
	StartTrace(QueueTest.PutGetStatusTest);
	typedef AnyQueueType QueueType;
	QueueType Q1("Q1", 1);
	t_assert(Q1.GetSize() == 0L);
	Anything anyTest1, anyTest2;
	anyTest1["Guguseli"] = 1;
	assertEqualm(QueueType::eSuccess, Q1.DoPut(anyTest1), "first put should succeed");
	t_assert(Q1.GetSize() == 1L);
	Anything anyOut;
	assertEqualm(QueueType::eSuccess, Q1.DoGet(anyOut), "first get should succeed");
	assertAnyEqual(anyTest1, anyOut);
	t_assert(Q1.GetSize() == 0L);
	anyOut = Anything();
	assertEqualm((QueueType::eEmpty|QueueType::eError), Q1.DoGet(anyOut), "second get should fail because of empty queue");
}

void QueueTest::SimplePutGetTest() {
	StartTrace(QueueTest.SimplePutGetTest);
	typedef AnyQueueType QueueType;
	QueueType Q1("Q1", 1);
	t_assert(Q1.GetSize() == 0L);
	Anything anyTest1, anyTest2;
	anyTest1["Guguseli"] = 1;
	// first call should not block
	assertEqual(QueueType::eSuccess, Q1.Put(anyTest1));
	anyTest2["Guguseli"] = 2;
	// second call should block so use a trylock and check return code
	assertEqual(QueueType::eFull, Q1.Put(anyTest2, true));
	t_assert(Q1.GetSize() == 1L);
	Anything anyOut;
	// first get should work without blocking
	assertEqual(QueueType::eSuccess, Q1.Get(anyOut));
	assertAnyEqual(anyTest1, anyOut);
	t_assert(Q1.GetSize() == 0L);
	// second get should block because of empty queue, so use trylock and check return code
	anyOut = Anything();
	assertEqual(QueueType::eEmpty, Q1.Get(anyOut, true));
	Q1.GetStatistics(anyOut);
	assertEqual(1L, anyOut["QueueSize"].AsLong(0L));
	assertEqual(1L, anyOut["MaxLoad"].AsLong(0L));
	TraceAny(anyOut, "statistics");
}

void QueueTest::DoMultiProducerSingleConsumerTest(long lQueueSize) {
	StartTrace1(QueueTest.DoMultiProducerSingleConsumerTest, "QueueSize:" << lQueueSize);
	{
		typedef AnyQueueType QueueType;
		QueueType aProductQueue("aProductQueue", lQueueSize);
		TestProducer<QueueType> aProd4(aProductQueue, 4L, 1500L), aProd5(aProductQueue, 5L, 2000L), aProd10(aProductQueue, 10L, 1000L);
		TestConsumer<QueueType> aConsumer(aProductQueue, 19L);
		Anything anyCons, anyProd;
		anyCons["TryLock"] = false;
		anyProd["TryLock"] = false;
		aConsumer.Start(Coast::Storage::Global(), anyCons);
		anyProd["Product"] = "Prod10";
		aProd10.Start(Coast::Storage::Global(), anyProd);
		anyProd["Product"] = "Prod4";
		aProd4.Start(Coast::Storage::Global(), anyProd);
		anyProd["Product"] = "Prod5";
		aProd5.Start(Coast::Storage::Global(), anyProd);
		// wait for 10s on consumer to terminate
		t_assert(aConsumer.CheckState(Thread::eTerminated, 10));
		TraceAny(aConsumer.fProducts, "produced items");
		assertEqual(19L, aConsumer.fProducts.GetSize());
		Anything anyStat;
		aProductQueue.GetStatistics(anyStat);
		TraceAny(anyStat, "statistics");
	}
	{
		typedef AnyQueueType QueueType;
		QueueType aProductQueue("aProductQueue", lQueueSize);
		TestProducer<QueueType> aProd4(aProductQueue, 4L), aProd5(aProductQueue, 5L), aProd10(aProductQueue, 10L);
		TestConsumer<QueueType> aConsumer(aProductQueue, 19L);
		Anything anyCons, anyProd;
		anyCons["TryLock"] = false;
		anyProd["TryLock"] = false;
		aConsumer.Start(Coast::Storage::Global(), anyCons);
		anyProd["Product"] = "Prod10";
		aProd10.Start(Coast::Storage::Global(), anyProd);
		anyProd["Product"] = "Prod4";
		aProd4.Start(Coast::Storage::Global(), anyProd);
		anyProd["Product"] = "Prod5";
		aProd5.Start(Coast::Storage::Global(), anyProd);
		// wait for 10s on consumer to terminate
		t_assert(aConsumer.CheckState(Thread::eTerminated, 10));
		TraceAny(aConsumer.fProducts, "produced items");
		assertEqual(19L, aConsumer.fProducts.GetSize());
		Anything anyStat;
		aProductQueue.GetStatistics(anyStat);
		TraceAny(anyStat, "statistics");
	}
}

void QueueTest::MultiProducerSingleConsumerTest() {
	StartTrace(QueueTest.MultiProducerSingleConsumerTest);
	DoMultiProducerSingleConsumerTest(1L);
	DoMultiProducerSingleConsumerTest(25L);
	DoMultiProducerSingleConsumerTest(5L);
}

void QueueTest::DoSingleProducerMultiConsumerTest(long lQueueSize) {
	StartTrace(QueueTest.DoSingleProducerMultiConsumerTest);
	{
		typedef AnyQueueType QueueType;
		QueueType aProductQueue("aProductQueue", lQueueSize);
		TestConsumer<QueueType> aCons4(aProductQueue, 4L, 1500L), aCons5(aProductQueue, 5L, 2000L), aCons10(aProductQueue, 10L, 1000L);
		TestProducer<QueueType> aProducer(aProductQueue, 19L);
		Anything anyCons, anyProd;
		anyCons["TryLock"] = false;
		anyProd["TryLock"] = false;
		aCons4.Start(Coast::Storage::Global(), anyCons);
		aCons5.Start(Coast::Storage::Global(), anyCons);
		aCons10.Start(Coast::Storage::Global(), anyCons);
		anyProd["Product"] = "Gugus";
		aProducer.Start(Coast::Storage::Global(), anyProd);
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
		typedef AnyQueueType QueueType;
		QueueType aProductQueue("aProductQueue", lQueueSize);
		TestConsumer<QueueType> aCons4(aProductQueue, 4L), aCons5(aProductQueue, 5L), aCons10(aProductQueue, 10L);
		TestProducer<QueueType> aProducer(aProductQueue, 19L);
		Anything anyCons, anyProd;
		anyCons["TryLock"] = false;
		anyProd["TryLock"] = false;
		aCons4.Start(Coast::Storage::Global(), anyCons);
		aCons5.Start(Coast::Storage::Global(), anyCons);
		aCons10.Start(Coast::Storage::Global(), anyCons);
		anyProd["Product"] = "Gugus";
		aProducer.Start(Coast::Storage::Global(), anyProd);
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

void QueueTest::SingleProducerMultiConsumerTest() {
	StartTrace(QueueTest.SingleProducerMultiConsumerTest);
	DoSingleProducerMultiConsumerTest(1L);
	DoSingleProducerMultiConsumerTest(5L);
	DoSingleProducerMultiConsumerTest(25L);
}

void QueueTest::ConsumerTerminationTest() {
	StartTrace(QueueTest.ConsumerTerminationTest);
	{
		typedef AnyQueueType QueueType;
		QueueType *pProductQueue = new (Coast::Storage::Global()) QueueType("pProductQueue", 2);
		ConsumerTerminationThread<QueueType> aConsumer(*pProductQueue);
		{
			Anything anyProduct;
			anyProduct["What"] = "Yippieh";
			assertEqual(QueueType::eSuccess, pProductQueue->Put(anyProduct));
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
		typedef AnyQueueType QueueType;
		QueueType *pProductQueue = new (Coast::Storage::Global()) QueueType("pProductQueue", 2);
		ConsumerTerminationThread<QueueType> aConsumer(*pProductQueue);
		{
			Anything anyProduct;
			anyProduct["What"] = "Yippieh";
			assertEqual(QueueType::eSuccess, pProductQueue->Put(anyProduct));
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
		pProductQueue->Block(QueueType::eGetSide);
		t_assertm(pProductQueue->IsBlocked(QueueType::eGetSide), "expected get side to be blocked");
		t_assert(aConsumer.CheckState(Thread::eTerminated, 10));
		assertEqualm(0L, pProductQueue->fBlockingGetCount, "expected 0 blocking getter");
		delete pProductQueue;
	}
}

/*
 //TODO - future: change DoGet() method, so that it's not restricted only for Anything's (see Queue.h)
 */
typedef Queue<long, Anything> LongAnyQueueType;
void QueueTest::SimpleTypeAnyStorageQueueTest() {
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

typedef dcd_event<DCDStateMachine> EventType;
typedef EventType *EventTypePtr;
#if defined(__GNUG__)  && ( __GNUC__ >= 4 )
typedef STLStorage::fast_pool_allocator_global<EventTypePtr> EvtAllocType;
typedef Queue<EventTypePtr, std::list<EventTypePtr, EvtAllocType> > EventQueueType;
#else
typedef Queue< EventTypePtr, std::list<EventTypePtr > > EventQueueType;
#endif
void QueueTest::SimpleTypeListStorageQueueTest() {
	StartTrace(QueueTest.SimpleTypeListStorageQueueTest);

	Anything anyMsg, anyMsg2;
	anyMsg["Content"] = "DummyContent1";
	anyMsg2["Content"] = "DummyContent2";

	EventTypePtr pEventWrapper = new EventType(DCDStateMachine::ev_ReloadMarketCodeFilter(anyMsg));
	EventTypePtr pEventWrapper2 = new EventType(DCDStateMachine::ev_ReloadMarketCodeFilter(anyMsg2));
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

void QueueTest::QueueWithAllocatorTest() {
	StartTrace(QueueTest.QueueWithAllocatorTest);
	{
		PoolAllocator aPoolAlloc(1234, 1234, 18);

		ul_long lAllocMark = aPoolAlloc.CurrentlyAllocated();
		{
			typedef AnyQueueType QueueType;
			QueueType Q1("Q1", 1, &aPoolAlloc);
			t_assert(Q1.GetSize() == 0L);
			Anything anyTest1, anyTest2;
			anyTest1["Guguseli"] = 1;
			// first call should not block
			assertEqual(QueueType::eSuccess, Q1.Put(anyTest1));
			anyTest2["Guguseli"] = 2;
			// second call should block so use a trylock and check return code
			assertEqual(QueueType::eFull, Q1.Put(anyTest2, true));
			t_assert(Q1.GetSize() == 1L);

			if (Coast::Storage::GetStatisticLevel() >= 1) {
				assertComparem(lAllocMark, less, aPoolAlloc.CurrentlyAllocated(), "expected PoolAllocator having had some allocations");
			}

			Anything anyOut;
			// first get should work without blocking
			assertEqual(QueueType::eSuccess, Q1.Get(anyOut));
			assertAnyEqual(anyTest1, anyOut);
			t_assert(Q1.GetSize() == 0L);
			// second get should block because of empty queue, so use trylock and check return code
			anyOut = Anything();
			assertEqual(QueueType::eEmpty, Q1.Get(anyOut, true));
			Q1.GetStatistics(anyOut);
			assertEqual(1L, anyOut["QueueSize"].AsLong(0L));
			assertEqual(1L, anyOut["MaxLoad"].AsLong(0L));
			TraceAny(anyOut, "statistics");
		}
		if (Coast::Storage::GetStatisticLevel() >= 1) {
			assertComparem(lAllocMark, equal_to, aPoolAlloc.CurrentlyAllocated(), "expected PoolAllocator to have allocated its memory on Coast::Storage::Global()");
		}
	}
	{
		PoolAllocator aPoolAlloc(2345, 2345, 18);
		Anything anyMsg, anyMsg2;
		anyMsg["Content"] = "DummyContent1";
		anyMsg2["Content"] = "DummyContent2";

		EventTypePtr pEventWrapper = new EventType(DCDStateMachine::ev_ReloadMarketCodeFilter(anyMsg));
		EventTypePtr pEventWrapper2 = new EventType(DCDStateMachine::ev_ReloadMarketCodeFilter(anyMsg2));
		EventTypePtr pEventWrapperOut2 = NULL;

		ul_long lAllocMark = aPoolAlloc.CurrentlyAllocated();

		{
			EventQueueType Q1("Q1", 20, &aPoolAlloc);
			assertEqualm(0L, Q1.GetSize(), "expected queue to be empty");
			Q1.Put(pEventWrapper);
			assertEqualm(1L, Q1.GetSize(), "expected queue to contain 1 element");
			Q1.Put(pEventWrapper2);
			assertEqualm(2L, Q1.GetSize(), "expected queue to contain 2 elements");

			if (Coast::Storage::GetStatisticLevel() >= 1) {
				assertComparem(lAllocMark, equal_to, aPoolAlloc.CurrentlyAllocated(), "expected PoolAllocator to not have allocations");
			}

			Q1.Get(pEventWrapperOut2);
			assertEqualm(1L, Q1.GetSize(), "expected queue to contain 1 element");
			delete pEventWrapperOut2;

			Q1.Get(pEventWrapperOut2);
			assertEqualm(0L, Q1.GetSize(), "expected queue to be empty");
			delete pEventWrapperOut2;
		}
		if ( Storage::GetStatisticLevel() >= 1 )
		{
			assertComparem(lAllocMark, equal_to, aPoolAlloc.CurrentlyAllocated(), "expected PoolAllocator to have allocated its memory on Storage::Global()");
		}
	}
}

namespace {
	String const testString100B = String("1234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890", Storage::Global());
}

using namespace Coast::Utility;	// for demangle

template<typename ElementType, typename StorageType, long LoopCount, int aResolution>
void MeasurePutGetForQueueType() {
	StartTrace(QueueTest.MeasurePutGetForQueueType);
	typedef Queue<ElementType, StorageType> myQueueType;
	myQueueType theQueue("MeasureTestQueue");
	String queueTypeString = theQueue.typeName();
	ElementType elementToPut = ElementType(testString100B);
	DiffTimer myTimer(static_cast<DiffTimer::eResolution>(aResolution));
	DiffTimer::tTimeType elapsedPutTime = 0;
	for (long idx=0; idx < LoopCount; ++idx) {
		myTimer.Start();
		theQueue.Put(elementToPut);
		elapsedPutTime += myTimer.Diff();
	}
	ElementType elementToGet;
	DiffTimer::tTimeType elapsedGetTime = 0;
	for (long idx=0; idx < LoopCount; ++idx) {
		myTimer.Start();
		theQueue.Get(elementToGet);
		elapsedGetTime += myTimer.Diff();
	}
	String resolutionSuffix = ( aResolution == DiffTimer::eMilliseconds ? "ms" : ( aResolution == DiffTimer::eMicroseconds ? "us" : ( aResolution == DiffTimer::eNanoseconds ? "ns" : "s" ) ) );

	SystemLog::WriteToStderr(String("\nPut   time for ").Append(LoopCount).Append(" elts: ").Append(static_cast<l_long>(elapsedPutTime)).Append(resolutionSuffix).Append(" avg: ").Append(static_cast<l_long>(elapsedPutTime/LoopCount)).Append(resolutionSuffix).Append(' ').Append(queueTypeString).Append('\n'));
	SystemLog::WriteToStderr(String("Get   time for ").Append(LoopCount).Append(" elts: ").Append(static_cast<l_long>(elapsedGetTime)).Append(resolutionSuffix).Append(" avg: ").Append(static_cast<l_long>(elapsedGetTime/LoopCount)).Append(resolutionSuffix).Append(' ').Append(queueTypeString).Append('\n'));
}

template<typename ElementType, typename StorageType, long LoopCount, int aResolution>
void MeasurePutFlushForQueueType() {
	StartTrace(QueueTest.MeasurePutFlushForQueueType);
	typedef Queue<ElementType, StorageType> myQueueType;
	myQueueType theQueue("MeasureTestQueue");
	String queueTypeString = theQueue.typeName();
	ElementType elementToPut = ElementType(testString100B);
	DiffTimer myTimer(static_cast<DiffTimer::eResolution>(aResolution));
	DiffTimer::tTimeType elapsedPutTime = 0;
	for (long idx=0; idx < LoopCount; ++idx) {
		myTimer.Start();
		theQueue.Put(elementToPut);
		elapsedPutTime += myTimer.Diff();
	}
	StorageType flushedElements;
	DiffTimer::tTimeType elapsedFlushTime = 0;
	{
		myTimer.Start();
		theQueue.EmptyQueue(flushedElements);
		elapsedFlushTime += myTimer.Diff();
	}
	String resolutionSuffix = ( aResolution == DiffTimer::eMilliseconds ? "ms" : ( aResolution == DiffTimer::eMicroseconds ? "us" : ( aResolution == DiffTimer::eNanoseconds ? "ns" : "s" ) ) );
	SystemLog::WriteToStderr(String("\nPut   time for ").Append(LoopCount).Append(" elts: ").Append(static_cast<l_long>(elapsedPutTime)).Append(resolutionSuffix).Append(" avg: ").Append(static_cast<l_long>(elapsedPutTime/LoopCount)).Append(resolutionSuffix).Append(' ').Append(queueTypeString).Append('\n'));
	SystemLog::WriteToStderr(String("Flush time for ").Append(LoopCount).Append(" elts: ").Append(static_cast<l_long>(elapsedFlushTime)).Append(resolutionSuffix).Append(" avg: ").Append(static_cast<l_long>(elapsedFlushTime/LoopCount)).Append(resolutionSuffix).Append(' ').Append(queueTypeString).Append('\n'));
}

#include <deque>

void QueueTest::QueueTypePerfTest() {
	StartTrace(QueueTest.QueueTypePerfTest);
	DiffTimer::eResolution const testResolution=DiffTimer::eMicroseconds;
	MeasurePutGetForQueueType<Anything, Anything, 100, testResolution>();
	MeasurePutGetForQueueType<Anything, Anything, 1000, testResolution>();
	MeasurePutGetForQueueType<Anything, Anything, 10000, testResolution>();
	{
		typedef Anything eltType;
		typedef STLStorage::fast_pool_allocator_global< eltType > QAllocType;
		typedef std::deque<eltType, QAllocType > myStorageType;
		MeasurePutGetForQueueType<eltType, myStorageType, 100, testResolution>();
		MeasurePutGetForQueueType<eltType, myStorageType, 1000, testResolution>();
		MeasurePutGetForQueueType<eltType, myStorageType, 10000, testResolution>();
	}
}

template<typename QueueType>
class PoolConsumer : public Thread
{
	friend class QueueTest;
public:
	typedef QueueType* QueueTypePtr;
	typedef typename QueueType::ElementType QElement;

	PoolConsumer(const char* name, ROAnything roaArgs)
	: Thread(name), fConsumed(0L), fWork(Storage::Global())
	{}
	bool DoStartRequestedHook(ROAnything roaArgs) {
		StartTrace(PoolConsumer.DoStartRequestedHook);
		TraceAny(roaArgs, "arguments");
		fWork = roaArgs.DeepClone();
		return true;
	}
	void Run()
	{
		StartTrace(PoolConsumer.Run);
		TraceAny(fWork, "work arguments");
		bool bTryLock = fWork["TryLock"].AsBool(false);
		bool bContinue = true;
		QueueTypePtr fQueue = SafeCast(fWork["Queue"].AsIFAObject(), QueueType);
		if ( !fQueue ) return;
		Trace("entering while loop");
		while ( CheckState( eRunning, 0, 1 ) && bContinue )
		{
			QElement aProduct;
			typename QueueType::StatusCode eRet= fQueue->Get(aProduct, bTryLock);
			switch ( eRet )
			{
				case QueueType::eSuccess:
					SetWorking();
					TraceAny(aProduct,"consumed product id:" << MyId());
					++fConsumed;
					SetReady();
				break;
				case QueueType::eBlocked:
					bContinue = false;
				break;
				default:
				break;
			};
		}
	}

private:
	long fConsumed;
	Anything fWork;
};

template<typename ThreadType>
class ConsumerThreadPool : public ThreadPoolManager {
	typedef ThreadPoolManager BaseClass;
public:
	ConsumerThreadPool() : BaseClass("ConsumerThreadPool")
	{}
	void CleanupPool() {
		Join(20);
		DoDeletePool();
	}

protected:
	Thread *DoAllocThread(long i, ROAnything args) {
		StartTrace(ConsumerThreadPool.DoAllocThread);
		Anything anyArgs = args.DeepClone();
		anyArgs["ThreadIdx"] = i;
		TraceAny(anyArgs, "Init-Arguments");
		return new (Coast::Storage::Global()) ThreadType(demangle<String>(typeid(ThreadType).name()), anyArgs);
	}

	ROAnything DoGetInitConfig(long i, ROAnything args) {
		StartTrace(ConsumerThreadPool.DoGetInitConfig);
		// use same config for all threads
		return args;
	}

	ROAnything DoGetStartConfig(long i, ROAnything args) {
		StartTrace(ConsumerThreadPool.DoGetStartConfig);
		// use same config for all threads
		return args;
	}
};

template<typename QueueType, long LoopCount, long NumThreads, int aResolution>
void ExecuteSingleProducerMultiConsumerQTypeTest() {
	StartTrace(QueueTest.ExecuteSingleProducerMultiConsumerQTypeTest);

	typedef PoolConsumer<QueueType> PoolConsumerThread;

	long const lQueueSize = 3000000L;
	Allocator *poolAlloc = MT_Storage::MakePoolAllocator(LoopCount, 26);
	MT_Storage::RefAllocator(poolAlloc);
	{
		QueueType aProductQueue("aProductQueue", lQueueSize, poolAlloc);

		Anything anyArgs;
		anyArgs["Queue"] = dynamic_cast<IFAObject*>(&aProductQueue);
		anyArgs["TryLock"] = false;
		Anything anyProd;
		anyProd["TryLock"] = false;
		anyProd["Product"] = testString100B;

		TraceAny(anyArgs, "" << NumThreads << " parallel workers and their config:");

		TestProducer<QueueType> aProducer(aProductQueue, LoopCount);
		aProducer.Start(MT_Storage::MakePoolAllocator(10, 26), anyProd);
		while ( aProductQueue.GetSize() < (LoopCount/(NumThreads+1)*NumThreads) ) {
			Thread::Wait(0,1000000);
		}

		ConsumerThreadPool<PoolConsumerThread> fThreadPool;
		if ( fThreadPool.Init(NumThreads, anyArgs) ) {
			if ( fThreadPool.Start(true, 10, 26, anyArgs) < 0 ) {
				SYSWARNING("not all thread could be started!");
				return;
			}
		}

		aProducer.CheckState(Thread::eTerminated, 1000);
		while ( aProductQueue.GetSize() > 0 ) {
			Thread::Wait(0,10000000);
		}

		{
			Anything anyStat;
			aProductQueue.GetStatistics(anyStat);
			StringStream aStream;
			aStream << "Statistics for " << aProductQueue.typeName() << "\n";
			anyStat.PrintOn(aStream, true) << "\n" << std::flush;
			SystemLog::WriteToStderr(aStream.str());
		}

		// force termination of thread through queue blocking
		aProductQueue.Block(QueueType::eGetSide);

		// wait on terminated threads and cleanup pool
		long lNumberOfWaits = 10, lPendingThreads=0;
		while ( ( lPendingThreads = fThreadPool.Terminate(1, 10) ) > 0 && (--lNumberOfWaits >= 0) ) {
			SYSWARNING("still " << lPendingThreads << " pending threads, waiting again");
		}
	}
	MT_Storage::UnrefAllocator(poolAlloc);
}

void QueueTest::SingleProducerMultiConsumerQTypeTest()
{
	StartTrace(QueueTest.SingleProducerMultiConsumerQTypeTest);
	DiffTimer::eResolution const testResolution=DiffTimer::eMicroseconds;
	ExecuteSingleProducerMultiConsumerQTypeTest<AnyQueueType, 10000, 8, testResolution>();
	{
		typedef Anything eltType;
		typedef STLStorage::fast_pool_allocator_global< eltType > QAllocType;
		typedef std::deque<eltType, QAllocType > myStorageType;
		typedef Queue<eltType, myStorageType> myQType;
		ExecuteSingleProducerMultiConsumerQTypeTest<myQType, 10000, 8, testResolution>();
	}
}

// builds up a suite of testcases, add a line for each testmethod
Test *QueueTest::suite() {
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
	ADD_CASE(testSuite, QueueTest, QueueTypePerfTest);
	ADD_CASE(testSuite, QueueTest, SingleProducerMultiConsumerQTypeTest);

	return testSuite;
}
