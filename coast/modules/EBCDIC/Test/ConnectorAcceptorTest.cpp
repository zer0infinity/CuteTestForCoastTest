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
#include "EBCDICConnector.h"

//--- interface include --------------------------------------------------------
#include "ConnectorAcceptorTest.h"

//--- standard modules used ----------------------------------------------------
#include "StringStream.h"
#include "Dbg.h"

//--- c-library modules used ---------------------------------------------------
#if defined(WIN32)
#include <io.h>
#endif

const char *localHost = "127.0.0.1";
const long localPort = 7183;

ConnectorAcceptorTest::ConnectorAcceptorTest(TString tname)
	:	TestCase(tname),
		fCallBack(0),
		fAcceptorThread(0)
{
	StartTrace(ConnectorAcceptorTest.Ctor);
}

ConnectorAcceptorTest::~ConnectorAcceptorTest()
{
	StartTrace(ConnectorAcceptorTest.Dtor);
}

void ConnectorAcceptorTest::setUp ()
// what: Setup the variables for all test cases in this class
{
	StartTrace(ConnectorAcceptorTest.setUp);
	TestCase::setUp();
	fCallBack = new EBCDICEchoMsgCallBack();
	fAcceptorThread = new AcceptorThread(new Acceptor(localHost, localPort, 2, fCallBack));
	t_assert(fCallBack != 0);
	t_assert(fAcceptorThread != 0);
	if (	(fCallBack) &&
			(fAcceptorThread) ) {
		bool startsuccesful = fAcceptorThread->Start();
		t_assertm(startsuccesful, "probably bind failed; since thread not started");
		bool running = fAcceptorThread->CheckState(Thread::eRunning, 3);
		if (!running) {
			t_assertm(running, "fAcceptorThread has to be running");
			fAcceptorThread->Terminate();
			fAcceptorThread->CheckState(Thread::eTerminated, 3);
		} else {
			return;
		}
	}
	// init failed or already terminated
	delete fAcceptorThread;
	fAcceptorThread = 0;
	t_assertm(fAcceptorThread != 0, "expected AcceptorThread to be ready; init or startup failed");
};

void ConnectorAcceptorTest::tearDown ()
{
	StartTrace(ConnectorAcceptorTest.tearDown);
	if (fAcceptorThread) {
		fAcceptorThread->Terminate();
		t_assert(fAcceptorThread->CheckState(Thread::eTerminated, 3));
		delete fAcceptorThread;
		fAcceptorThread = 0;
	} // if
} // tearDown

void ConnectorAcceptorTest::basicOperation ()
{
	StartTrace(ConnectorAcceptorTest.basicOperation);
	if (fAcceptorThread) {
		EBCDICConnector conn(localHost, localPort);

		Socket *socket = conn.MakeSocket();
		t_assert( socket != 0 );
		if ( socket != 0 ) {
			iostream *Ios = socket->GetStream();
			t_assert( Ios != 0 );
			if ( Ios != 0 ) {
				String input("ABC");
				(*Ios) << input << flush;
				Trace("written:<" << input << ">");
				t_assert(!!(*Ios));
				socket->ShutDownWriting();
				Trace("socket shutdown");
				t_assert(!!(*Ios));

				String reply;
				{
					StringStream os(&reply);
					t_assert(Ios->rdbuf() != 0);
					Trace("reading reply...");
					os << Ios->rdbuf() << flush;
					t_assert(!!(*Ios));
				}
				Trace("reply read:<" << reply << ">");
				assertEqual(input, reply);
				String inEBCDIC( fCallBack->GetLastRequest() );
				assertEqual( input.Length(), inEBCDIC.Length() );
				StringStream intermediate;
				inEBCDIC.IntPrintOn(intermediate);
				assertEqual( _QUOTE_("\xC1\xC2\xC3") , intermediate.str() );
			} // if
		} // if
	}
}

void ConnectorAcceptorTest::basicOperationWithAllocator ()
{
	StartTrace(ConnectorAcceptorTest.basicOperationWithAllocator);
	if (fAcceptorThread) {
		EBCDICConnector conn(localHost, localPort);
		conn.SetThreadLocal(true);

		Socket *socket = conn.MakeSocket();
		t_assert( socket != 0 );
		if ( socket != 0 ) {
			iostream *Ios = socket->GetStream();
			t_assert( Ios != 0 );
			if ( Ios != 0 ) {
				String input("ABC");
				(*Ios) << input << flush;
				t_assert(!!(*Ios));
				socket->ShutDownWriting();
				t_assert(!!(*Ios));

				StringStream reply;
				t_assert(Ios->rdbuf() != 0);
				reply << Ios->rdbuf() << flush;
				t_assert(!!(*Ios));
				assertEqual(input, reply.str());
				String inEBCDIC( fCallBack->GetLastRequest() );
				assertEqual( input.Length(), inEBCDIC.Length() );
				StringStream intermediate;
				inEBCDIC.IntPrintOn(intermediate);
				assertEqual( _QUOTE_("\xC1\xC2\xC3") , intermediate.str() );
			} // if
		} // if
	}
}

void ConnectorAcceptorTest::differentReply ()
{
	StartTrace(ConnectorAcceptorTest.differentReply);
	if (fAcceptorThread) {
		EBCDICConnector conn(localHost, localPort);

		Socket *socket = conn.MakeSocket();
		t_assert( socket != 0 );
		if ( socket != 0 ) {
			iostream *Ios = socket->GetStream();
			t_assert( Ios != 0 );
			if ( Ios != 0 ) {
				String EBCDICReply("\xC3\xC4\xC5\xC6");
				fCallBack->SetReplyMessage(EBCDICReply);
				String input("ABC");
				(*Ios) << input << flush;
				socket->ShutDownWriting();
				StringStream reply;
				reply << Ios->rdbuf() << flush;

				assertEqual("CDEF", reply.str());
			} // if
		} // if
	}
} // differentReply

Test *ConnectorAcceptorTest::suite ()
{
	StartTrace(ConnectorAcceptorTest.suite);
	TestSuite *testSuite = new TestSuite;
	ADD_CASE(testSuite, ConnectorAcceptorTest, basicOperation);
	ADD_CASE(testSuite, ConnectorAcceptorTest, differentReply);
	ADD_CASE(testSuite, ConnectorAcceptorTest, basicOperationWithAllocator);

	return testSuite;
}

String &EBCDICEchoMsgCallBack::GetLastRequest()
{
	StartTrace(EBCDICEchoMsgCallBack.GetLastRequest);
	return fLastRequest;
}

void EBCDICEchoMsgCallBack::SetReplyMessage( String &reply )
{
	StartTrace(EBCDICEchoMsgCallBack.SetReplyMessage);
	fReplyMessage = reply;
}

void EBCDICEchoMsgCallBack::CallBack(Socket *socket)
{
	StartTrace(EBCDICEchoMsgCallBack.CallBack);
	if (socket != 0) {	// might be 0 if we run out of memory, not likely
		StringStream is(&fLastRequest);
		iostream *Ios = socket->GetStream();

		if ( Ios != 0 ) {
			is << Ios->rdbuf() << flush;
			if ( fReplyMessage.Length() > 0 ) {
				(*Ios) << fReplyMessage << flush;
			} else {
				(*Ios) << fLastRequest << flush;
			} // if
		} // if
		socket->ShutDownWriting();
	} // if
	delete socket;
} // CallBack

AcceptorThread::~AcceptorThread()
{
	StartTrace(AcceptorThread.Dtor);
	if (fAcceptor) {
		delete fAcceptor;
	}
	fAcceptor = 0;
}

bool AcceptorThread::DoStartRequestedHook(ROAnything args)
{
	StartTrace(AcceptorThread.DoStartRequestedHook);
	if (fAcceptor) {
		fAcceptor->SetThreadLocal(true);
		return (fAcceptor->PrepareAcceptLoop() == 0);
	}
	return false;
}

void AcceptorThread::Run()
{
	StartTrace(AcceptorThread.Run);
	if (fAcceptor) {
		fAcceptor->RunAcceptLoop();
	} // if

}

void AcceptorThread::DoTerminationRequestHook(ROAnything)
{
	StartTrace(AcceptorThread.DoTerminationRequestHook);
	if (fAcceptor) {
		fAcceptor->StopAcceptLoop();
	}
}
