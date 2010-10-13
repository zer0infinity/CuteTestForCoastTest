/*
 * Copyright (c) 2009, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

//--- interface include --------------------------------------------------------
#include "MultiThreadedTest.h"

//--- module under test --------------------------------------------------------

//--- test modules used --------------------------------------------------------
#include "TestSuite.h"

//--- standard modules used ----------------------------------------------------
#include "AnyIterators.h"
#include "AnyUtils.h"
#include "Dbg.h"
#include "SystemLog.h"
#include "Threads.h"
#include "DataAccess.h"
#include "DiffTimer.h"
#include "Timers.h"
#include "System.h"

//--- c-modules used -----------------------------------------------------------

//---- MultiThreadedTest ----------------------------------------------------------------
MultiThreadedTest::MultiThreadedTest( TString tstrName ) :
	TestCaseType( tstrName )
{
	StartTrace(MultiThreadedTest.MultiThreadedTest);
}

MultiThreadedTest::~MultiThreadedTest()
{
	StartTrace(MultiThreadedTest.Dtor);
}

void MultiThreadedTest::setUp()
{
	StartTrace(MultiThreadedTest.setUp);
}

void MultiThreadedTest::tearDown()
{
	StartTrace(MultiThreadedTest.tearDown);
}

class DATestThread: public Thread
{
	MultiThreadedTest &fMaster;
	long fId, fLoopCount, fLoopWait;
	const char *fGoodDAName, *fFailDAName;
	ROAnything froaExpected;

public:
	DATestThread( MultiThreadedTest &master, long id, const char *goodDAName, const char *failDAName,
				  long lLoops, long lWait, ROAnything roaExpected ) :
		Thread( "DATestThread" ), fMaster( master ), fId( id ), fLoopCount( lLoops ), fLoopWait( lWait ), fGoodDAName( goodDAName ), fFailDAName( failDAName ),
		froaExpected( roaExpected ) {
	}

protected:
	virtual void Run() {
		fMaster.Run( fId, fGoodDAName, fFailDAName, fLoopCount, fLoopWait, froaExpected );
	}
};

void MultiThreadedTest::Run( long id, const char *goodDAName, const char *failDAName, long lLoops, long lWait, ROAnything roaExpected )
{
	for ( int i = 0; i < lLoops; i++ ) {
		Anything aEnv( GetConfig().DeepClone(Storage::Current()) );
		Context ctx( aEnv );

		DataAccess da( goodDAName );
		t_assert(da.StdExec(ctx));
		RequestTimeLogger(ctx);

		Anything tmpStore = ctx.GetTmpStore();

		String strTestPath( goodDAName );
		strTestPath << "." << toString();
		assertAnyCompareEqual(roaExpected, tmpStore, strTestPath, '.', ':');

		DataAccess da2( failDAName );
		t_assertm( !da2.StdExec(ctx), TString(failDAName) << " expected test to fail because of SQL syntax error");
		RequestTimeLogger(ctx);
		if ( lWait > 0 && i != lLoops - 1 ) {
			Thread::Wait(lWait);
		}
	}
}

void MultiThreadedTest::DAImplTest()
{
	StartTrace(MultiThreadedTest.DAImplTest);
	TraceAny(GetTestCaseConfig(), "case config");
	AnyExtensions::Iterator<ROAnything> tcIterator( GetTestCaseConfig() );
	ROAnything roaEntry;
	while ( tcIterator.Next( roaEntry ) ) {
		DiffTimer aTimer;
		String strSuccDA( roaEntry["SuccessDA"].AsString( "MultiThreadedDA" ) ), strFailDA(
			roaEntry["FailureDA"].AsString( "MultiThreadedDAWithError" ) );
		DoTest( roaEntry, strSuccDA, strFailDA );
		SystemLog::WriteToStderr( String( "elapsed time for [" ) << strSuccDA << '&' << strFailDA << "] "
								  << (long) aTimer.Diff() << "ms\n" );
	}
}

void MultiThreadedTest::DoTest( ROAnything roaTestConfig, const char *goodDAName, const char *failDAName )
{
	StartTrace(MultiThreadedTest.DoTest);

	long nThreads = roaTestConfig["ThreadPoolSize"].AsLong( 10L );
	u_long lPoolSize = (u_long) roaTestConfig["PoolStorageSize"].AsLong( 1000L );
	u_long lPoolBuckets = (u_long) roaTestConfig["NumOfPoolBucketSizes"].AsLong( 20L );

	DATestThread **threadArray = new DATestThread*[nThreads];
	long i = 0;
	for ( i = 0; i < nThreads; i++ ) {
		threadArray[i] = new (Storage::Global()) DATestThread( *this, i, goodDAName, failDAName, roaTestConfig["ThreadDALoops"].AsLong(10L), roaTestConfig["ThreadLoopWait"].AsLong(0L), roaTestConfig["Result"] );
		threadArray[i]->Start( MT_Storage::MakePoolAllocator( lPoolSize, lPoolBuckets, i ) );
	}
	for ( i = 0; i < nThreads; i++ ) {
		threadArray[i]->CheckState( Thread::eTerminated );
	}
	for ( i = 0; i < nThreads; i++ ) {
		delete threadArray[i];
	}
	delete[] threadArray;
}

// builds up a suite of tests, add a line for each testmethod
Test *MultiThreadedTest::suite()
{
	StartTrace(MultiThreadedTest.suite);
	TestSuite *testSuite = new TestSuite;
	ADD_CASE(testSuite, MultiThreadedTest, DAImplTest);
	return testSuite;
}
