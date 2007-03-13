/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

//--- interface include --------------------------------------------------------
#include "InterruptHandlerTest.h"

//--- test modules used --------------------------------------------------------
#include "TestSuite.h"

//--- module under test --------------------------------------------------------
#include "ServerUtils.h"

//--- standard modules used ----------------------------------------------------
#include "Server.h"

//--- c-library modules used ---------------------------------------------------
#if defined(WIN32)
#include <signal.h>
#endif

//---- InterruptHandlerTest ----------------------------------------------------------------
InterruptHandlerTest::InterruptHandlerTest(TString tname)
	: TestCaseType(tname)
{
	StartTrace(InterruptHandlerTest.InterruptHandlerTest);
}

InterruptHandlerTest::~InterruptHandlerTest()
{
	StartTrace(InterruptHandlerTest.Dtor);
}

void InterruptHandlerTest::setUp ()
{
	StartTrace(InterruptHandlerTest.setUp);
	t_assert(GetConfig().IsDefined("Modules"));
}

void InterruptHandlerTest::PidFileHandlingTest()
{
	StartTrace(InterruptHandlerTest.PidFileHandlingTest);
	Server *server = Server::FindServer("Server");

	if (t_assertm(server != 0, "expected 'Server' to be there")) {
#if !defined(WIN32)
		sigset_t set;
		sigemptyset(&set);
		sigaddset(&set, SIGHUP);
		sigaddset(&set, SIGINT);
		sigaddset(&set, SIGPIPE);
		sigaddset(&set, SIGTERM);

		THRSETSIGMASK(SIG_BLOCK, &set, NULL);
#endif
		{
			// kill without using file
			InterruptHandler ih(server); // starts thread
#if !defined(WIN32)
			if (t_assertm(ih.CheckRunningState(Thread::eWorking, 5), "expected InterruptHandler to startup ok"))
#endif
			{
				String cmd("kill -15 ");
				cmd << (long)server->GetPid();

				// on linux: only after the start of the interrupt handler you have to correct pid
				String pidfilename;
				server->PIDFileName(pidfilename);
				Trace("PIDFileName: <" << pidfilename << ">");
				iostream *Ios = System::OpenStream(pidfilename, (const char *)0, ios::in, true);
				if ( t_assertm(Ios != 0, "expected pid file to be there") ) {
					delete Ios;
				}
				String msg("expected '");
				msg << cmd << "' to succeed";

#if !defined(WIN32)
				t_assertm((::system(cmd) == 0), (const char *)msg);
				ih.CheckState(Thread::eTerminated, 5);
#else
				// until we find a way to send SIGTERM from an external program to this process
				// we need to test this funtionality with raise
				raise(SIGTERM);
#endif
			}
		}
		String pidfilename;
		server->PIDFileName(pidfilename);
		Trace("trying to open <" << pidfilename << ">");
		iostream *Ios = System::OpenStream(pidfilename, (const char *)0);
		if ( !t_assertm(Ios == 0, "expected pid file to be deleted") ) {
			delete Ios;
		}
	}
}

// builds up a suite of testcases, add a line for each testmethod
Test *InterruptHandlerTest::suite ()
{
	StartTrace(InterruptHandlerTest.suite);
	TestSuite *testSuite = new TestSuite;
	ADD_CASE(testSuite, InterruptHandlerTest, PidFileHandlingTest);
	return testSuite;
}
