/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

//--- test modules used --------------------------------------------------------
#include "TestSuite.h"

//--- module under test --------------------------------------------------------
#include "Server.h"

//--- interface include --------------------------------------------------------
#include "ServerTest.h"

//--- standard modules used ----------------------------------------------------
#include "Socket.h"
#include "StringStream.h"
#include "Context.h"
#include "Dbg.h"

//--- c-library modules used ---------------------------------------------------

#define TESTHOST "localhost"

//---- ServerTest ----------------------------------------------------------------
ServerTest::ServerTest(TString tname) : ConfiguredTestCase(tname, "Config")
{
	StartTrace(ServerTest.Ctor);
}

ServerTest::~ServerTest()
{
	StartTrace(ServerTest.Dtor);
}

void ServerTest::setUp ()
{
	StartTrace(ServerTest.setUp);
	ConfiguredTestCase::setUp();
	t_assert(fConfig.IsDefined("Modules"));
	Application::InitializeGlobalConfig(fConfig);
	WDModule::Install(fConfig);
	Server *s;
	if (t_assert((s = Server::FindServer("Server")) != NULL)) {
		ROAnything result;
		t_assert(s->Lookup("TCP5010", result));
		TraceAny(result, "server lookup TCP5010");
	}
}

void ServerTest::tearDown ()
{
	StartTrace(ServerTest.tearDown);
	WDModule::Terminate(fConfig);
	Application::InitializeGlobalConfig(Anything());

	ConfiguredTestCase::tearDown();
}

void ServerTest::InitRunTerminateTest()
{
	StartTrace(ServerTest.InitRunTerminateTest);
	for (long i = 0; i < 10; i++) {
		{
			Server *server = Server::FindServer("AcceptorWorkerServer");
			if ( t_assertm(server != NULL, "expected AcceptorsWorkersServer to be there") ) {
				server = (Server *)server->ConfiguredClone("Server", "AcceptorWorkerServer");
				if ( t_assertm(server->Init() == 0, "expected initialization to succeed") ) {
					ServerThread mt(server);
					if (t_assert(mt.Start()) &&
						t_assert(mt.CheckState(Thread::eRunning, 5))
					   ) {
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
				delete server;
			}
		}
		{
			Server *server = Server::FindServer("LeaderFollowerServer");
			if ( t_assertm(server != NULL, "expected LeaderFollowerServer to be there") ) {
				server = (Server *)server->ConfiguredClone("Server", "LeaderFollowerServer");
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
				delete server;
			}
		}
	}
}

void ServerTest::InitRunResetRunTerminateTest ()
{
	StartTrace(ServerTest.InitRunResetRunTerminateTest);
	for (long i = 0; i < 10; i++) {
		{
			Server *server = Server::FindServer("AcceptorWorkerServer");
			if ( t_assertm(server != NULL, "expected AcceptorsWorkersServer to be there") ) {
				server = (Server *)server->ConfiguredClone("Server", "AcceptorWorkerServer");
				if ( t_assertm(server->Init() == 0, "expected initialization to succeed") ) {
					ServerThread mt(server);
					if (t_assert(mt.Start()) &&
						t_assert(mt.CheckState(Thread::eRunning, 5))) {
						if (t_assertm(server->IsReady(true, 5), "expected server to become ready within 5 seconds")) {
							// --- run various request
							//     sequences
							RunTestSequence();
							if ( t_assertm(server->GlobalReinit() == 0, "expected server to reinit ok") ) {
								RunTestSequence();
							}
							server->PrepareShutdown(0);
						}
					}
					if (t_assertm(server->IsReady(false, 5), "expected server to become terminated within 5 seconds")) {
						mt.Terminate(0);
					}
				}
				delete server;
			}
		}
		{
			Server *server = Server::FindServer("LeaderFollowerServer");
			if ( t_assertm(server != NULL, "expected LeaderFollowerServer to be there") ) {
				server = (Server *)server->ConfiguredClone("Server", "LeaderFollowerServer");
				if ( t_assertm(server->Init() == 0, "expected initialization to succeed") ) {
					ServerThread mt(server);
					if (t_assert(mt.Start()) &&
						t_assert(mt.CheckState(Thread::eRunning, 5))) {
						if (t_assertm(server->IsReady(true, 5), "expected server to become ready within 5 seconds")) {
							// --- run various request
							//     sequences
							RunTestSequence();
							if ( t_assertm(server->GlobalReinit() == 0, "expected server to reinit ok") ) {
								RunTestSequence();
							}
							server->PrepareShutdown(0);
						}
					}
					if (t_assertm(server->IsReady(false, 5), "expected server to become terminated within 5 seconds")) {
						mt.Terminate(0);
					}
				}
				delete server;
			}
		}
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
	Connector c1(TESTHOST, fConfig["TCP5010"]["Port"].AsLong());
	Connector c2(TESTHOST, fConfig["TCP5011"]["Port"].AsLong());

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
	ADD_CASE(testSuite, ServerTest, InitRunTerminateTest);
	ADD_CASE(testSuite, ServerTest, InitRunResetRunTerminateTest);
	return testSuite;
}
