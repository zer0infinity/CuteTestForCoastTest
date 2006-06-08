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
#include "Context.h"
#include "AnyIterators.h"

//--- c-library modules used ---------------------------------------------------

#define TESTHOST "localhost"

//---- MasterServerTest ----------------------------------------------------------------
MasterServerTest::MasterServerTest(TString tname)
	: TestCaseType(tname)
{
	StartTrace(MasterServerTest.MasterServerTest);
}

MasterServerTest::~MasterServerTest()
{
	StartTrace(MasterServerTest.Dtor);
}

void MasterServerTest::setUp ()
{
	StartTrace(MasterServerTest.setUp);
	t_assert(GetConfig().IsDefined("Modules"));
	Server *s;
	if (t_assert((s = Server::FindServer("Server")) != NULL)) {
		ROAnything result;
		t_assert(s->Lookup("TCP5010", result));
		TraceAny(result, "server lookup TCP5010");
	}
}

void MasterServerTest::InitRunTerminateTest()
{
	StartTrace(MasterServerTest.InitRunTerminateTest);

	ROAnything roaConfig;
	AnyExtensions::Iterator<ROAnything, ROAnything, String> aEntryIterator(GetTestCaseConfig());
	while ( aEntryIterator.Next(roaConfig) ) {
		String serverName;
		aEntryIterator.SlotName(serverName);
		Trace("Checks with server <" << serverName << ">");

		Server *server = SafeCast(Server::FindServer(serverName), MasterServer);
		String msg;
		msg << "expected " << serverName << "  to be there";
		if ( t_assertm(server != NULL, (const char *)msg) ) {
			TraceAny(roaConfig, "Ports to check");

			long numOfThreads = Thread::NumOfThreads();

			server = (Server *)server->ConfiguredClone("Server", serverName);
			if ( t_assertm(server->Init() == 0, "expected initialization to succeed") ) {
				ServerThread mt(server);
				if ( t_assert( mt.Start() ) && t_assert( mt.CheckState(Thread::eRunning, 5) ) ) {
					if (t_assertm(server->IsReady(true, 5), "expected server to become ready within 5 seconds")) {
						// --- run various request
						//     sequences
						RunTestSequence(roaConfig);
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
		ROAnything roaConfig;
		AnyExtensions::Iterator<ROAnything, ROAnything, String> aEntryIterator(GetTestCaseConfig());
		while ( aEntryIterator.Next(roaConfig) ) {
			String serverName;
			aEntryIterator.SlotName(serverName);
			Trace("Checks with server <" << serverName << ">");

			Server *server = SafeCast(Server::FindServer(serverName), MasterServer);
			String msg;
			msg << "expected " << serverName << "  to be there";

			if ( t_assertm(server != NULL, (const char *)msg) ) {
				TraceAny(roaConfig, "Ports to check");
				long numOfThreads = Thread::NumOfThreads();

				server = (Server *)server->ConfiguredClone("Server", serverName);
				if ( t_assertm(server->Init() == 0, "expected initialization to succeed") ) {
					ServerThread mt(server);
					if ( t_assert( mt.Start() ) && t_assert( mt.CheckState(Thread::eRunning, 5) ) ) {
						if (t_assertm(server->IsReady(true, 5), "expected server to become ready within 5 seconds")) {
							// --- run various request
							//     sequences
							RunTestSequence(roaConfig);
							t_assertm(server->GlobalReinit() == 0, "expected server to reinit ok");
							RunTestSequence(roaConfig);
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

void MasterServerTest::RunTestSequence(ROAnything roaConfig)
{
	StartTrace(MasterServerTest.RunTestSequence);
	Anything testMessage;
	Anything replyMessage;
	testMessage = "Hallo there message";

	for (long i = 0; i < roaConfig.GetSize(); i++) {
		long port = roaConfig[i].AsLong();
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
{
	TestSuite *testSuite = new TestSuite;
	ADD_CASE(testSuite, MasterServerTest, InitRunTerminateTest);
	ADD_CASE(testSuite, MasterServerTest, InitRunResetRunTerminateTest);
	return testSuite;
}
