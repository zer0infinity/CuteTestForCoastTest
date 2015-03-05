/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#include "MasterServerTest.h"
#include "TestSuite.h"
#include "FoundationTestTypes.h"
#include "Server.h"
#include "Socket.h"
#include "AnyIterators.h"
#define TESTHOST "localhost"

void MasterServerTest::setUp() {
	StartTrace(MasterServerTest.setUp);
	t_assert(GetConfig().IsDefined("Modules"));
	Server *s;
	if (t_assert((s = Server::FindServer("Server")) != NULL)) {
		ROAnything result;
		t_assert(s->Lookup("TCP5010", result));
		TraceAny(result, "server lookup TCP5010");
	}
}

void MasterServerTest::InitRunTerminateTest() {
	StartTrace(MasterServerTest.InitRunTerminateTest);

	TestCaseType::DoUnloadConfig();
	ROAnything roaConfig;
	AnyExtensions::Iterator<ROAnything, ROAnything, String> aEntryIterator(GetTestCaseConfig());
	while (aEntryIterator.Next(roaConfig)) {
		TestCaseType::DoLoadConfig("MasterServerTest", "InitRunTerminateTest");
		String serverName;
		aEntryIterator.SlotName(serverName);
		Trace("Checks with server <" << serverName << ">");

		Server *server = SafeCast(Server::FindServer(serverName), MasterServer);
		String msg;
		msg << "expected " << serverName << "  to be there";
		if (t_assertm(server != NULL, (const char *)msg)) {
			long numOfThreads = 0;
			TraceAny(roaConfig, "Ports to check");
			serverName << "_of_InitRunTerminateTest";
			server = (Server *) server->ConfiguredClone("Server", serverName, true);
			if (t_assertm(server != NULL, "expected server-clone to succeed")) {
				ServerThread mt(server);
				numOfThreads = Thread::NumOfThreads();
				if (t_assert(mt.Start()) && t_assert(mt.CheckState(Thread::eRunning, 5))) {
					if (t_assertm(mt.serverIsInitialized(), "expected initialization to succeed")) {
						mt.SetWorking();
						if (t_assertm(mt.IsReady(true, 5), "expected server to become ready within 5 seconds")) {
							// --- run various request
							//     sequences
							RunTestSequence(roaConfig);
							mt.PrepareShutdown(0);
						}
					}
					if (t_assertm(mt.IsReady(false, 5), "expected server to become terminated within 5 seconds")) {
						mt.Terminate(10);
					}
				}
				mt.CheckState(Thread::eTerminated, 10);
				server->Finalize();
				delete server;
			}
			assertComparem(numOfThreads, equal_to, Thread::NumOfThreads(), "expected number of threads to match");
		}
		TestCaseType::DoUnloadConfig();
	}
}

void MasterServerTest::InitRunResetRunTerminateTest() {
	StartTrace(MasterServerTest.InitRunResetRunTerminateTest);

	ROAnything roaConfig;
	AnyExtensions::Iterator<ROAnything, ROAnything, String> aEntryIterator(GetTestCaseConfig());
	while (aEntryIterator.Next(roaConfig)) {
		String serverName;
		aEntryIterator.SlotName(serverName);
		Trace("Checks with server <" << serverName << ">");

		Server *server = SafeCast(Server::FindServer(serverName), MasterServer);
		String msg;
		msg << "expected " << serverName << "  to be there";

		if (t_assertm(server != NULL, (const char *)msg)) {
			long numOfThreads = 0;
			TraceAny(roaConfig, "Ports to check");
			serverName << "_of_InitRunResetRunTerminateTest";
			server = (Server *) server->ConfiguredClone("Server", serverName, true);
			if (t_assertm(server != NULL, "expected server-clone to succeed")) {
				ServerThread mt(server);
				numOfThreads = Thread::NumOfThreads();
				if (t_assert(mt.Start()) && t_assert(mt.CheckState(Thread::eRunning, 5))) {
					if (t_assertm(mt.serverIsInitialized(), "expected initialization to succeed")) {
						mt.SetWorking();
						if (t_assertm(mt.IsReady(true, 5), "expected server to become ready within 5 seconds")) {
							// --- run various request
							//     sequences
							RunTestSequence(roaConfig);
							t_assertm(server->GlobalReinit() == 0, "expected server to reinit ok");
							RunTestSequence(roaConfig);
							mt.PrepareShutdown(0);
						}
					}
					if (t_assertm(mt.IsReady(false, 5), "expected server to become terminated within 5 seconds")) {
						mt.Terminate(10);
					}
				}
				t_assertm(mt.CheckState(Thread::eTerminated, 10), "expected server thread to become terminated");
				server->Finalize();
				delete server;
			}
			assertComparem(numOfThreads, equal_to, Thread::NumOfThreads(), TString("expected number of threads to match in loop ").Append(aEntryIterator.Index()));
		}
	}
}

void MasterServerTest::RunTestSequence(ROAnything roaConfig) {
	StartTrace(MasterServerTest.RunTestSequence);
	Anything testMessage;
	Anything replyMessage;
	testMessage = "Hallo there message";

	for (long i = 0; i < roaConfig.GetSize(); i++) {
		long port = roaConfig[i].AsLong();
		Trace("Check ip port : " << port);

		Connector con(TESTHOST, port);
		if (t_assert(con.GetStream() != NULL)) {
			testMessage.PrintOn((*con.GetStream()));
			t_assert(!!(*con.GetStream()));

			replyMessage.Import((*con.GetStream()), "Server reply");
			t_assert(!!(*con.GetStream()));

			assertAnyEqual(testMessage, replyMessage[0L]); // implies LoopbackProcessor
		}
	}
}

Test *MasterServerTest::suite() {
	TestSuite *testSuite = new TestSuite;
	ADD_CASE(testSuite, MasterServerTest, InitRunTerminateTest);
	ADD_CASE(testSuite, MasterServerTest, InitRunResetRunTerminateTest);
	return testSuite;
}
