/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

//--- interface include --------------------------------------------------------
#include "QueueWorkingModuleTest.h"

//--- module under test --------------------------------------------------------
#include "QueueWorkingModule.h"

//--- test modules used --------------------------------------------------------
#include "TestSuite.h"

//--- project modules used ----------------------------------------------------
#include "Queue.h"

//--- standard modules used ----------------------------------------------------
#include "System.h"
#include "Dbg.h"

//--- c-modules used -----------------------------------------------------------

//---- QueueWorkingModuleTest ----------------------------------------------------------------
QueueWorkingModuleTest::QueueWorkingModuleTest(TString name)
	: ConfiguredTestCase(name, "QueueWorkingModuleTestConfig")
{
	StartTrace(QueueWorkingModuleTest.Ctor);
}

QueueWorkingModuleTest::~QueueWorkingModuleTest()
{
	StartTrace(QueueWorkingModuleTest.Dtor);
}

// setup for this ConfiguredTestCase
void QueueWorkingModuleTest::setUp ()
{
	StartTrace(QueueWorkingModuleTest.setUp);
	ConfiguredTestCase::setUp();
	TraceAny(fTestCaseConfig, "TestCaseConfig");
}

void QueueWorkingModuleTest::tearDown ()
{
	StartTrace(QueueWorkingModuleTest.tearDown);
	ConfiguredTestCase::tearDown();
}

void QueueWorkingModuleTest::InitFinisNoModuleConfigTest()
{
	StartTrace(QueueWorkingModuleTest.InitFinisNoModuleConfigTest);
	QueueWorkingModule aModule("QueueWorkingModule");
	t_assertm( !aModule.Init(fTestCaseConfig["ModuleConfig"]), "module init should have failed due to missing configuration" );
}

void QueueWorkingModuleTest::InitFinisDefaultsTest()
{
	StartTrace(QueueWorkingModuleTest.InitFinisDefaultsTest);
	QueueWorkingModule aModule("QueueWorkingModule");
	if ( t_assertm( aModule.Init(fTestCaseConfig["ModuleConfig"]), "module init should have succeeded" ) ) {
		if ( t_assert( aModule.fpContext != NULL ) ) {
			assertAnyEqual(fTestCaseConfig["ModuleConfig"]["QueueWorkingModule"], aModule.fpContext->GetEnvStore());
		}
		if ( t_assert( aModule.fpQueue != NULL ) ) {
			assertEqualm(0L, aModule.fpQueue->GetSize(), "expected queue to be empty");
			Anything anyStatistics;
			aModule.fpQueue->GetStatistics(anyStatistics);
			assertEqualm(100L, anyStatistics["QueueSize"].AsLong(-1L), "expected default queue size to be 100");
		}
		// terminate module and perform some checks
		t_assert( aModule.Finis() );
		t_assert( aModule.fpContext == NULL );
		t_assert( aModule.fpQueue == NULL );
	}
}

void QueueWorkingModuleTest::InitFinisTest()
{
	StartTrace(QueueWorkingModuleTest.InitFinisTest);
	QueueWorkingModule aModule("QueueWorkingModule");
	if ( t_assertm( aModule.Init(fTestCaseConfig["ModuleConfig"]), "module init should have succeeded" ) ) {
		if ( t_assert( aModule.fpContext != NULL ) ) {
			// check for invalid Server
			t_assertm( NULL == aModule.fpContext->GetServer(), "server must be NULL because of non-existing server");
			assertAnyEqual(fTestCaseConfig["ModuleConfig"]["QueueWorkingModule"], aModule.fpContext->GetEnvStore());
		}
		if ( t_assert( aModule.fpQueue != NULL ) ) {
			assertEqualm(0L, aModule.fpQueue->GetSize(), "expected queue to be empty");
			Anything anyStatistics;
			aModule.fpQueue->GetStatistics(anyStatistics);
			assertEqualm(25L, anyStatistics["QueueSize"].AsLong(-1L), "expected queue size to be 25 as configured");
		}

		// terminate module and perform some checks
		t_assert( aModule.Finis() );
		t_assert( aModule.fpContext == NULL );
		t_assert( aModule.fpQueue == NULL );
	}
}

void QueueWorkingModuleTest::GetAndPutbackTest()
{
	StartTrace(QueueWorkingModuleTest.GetAndPutbackTest);
	QueueWorkingModule aModule("QueueWorkingModule");
	// set modules fAlive-field to enable working of the functions
	// first check if they don't work
	{
		Anything anyMsg;
		// must fail
		t_assert( !aModule.PutElement(anyMsg, false) );
		// fails because of uninitialized queue
		t_assert( !aModule.GetElement(anyMsg) );
	}
	aModule.IntInitQueue(fTestCaseConfig["ModuleConfig"]["QueueWorkingModule"]);
	{
		Anything anyMsg;
		// must still fail because of dead-state
		t_assert( !aModule.GetElement(anyMsg) );
	}
	aModule.fAlive = 0xf007f007;
	if ( t_assertm( aModule.fpQueue != NULL , "queue should be created" ) ) {
		// queue size is 1, so we load it with 1 element
		{
			Anything anyMsg;
			anyMsg["Number"] = 1;
			// this one must succeed
			t_assert( aModule.PutElement(anyMsg, false) );
		}
		// now putback one message
		{
			Anything anyMsg;
			anyMsg["Number"] = 2;
			// this one must fail
			if ( t_assert( !aModule.PutElement(anyMsg, true) ) ) {
				aModule.PutBackElement(anyMsg);
				assertEqualm(1, aModule.fFailedPutbackMessages.GetSize(), "expected overflow buffer to contain an element");
			}
		}
		Anything anyElement;
		if ( t_assert( aModule.GetElement(anyElement) ) ) {
			assertEqualm( 2, anyElement["Number"].AsLong(-1L), "expected to get putback element first");
		}
		assertEqualm( 0, aModule.fFailedPutbackMessages.GetSize(), "expected overflow buffer to be empty now");
		if ( t_assert( aModule.GetElement(anyElement) ) ) {
			assertEqualm( 1, anyElement["Number"].AsLong(-1L), "expected to get regular queue element last");
		}
		assertEqualm( 0, aModule.fpQueue->GetSize(), "expected queue to be empty now");
		aModule.IntCleanupQueue();
	}
}

// builds up a suite of ConfiguredTestCases, add a line for each testmethod
Test *QueueWorkingModuleTest::suite ()
{
	StartTrace(QueueWorkingModuleTest.suite);
	TestSuite *testSuite = new TestSuite;

	ADD_CASE(testSuite, QueueWorkingModuleTest, InitFinisNoModuleConfigTest);
	ADD_CASE(testSuite, QueueWorkingModuleTest, InitFinisDefaultsTest);
	ADD_CASE(testSuite, QueueWorkingModuleTest, InitFinisTest);
	ADD_CASE(testSuite, QueueWorkingModuleTest, GetAndPutbackTest);

	return testSuite;
}
