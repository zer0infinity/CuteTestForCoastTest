/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

//--- module under test --------------------------------------------------------
#include "Stresser.h"

//--- interface include --------------------------------------------------------
#include "RemoteStresserTest.h"

//--- standard modules used ----------------------------------------------------
#include "Server.h"
#include "Dbg.h"

//--- test modules used --------------------------------------------------------
#include "TestSuite.h"

//--- c-library modules used ---------------------------------------------------

//---- RemoteStresserTest ----------------------------------------------------------------
RemoteStresserTest::RemoteStresserTest(TString tstrName)
	: StressAppTest(tstrName)
	, fServerRunner(0)
	, fStressServer(0)
{
}

RemoteStresserTest::~RemoteStresserTest()
{
	if (fServerRunner) {
		fServerRunner->PrepareShutdown(0);
		fServerRunner->CheckState(Thread::eTerminated, 10);
		fServerRunner->Terminate(0);
		delete fServerRunner;
		delete fStressServer;
	}
}

void RemoteStresserTest::setUp ()
{
	StressAppTest::setUp();
	if (!fStressServer) {
		fStressServer = new (Storage::Global()) Server("StressServer");
		fStressServer->GetConfig();
		fStressServer->Initialize("Server");

		fServerRunner = new (Storage::Global()) ServerThread(fStressServer);
		if (fStressServer && fServerRunner) {
			bool bSuccess = fServerRunner->Start();
			fServerRunner->CheckState(Thread::eStarted);
			bSuccess = bSuccess && fServerRunner->serverIsInitialized();
			if ( t_assertm(bSuccess, "StressServer init failed") ) {
				fServerRunner->CheckState(Thread::eRunning, 10);
				fServerRunner->SetWorking();
			}
		} else {
			t_assertm(0, "Server start failed");
			delete fServerRunner;
			fServerRunner = 0;
			delete fStressServer;
			fStressServer = 0;
		}
	}
}

void RemoteStresserTest::tearDown()
{
	StressAppTest::tearDown();
}

void RemoteStresserTest::TestRemoteStresser()
{
	Anything result;
	t_assert(fStressServer != NULL);
	if (fStressServer) {
		// Uses ten DummyStressers
		result = Stresser::RunStresser("RemoteStresser1");
	}
	assertEqual(10, result["Results"].GetSize());
	assertEqual(10, result["Total"]["Nr"].AsLong(0));
	assertEqual(500000, result["Total"]["Steps"].AsLong(0));
	assertEqual(1, result["Total"]["Max"].AsLong(0));
	assertEqual(1, result["Total"]["Min"].AsLong(0));
	assertEqual(0, result["Total"]["Error"].AsLong(2));
	CheckSum(result);
	TestMultiRemoteStresser();
}

void RemoteStresserTest::TestMultiRemoteStresser()
{
	StartTrace(RemoteStresserTest.TestMultiRemoteStresser);
	Anything result;
	t_assert(fStressServer != NULL);
	if (fStressServer) {
		// Uses ten DummyStressers
		result = Stresser::RunStresser("MultiRemote");
	}
	TraceAny(result, "Results");
	assertEqual(10, result["Results"].GetSize());
	assertEqual(100, result["Total"]["Nr"].AsLong(0));
	assertEqual(5000000, result["Total"]["Steps"].AsLong(0));
	assertEqual(1, result["Total"]["Max"].AsLong(0));
	assertEqual(1, result["Total"]["Min"].AsLong(0));
	assertEqual(0, result["Total"]["Error"].AsLong(2));
	// Check sum of doubly nested results
	long checkSum(0);
	ROAnything results = result["Results"];
	long sz = results.GetSize();
	for (long i = 0; i < sz; i++) {
		CheckSum(results[i]);
		checkSum += results[i]["Sum"].AsLong(-1);
		Trace("Checksum now :" << checkSum);
	}
	assertEqual(checkSum, result["Total"]["Sum"].AsLong(0));
	TraceAny(result, "Result");
}

Test *RemoteStresserTest::suite ()
{
	TestSuite *testSuite = new TestSuite;
	ADD_CASE(testSuite, RemoteStresserTest, TestRemoteStresser);

	return testSuite;
}
