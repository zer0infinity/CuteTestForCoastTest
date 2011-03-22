/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#include "TestSuite.h"
#include "Server.h"
#include "ServerTest.h"
#include "Socket.h"

#define TESTHOST "localhost"

//---- ServerTest ----------------------------------------------------------------
ServerTest::ServerTest(TString tname)
	: TestCaseType(tname)
{
	StartTrace(ServerTest.ServerTest);
}

ServerTest::~ServerTest()
{
	StartTrace(ServerTest.Dtor);
}

void ServerTest::setUp ()
{
	StartTrace(ServerTest.setUp);
	t_assert(GetConfig().IsDefined("Modules"));
	Server *s;
	if (t_assert((s = Server::FindServer("Server")) != NULL)) {
		ROAnything result;
		t_assert(s->Lookup("TCP5010", result));
		TraceAny(result, "server lookup TCP5010");
	}
}

void ServerTest::InitRunTerminateAcceptorTest()
{
	StartTrace(ServerTest.InitRunTerminateAcceptorTest);
	TestCaseType::DoUnloadConfig();
	for (long i = 0; i < 3; i++) {
		TestCaseType::DoLoadConfig("ServerTest", "InitRunTerminateAcceptorTest");
		Server *server = Server::FindServer("AcceptorWorkerServer");
		if ( t_assertm(server != NULL, "expected AcceptorsWorkersServer to be there") ) {
			server = (Server *)server->ConfiguredClone("Server", "AcceptorWorkerServerMaster", true);
			if ( t_assertm(server != NULL, "expected server-clone to succeed") ) {
				ServerThread mt(server);
				if ( t_assert(mt.Start()) && t_assert(mt.CheckState(Thread::eRunning, 5)) ) {
					if ( t_assertm(mt.serverIsInitialized(), "expected initialization to succeed") ) {
						mt.SetWorking();
						if (t_assertm(mt.IsReady(true, 5), "expected server to become ready within 5 seconds")) {
							// --- run various request
							//     sequences
							RunTestSequence();
							mt.PrepareShutdown(0);
						}
					}
					if (t_assertm(mt.IsReady(false, 5), "expected server to become terminated within 5 seconds")) {
						mt.Terminate(10);
					}
				}
				mt.CheckState(Thread::eTerminated, 5);
				server->Finalize();
				delete server;
			}
		}
		TestCaseType::DoUnloadConfig();
	}
}

void ServerTest::InitRunTerminateLeaderFollowerTest()
{
	StartTrace(ServerTest.InitRunTerminateLeaderFollowerTest);
	TestCaseType::DoUnloadConfig();
	for (long i = 0; i < 3; i++) {
		TestCaseType::DoLoadConfig("ServerTest", "InitRunTerminateLeaderFollowerTest");
		Server *server = Server::FindServer("LeaderFollowerServer");
		if ( t_assertm(server != NULL, "expected LeaderFollowerServer to be there") ) {
			server = (Server *)server->ConfiguredClone("Server", "LeaderFollowerServerMaster", true);
			if ( t_assertm(server != NULL, "expected server-clone to succeed") ) {
				ServerThread mt(server);
				if ( t_assert(mt.Start()) && t_assert(mt.CheckState(Thread::eRunning, 5)) ) {
					if ( t_assertm(mt.serverIsInitialized(), "expected initialization to succeed") ) {
						mt.SetWorking();
						if (t_assertm(mt.IsReady(true, 5), "expected server to become ready within 5 seconds")) {
							// --- run various request
							//     sequences
							RunTestSequence();
							mt.PrepareShutdown(0);
						}
					}
					if (t_assertm(mt.IsReady(false, 5), "expected server to become terminated within 5 seconds")) {
						mt.Terminate(10);
					}
				}
				mt.CheckState(Thread::eTerminated, 5);
				server->Finalize();
				delete server;
			}
		}
		TestCaseType::DoUnloadConfig();
	}
}

void ServerTest::InitRunResetRunTerminateAcceptorTest()
{
	StartTrace(ServerTest.InitRunResetRunTerminateAcceptorTest);
	TestCaseType::DoUnloadConfig();
	for (long i = 0; i < 3; i++) {
		TestCaseType::DoLoadConfig("ServerTest", "InitRunResetRunTerminateAcceptorTest");
		Server *server = Server::FindServer("AcceptorWorkerServer");
		if ( t_assertm(server != NULL, "expected AcceptorsWorkersServer to be there") ) {
			server = (Server *)server->ConfiguredClone("Server", "AcceptorWorkerServerMaster", true);
			if ( t_assertm(server != NULL, "expected server-clone to succeed") ) {
				ServerThread mt(server);
				if ( t_assert(mt.Start()) && t_assert(mt.CheckState(Thread::eRunning, 5)) ) {
					if ( t_assertm(mt.serverIsInitialized(), "expected initialization to succeed") ) {
						mt.SetWorking();
						if (t_assertm(mt.IsReady(true, 5), "expected server to become ready within 5 seconds")) {
							// --- run various request
							//     sequences
							RunTestSequence();
							if ( t_assertm(server->GlobalReinit() == 0, "expected server to reinit ok") ) {
								RunTestSequence();
							}
							mt.PrepareShutdown(0);
						}
					}
					if (t_assertm(mt.IsReady(false, 5), "expected server to become terminated within 5 seconds")) {
						mt.Terminate(10);
					}
				}
				mt.CheckState(Thread::eTerminated, 5);
				server->Finalize();
				delete server;
			}
		}
		TestCaseType::DoUnloadConfig();
	}
}

void ServerTest::InitRunResetRunTerminateLeaderFollowerTest()
{
	StartTrace(ServerTest.InitRunResetRunTerminateLeaderFollowerTest);
	TestCaseType::DoUnloadConfig();
	for (long i = 0; i < 3; i++) {
		TestCaseType::DoLoadConfig("ServerTest", "InitRunResetRunTerminateLeaderFollowerTest");
		Server *server = Server::FindServer("LeaderFollowerServer");
		if ( t_assertm(server != NULL, "expected LeaderFollowerServer to be there") ) {
			server = (Server *)server->ConfiguredClone("Server", "LeaderFollowerServerMaster", true);
			if ( t_assertm(server != NULL, "expected server-clone to succeed") ) {
				ServerThread mt(server);
				if ( t_assert(mt.Start()) && t_assert(mt.CheckState(Thread::eRunning, 5)) ) {
					if ( t_assertm(mt.serverIsInitialized(), "expected initialization to succeed") ) {
						mt.SetWorking();
						if (t_assertm(mt.IsReady(true, 5), "expected server to become ready within 5 seconds")) {
							// --- run various request
							//     sequences
							RunTestSequence();
							if ( t_assertm(server->GlobalReinit() == 0, "expected server to reinit ok") ) {
								RunTestSequence();
							}
							mt.PrepareShutdown(0);
						}
					}
					if (t_assertm(mt.IsReady(false, 5), "expected server to become terminated within 5 seconds")) {
						mt.Terminate(10);
					}
				}
				mt.CheckState(Thread::eTerminated, 5);
				server->Finalize();
				delete server;
			}
		}
		TestCaseType::DoUnloadConfig();
	}
}

void ServerTest::RunTestSequence()
{
	StartTrace(ServerTest.RunTestSequence);
	Anything testMessage1;
	Anything testMessage2;
	Anything replyMessage1;
	Anything replyMessage2;
	testMessage1 = "Hallo there first message";
	testMessage2 = "Hallo there second message";
	Connector c1(TESTHOST, GetConfig()["TCP5010"]["Port"].AsLong());
	Connector c2(TESTHOST, GetConfig()["TCP5011"]["Port"].AsLong());

	if (t_assert(c1.GetStream() != NULL) && t_assert(c2.GetStream() != NULL)) {
		testMessage1.PrintOn((*c1.GetStream()));
		testMessage2.PrintOn((*c2.GetStream()));
		t_assert(!!(*c1.GetStream()));
		t_assert(!!(*c2.GetStream()));

		replyMessage1.Import((*c1.GetStream()), "Server1 reply");
		replyMessage2.Import((*c2.GetStream()), "Server2 reply");
		t_assert(!!(*c1.GetStream()));
		t_assert(!!(*c2.GetStream()));

		assertAnyEqual(testMessage1, replyMessage1[0L]); // implies LoopbackProcessor
		assertAnyEqual(testMessage2, replyMessage2[0L]);
	}
}

Test *ServerTest::suite ()
{
	TestSuite *testSuite = new TestSuite;
	ADD_CASE(testSuite, ServerTest, InitRunTerminateAcceptorTest);
	ADD_CASE(testSuite, ServerTest, InitRunTerminateLeaderFollowerTest);
	ADD_CASE(testSuite, ServerTest, InitRunResetRunTerminateAcceptorTest);
	ADD_CASE(testSuite, ServerTest, InitRunResetRunTerminateLeaderFollowerTest);
	return testSuite;
}
