/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

//--- interface include --------------------------------------------------------
#include "MasterServerTest.h"

//--- test modules used --------------------------------------------------------
#include "TestSuite.h"

//--- module under test --------------------------------------------------------
#include "Server.h"

//--- standard modules used ----------------------------------------------------
#include "Socket.h"
#include "StringStream.h"
#include "Context.h"
#include "Dbg.h"

//--- c-library modules used ---------------------------------------------------

#define TESTHOST "localhost"

//---- MasterServerTest ----------------------------------------------------------------
MasterServerTest::MasterServerTest(TString tname) :
	ConfiguredTestCase(tname, "Config"),
	fCheckPorts()//,
	//fTestConfig()
{
	StartTrace(MasterServerTest.Ctor);

}

MasterServerTest::~MasterServerTest()
{
	StartTrace(MasterServerTest.Dtor);
}

void MasterServerTest::setUp ()
{
	StartTrace(MasterServerTest.setUp);
	ConfiguredTestCase::setUp();
	fTestCaseConfig = Anything(); // nullify here, initialized below for traditional reasons
	t_assert(fConfig.IsDefined("Modules"));
	Application::InitializeGlobalConfig(fConfig);
	WDModule::Install(fConfig);
	Server *s;
	if (t_assert((s = Server::FindServer("Server")) != NULL)) {
		ROAnything result;
		t_assert(s->Lookup("TCP5010", result));
		TraceAny(result, "server lookup TCP5010");
	}

	// MasterServer with 0 SubServers
	Anything &test0 = fTestCaseConfig["TestMasterServer0"];
	test0.Append(fConfig["TCP5020"]["Port"].AsLong());
	test0.Append(fConfig["TCP5021"]["Port"].AsLong());

	// MasterServer with 1 SubServers
	Anything &test1 = fTestCaseConfig["TestMasterServer1"];
	test1.Append(fConfig["TCP5020"]["Port"].AsLong());
	test1.Append(fConfig["TCP5021"]["Port"].AsLong());
	test1.Append(fConfig["TCP5010"]["Port"].AsLong());
	test1.Append(fConfig["TCP5011"]["Port"].AsLong());

	// MasterServer with 2 SubServers
	Anything &test2 = fTestCaseConfig["TestMasterServer2"];
	test2.Append(fConfig["TCP5020"]["Port"].AsLong());
	test2.Append(fConfig["TCP5021"]["Port"].AsLong());
	test2.Append(fConfig["TCP5010"]["Port"].AsLong());
	test2.Append(fConfig["TCP5011"]["Port"].AsLong());
	test2.Append(fConfig["TCP5012"]["Port"].AsLong());
	test2.Append(fConfig["TCP5013"]["Port"].AsLong());

	// MasterServer with many (3) SubServers
	Anything &test3 = fTestCaseConfig["TestMasterServer3"];
	test3.Append(fConfig["TCP5020"]["Port"].AsLong());
	test3.Append(fConfig["TCP5021"]["Port"].AsLong());
	test3.Append(fConfig["TCP5010"]["Port"].AsLong());
	test3.Append(fConfig["TCP5011"]["Port"].AsLong());
	test3.Append(fConfig["TCP5012"]["Port"].AsLong());
	test3.Append(fConfig["TCP5013"]["Port"].AsLong());
	test3.Append(fConfig["TCP5014"]["Port"].AsLong());
	test3.Append(fConfig["TCP5015"]["Port"].AsLong());
}

void MasterServerTest::tearDown ()
{
	StartTrace(MasterServerTest.tearDown);
	WDModule::Terminate(fConfig);
	Application::InitializeGlobalConfig(Anything());

	ConfiguredTestCase::tearDown();
}

void MasterServerTest::InitRunTerminateTest()
{
	StartTrace(MasterServerTest.InitRunTerminateTest);

	for (long i = 0; i < fTestCaseConfig.GetSize(); i++) {
		String serverName(fTestCaseConfig.SlotName(i));
		Trace("Checks with server <" << serverName << ">");

		Server *server = SafeCast(Server::FindServer(serverName), MasterServer);
		String msg;
		msg << "expected " << serverName << "  to be there";
		if ( t_assertm(server != NULL, (const char *)msg) ) {
			fCheckPorts = fTestCaseConfig[i];
			TraceAny(fCheckPorts, "Ports to check");

			long numOfThreads = Thread::NumOfThreads();

			server = (Server *)server->ConfiguredClone("Server", serverName);
			if ( t_assertm(server->Init() == 0, "expected initialization to succeed") ) {
				ServerThread mt(server);
				if (t_assert(mt.Start()) &&
					t_assert(mt.CheckState(Thread::eRunning, 5))) {
					if (t_assertm(server->IsReady(true, 5), "expected server to become ready within 5 seconds")) {
						// --- run various request
						//     sequences
						RunTestSequence();
						server->PrepareShutdown(0);
					}
				}
				if (t_assertm(server->IsReady(false, 5), "expected server to become terminated within 5 seconds")) {
					mt.Terminate(0);
				}
			}
			assertEqualm(numOfThreads, Thread::NumOfThreads(), "expected number of threads to match");
			delete server;
		}
	}
}

void MasterServerTest::InitRunResetRunTerminateTest ()
{
	StartTrace(MasterServerTest.InitRunResetRunTerminateTest);
	for (long j = 0; j < 5; j++) {
		for (long i = 0; i < fTestCaseConfig.GetSize(); i++) {
			String serverName(fTestCaseConfig.SlotName(i));
			Trace("Checks with server <" << serverName << ">");

			Server *server = SafeCast(Server::FindServer(serverName), MasterServer);
			String msg;
			msg << "expected " << serverName << "  to be there";

			if ( t_assertm(server != NULL, (const char *)msg) ) {
				fCheckPorts = fTestCaseConfig[i];
				TraceAny(fCheckPorts, "Ports to check");

				long numOfThreads = Thread::NumOfThreads();

				server = (Server *)server->ConfiguredClone("Server", serverName);
				if ( t_assertm(server->Init() == 0, "expected initialization to succeed") ) {
					ServerThread mt(server);
					if (t_assert(mt.Start()) &&
						t_assert(mt.CheckState(Thread::eRunning, 5))) {
						if (t_assertm(server->IsReady(true, 5), "expected server to become ready within 5 seconds")) {
							// --- run various request
							//     sequences
							RunTestSequence();
							t_assertm(server->GlobalReinit() == 0, "expected server to reinit ok");
							RunTestSequence();
							server->PrepareShutdown(0);
						}
					}
					if (t_assertm(server->IsReady(false, 5), "expected server to become terminated within 5 seconds")) {
						mt.Terminate(0);
					}
				}
				assertEqualm(numOfThreads, Thread::NumOfThreads(), "expected number of threads to match");
				delete server;
			}
		}
	}
}

void MasterServerTest::RunTestSequence()
{
	StartTrace(MasterServerTest.RunTestSequence);
	Anything testMessage;
	Anything replyMessage;
	testMessage = "Hallo there message";

	for (long i = 0; i < fCheckPorts.GetSize(); i++) {
		long port = fCheckPorts[i].AsLong();

		Trace("Check ip port : " << port);

		Connector con(TESTHOST, port);
		if ( t_assert(con.GetStream() != NULL) ) {
			testMessage.PrintOn((*con.GetStream()));
			t_assert(!!(*con.GetStream()));

			replyMessage.Import((*con.GetStream()), "Server reply");
			t_assert(!!(*con.GetStream()));

			assertAnyEqual(testMessage, replyMessage[0L]); // implies LoopbackProcessor
		}
	}
}

Test *MasterServerTest::suite ()
// collect all test cases for the RegistryStream
{
	TestSuite *testSuite = new TestSuite;
	testSuite->addTest (NEW_CASE(MasterServerTest, InitRunTerminateTest));
	testSuite->addTest (NEW_CASE(MasterServerTest, InitRunResetRunTerminateTest));

	return testSuite;

} // suite
