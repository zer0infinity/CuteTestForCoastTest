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
#include "Dbg.h"
#include "System.h"

//--- c-modules used -----------------------------------------------------------

class TestConsumer : public Thread
{
	friend class QueueTest;
public:
	TestConsumer(Queue &aQueue, long lHowManyConsumes, long lWaitTimeMicroSec = 0L)
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
			if ( fQueue.Get(anyProduct, bTryLock) ) {
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
	Queue &fQueue;
	long fWaitTimeMicroSec, fToConsume, fConsumed;
	Anything fWork, fProducts;
};

class TestProducer : public Thread
{
public:
	TestProducer(Queue &aQueue, long lHowManyProduces, long lWaitTimeMicroSec = 0L)
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
			if ( fQueue.Put(anyProduct, bTryLock) ) {
				TraceAny(anyProduct, "produced product");
				if (fWaitTimeMicroSec) {
					System::MicroSleep(fWaitTimeMicroSec);
				}
				lProductCount++;
			}
		}
	}

private:
	Queue &fQueue;
	long fWaitTimeMicroSec, fToProduce, fProduced;
	Anything fWork;
};

class ConsumerTerminationThread : public Thread
{
	friend class QueueTest;
public:
	ConsumerTerminationThread(Queue &aQueue)
		: Thread("ConsumerTerminationThread"), fQueue(aQueue), fConsumed(0L)
	{}
	~ConsumerTerminationThread()
	{}
	void Run() {
		StartTrace(ConsumerTerminationThread.Run);
		// signal start using working state
		Trace("before CheckRunningState(eWorking)");
		while ( IsRunning() ) {
			CheckRunningState(eWorking);
			Trace("now working");
			Anything anyProduct;
			if ( fQueue.Get(anyProduct, false) ) {
				TraceAny(anyProduct, "consumed product");
				fConsumed++;
			}
		}
	}

private:
	Queue &fQueue;
	long fConsumed;
};
//---- QueueTest ----------------------------------------------------------------
QueueTest::QueueTest(TString tstrName) : TestCase(tstrName)
{
	StartTrace(QueueTest.Ctor);
}

QueueTest::~QueueTest()
{
	StartTrace(QueueTest.Dtor);
}

// setup for this TestCase
void QueueTest::setUp ()
{
	StartTrace(QueueTest.setUp);
}

void QueueTest::tearDown ()
{
	StartTrace(QueueTest.tearDown);
}

void QueueTest::SimplePutGetTest()
{
	StartTrace(QueueTest.SimplePutGetTest);
	Queue Q1("Q1", 1);
	t_assert(Q1.GetSize() == 0L);
	Anything anyTest1, anyTest2;
	anyTest1["Guguseli"] = 1;
	// first call should not block
	t_assert(Q1.Put(anyTest1));
	anyTest2["Guguseli"] = 2;
	t_assert(Q1.Put(anyTest2, true) == false);
	t_assert(Q1.GetSize() == 1L);
	Anything anyOut;
	// first get should work without blocking
	t_assert(Q1.Get(anyOut));
	assertAnyEqual(anyTest1, anyOut);
	t_assert(Q1.GetSize() == 0L);
	// second get should fail because of empty queue
	anyOut = Anything();
	t_assert(Q1.Get(anyOut, true) == false);
	Q1.GetStatistics(anyOut);
	assertEqual(1L, anyOut["QueueSize"].AsLong(0L));
	assertEqual(1L, anyOut["MaxLoad"].AsLong(0L));
	TraceAny(anyOut, "statistics");
}

void QueueTest::DoMultiProducerSingleConsumerTest(long lQueueSize)
{
	StartTrace1(QueueTest.DoMultiProducerSingleConsumerTest, "QueueSize:" << lQueueSize);
	{
		Queue aProductQueue("aProductQueue", lQueueSize);
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
		Queue aProductQueue("aProductQueue", lQueueSize);
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
		Queue aProductQueue("aProductQueue", lQueueSize);
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
		Queue aProductQueue("aProductQueue", lQueueSize);
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
	Queue *pProductQueue = new Queue("pProductQueue", 2);
	ConsumerTerminationThread aConsumer(*pProductQueue);
	{
		Anything anyProduct;
		anyProduct["What"] = "Yippieh";
		t_assert(pProductQueue->Put(anyProduct));
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

// builds up a suite of testcases, add a line for each testmethod
Test *QueueTest::suite ()
{
	StartTrace(QueueTest.suite);
	TestSuite *testSuite = new TestSuite;

	ADD_CASE(testSuite, QueueTest, SimplePutGetTest);
	ADD_CASE(testSuite, QueueTest, MultiProducerSingleConsumerTest);
	ADD_CASE(testSuite, QueueTest, SingleProducerMultiConsumerTest);
	ADD_CASE(testSuite, QueueTest, ConsumerTerminationTest);

	return testSuite;
}
