/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#include "ConnectorAcceptorTest.h"
#include "EBCDICConnector.h"
#include "TestSuite.h"
#include "Socket.h"
#include "Threads.h"
#include "StringStream.h"
#include "Tracer.h"
#if defined(WIN32)
#include <io.h>
#endif

//!Callback for echo or return of a fixed message
//! echos incoming messages if the length of the return message is 0 or
//! returns a fixed message if one has been set
class EBCDICEchoMsgCallBack: public AcceptorCallBack {
public:

	//!default constructor is empty since class provides only protocol no implementation
	EBCDICEchoMsgCallBack() :
		fLastRequest(), fReplyMessage() {
	}

	//!default destructor is empty since class provides only protocol no implementation
	virtual ~EBCDICEchoMsgCallBack() {
	}

	//!call back functionality provides the socket object
	//! \param socket the Socket object resulting from the accepted connection
	virtual void CallBack(Socket *socket) {
		StartTrace(EBCDICEchoMsgCallBack.CallBack);
		if (socket != 0) { // might be 0 if we run out of memory, not likely
			StringStream is(&fLastRequest);
			std::iostream *Ios = socket->GetStream();

			if (Ios != 0) {
				is << Ios->rdbuf() << std::flush;
				if (fReplyMessage.Length() > 0) {
					(*Ios) << fReplyMessage << std::flush;
				} else {
					(*Ios) << fLastRequest << std::flush;
				} // if
			} // if
			socket->ShutDownWriting();
		} // if
		delete socket;
	} // CallBack
	virtual void Wait() {
		// dummy wait to force context switch
		Thread::Wait(0,1000L);
	}
	virtual void Signal() {
		// dummy wait to force context switch
		Thread::Wait(0,1000L);
	}

	//!I return the last message I read
	virtual String &GetLastRequest() {
		StartTrace(EBCDICEchoMsgCallBack.GetLastRequest);
		return fLastRequest;
	}

	//!Set the message that I shall return on subsequent requests
	void SetReplyMessage(String &reply) {
		StartTrace(EBCDICEchoMsgCallBack.SetReplyMessage);
		fReplyMessage = reply;
	}

protected:
	// no locking as only one thread initiates actions and the second thread
	// only accesses while the first one waits for I/O
	String fLastRequest;
	String fReplyMessage;
};

//!a thread to run a specific acceptor
//! very simple server thread that allows one connection, useful for testing Sockets
class AcceptorThread: public Thread {
public:
	AcceptorThread(Acceptor *acceptor) :
		Thread("AcceptorThread"), fAcceptor(acceptor) {
	}
	~AcceptorThread() {
		StartTrace(AcceptorThread.Dtor);
		if (fAcceptor) {
			delete fAcceptor;
		}
		fAcceptor = 0;
	}

	void Run() {
		StartTrace(AcceptorThread.Run);
		if (fAcceptor) {
			fAcceptor->RunAcceptLoop();
		} // if
	}

protected:
	bool DoStartRequestedHook(ROAnything args) {
		StartTrace(AcceptorThread.DoStartRequestedHook);
		if (fAcceptor) {
			fAcceptor->SetThreadLocal(true);
			return (fAcceptor->PrepareAcceptLoop() == 0);
		}
		return false;
	}
	void DoTerminationRequestHook(ROAnything) {
		StartTrace(AcceptorThread.DoTerminationRequestHook);
		if (fAcceptor) {
			fAcceptor->StopAcceptLoop();
		}
	}
private:
	Acceptor *fAcceptor;
};

ConnectorAcceptorTest::ConnectorAcceptorTest(TString tname) :
	TestCaseType(tname), fCallBack(0), fAcceptorThread(0) {
	StartTrace(ConnectorAcceptorTest.Ctor);
}

ConnectorAcceptorTest::~ConnectorAcceptorTest() {
	StartTrace(ConnectorAcceptorTest.Dtor);
}

void ConnectorAcceptorTest::setUp() {
	StartTrace(ConnectorAcceptorTest.setUp); TraceAny(GetTestCaseConfig(), "TestCaseConfig");
	String strHost = GetTestCaseConfig()["Address"].AsString();
	long lPort = GetTestCaseConfig()["Port"].AsLong();

	fCallBack = new EBCDICEchoMsgCallBack();
	fAcceptorThread = new (coast::storage::Global()) AcceptorThread(new Acceptor(strHost, lPort, 2, fCallBack));
	t_assert(fCallBack != 0);
	t_assert(fAcceptorThread != 0);
	if ((fCallBack) && (fAcceptorThread)) {
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
}

void ConnectorAcceptorTest::tearDown() {
	StartTrace(ConnectorAcceptorTest.tearDown);
	if (fAcceptorThread) {
		fAcceptorThread->Terminate();
		t_assert(fAcceptorThread->CheckState(Thread::eTerminated, 3));
		delete fAcceptorThread;
		fAcceptorThread = 0;
	}
}

void ConnectorAcceptorTest::basicOperation() {
	StartTrace(ConnectorAcceptorTest.basicOperation);
	if (fAcceptorThread) {
		TraceAny(GetTestCaseConfig(), "TestCaseConfig");
		EBCDICConnector conn(GetTestCaseConfig()["Address"].AsString(), GetTestCaseConfig()["Port"].AsLong());
		Socket *socket = conn.MakeSocket();
		t_assert( socket != 0 );
		if (socket != 0) {
			std::iostream *Ios = socket->GetStream();
			t_assert( Ios != 0 );
			if (Ios != 0) {
				String input("ABC");
				(*Ios) << input << std::flush;
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
					os << Ios->rdbuf() << std::flush;
					t_assert(!!(*Ios));
				}
				Trace("reply read:<" << reply << ">");
				assertEqual(input, reply);
				String inEBCDIC(fCallBack->GetLastRequest());
				assertEqual( input.Length(), inEBCDIC.Length() );
				StringStream intermediate;
				inEBCDIC.IntPrintOn(intermediate);
				assertEqual( _QUOTE_("\xC1\xC2\xC3") , intermediate.str() );
			} // if
		} // if
	}
}

void ConnectorAcceptorTest::basicOperationWithAllocator() {
	StartTrace(ConnectorAcceptorTest.basicOperationWithAllocator);
	if (fAcceptorThread) {
		EBCDICConnector conn(GetTestCaseConfig()["Address"].AsString(), GetTestCaseConfig()["Port"].AsLong());
		conn.SetThreadLocal(true);

		Socket *socket = conn.MakeSocket();
		t_assert( socket != 0 );
		if (socket != 0) {
			std::iostream *Ios = socket->GetStream();
			t_assert( Ios != 0 );
			if (Ios != 0) {
				String input("ABC");
				(*Ios) << input << std::flush;
				t_assert(!!(*Ios));
				socket->ShutDownWriting();
				t_assert(!!(*Ios));

				StringStream reply;
				t_assert(Ios->rdbuf() != 0);
				reply << Ios->rdbuf() << std::flush;
				t_assert(!!(*Ios));
				assertEqual(input, reply.str());
				String inEBCDIC(fCallBack->GetLastRequest());
				assertEqual( input.Length(), inEBCDIC.Length() );
				StringStream intermediate;
				inEBCDIC.IntPrintOn(intermediate);
				assertEqual( _QUOTE_("\xC1\xC2\xC3") , intermediate.str() );
			} // if
		} // if
	}
}

void ConnectorAcceptorTest::differentReply() {
	StartTrace(ConnectorAcceptorTest.differentReply);
	if (fAcceptorThread) {
		EBCDICConnector conn(GetTestCaseConfig()["Address"].AsString(), GetTestCaseConfig()["Port"].AsLong());

		Socket *socket = conn.MakeSocket();
		t_assert( socket != 0 );
		if (socket != 0) {
			std::iostream *Ios = socket->GetStream();
			t_assert( Ios != 0 );
			if (Ios != 0) {
				String EBCDICReply("\xC3\xC4\xC5\xC6");
				fCallBack->SetReplyMessage(EBCDICReply);
				String input("ABC");
				(*Ios) << input << std::flush;
				socket->ShutDownWriting();
				StringStream reply;
				reply << Ios->rdbuf() << std::flush;

				assertEqual("CDEF", reply.str());
			} // if
		} // if
	}
} // differentReply

Test *ConnectorAcceptorTest::suite() {
	StartTrace(ConnectorAcceptorTest.suite);
	TestSuite *testSuite = new TestSuite;
	ADD_CASE(testSuite, ConnectorAcceptorTest, basicOperation);
	ADD_CASE(testSuite, ConnectorAcceptorTest, differentReply);
	ADD_CASE(testSuite, ConnectorAcceptorTest, basicOperationWithAllocator);
	return testSuite;
}
