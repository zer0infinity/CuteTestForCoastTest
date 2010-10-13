/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

//--- interface include --------------------------------------------------------
#include "ConnectorDAImplTest.h"

//--- test modules used --------------------------------------------------------
#include "TestSuite.h"

//--- module under test --------------------------------------------------------
#include "ConnectorDAImpl.h"

//--- standard modules used ----------------------------------------------------
#include "SocketStream.h"
#include "Dbg.h"
#include "SystemLog.h"
#include "DataAccess.h"

//--- c-modules used -----------------------------------------------------------
#if defined(WIN32)
#include <io.h>				// for SO_ERROR
#else
#include <sys/socket.h>		// for SO_ERROR
#endif

class EchoMsgCallBack: public AcceptorCallBack {
public:
	//!default constructor is empty since class provides only protocol no implementation
	EchoMsgCallBack() :
		fReplyMessage("Accepted") {
	}

	//!default destructor is empty since class provides only protocol no implementation
	virtual ~EchoMsgCallBack() {
	}

	//!call back functionality provides the socket object
	//! \param socket the Socket object resulting from the accepted connection
	virtual void CallBack(Socket *pSocket) {
		StartTrace(EchoMsgCallBack.CallBack);
		if (pSocket != 0) { // might be 0 if we run out of memory, not likely
			while (true) {
				std::iostream *Ios = pSocket->GetStream();
				if (Ios != 0 && (*Ios)) {
					TimeoutModifier aTimeoutModifier((SocketStream *) Ios, 10 * 1000);
					aTimeoutModifier.Use();
					Trace("stream valid");
					// get character from stream, blocking call
					int ch = Ios->get();
					if ((ch == EOF) || !Ios->good()) {
						SYSINFO("termination requested due to closed or bad stream");
						break;
					}
					// put back character to allow reading of whole message
					Ios->putback(ch);
					Trace("First character from stream " << String("0x").AppendAsHex(ch) << ":[" << (char)ch << "]");

					Anything anyMessage;
					anyMessage.Import(*Ios, "importing stream");
					TraceAny(anyMessage, "Message");
					fLastRequest = anyMessage;
					long lRetCode;
					if (pSocket->IsReadyForWriting(10 * 1000, lRetCode)) {
						Trace("sending back reply message");
						fReplyMessage.PrintOn(*Ios, false) << std::flush;
					} else {
						SYSWARNING("socket not ready for writing!");
					}
					if (anyMessage.IsDefined("CloseConnection") && anyMessage["CloseConnection"].AsBool(false)) {
						Trace("closing socket");
						break;
					}
				}
			}
			pSocket->ShutDown();
		}
		delete pSocket;
	}

	//!I return the last message I read
	virtual Anything &GetLastRequest() {
		StartTrace(EchoMsgCallBack.GetLastRequest);
		return fLastRequest;
	}

	//!Set the message that I shall return on subsequent requests
	void SetReplyMessage(Anything &reply) {
		StartTrace(EchoMsgCallBack.SetReplyMessage);
		fReplyMessage = reply;
	}

protected:
	// no locking as only one thread initiates actions and the second thread
	// only accesses while the first one waits for I/O
	Anything fLastRequest;
	Anything fReplyMessage;
};

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
		}
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

//---- ConnectorDAImplTest ----------------------------------------------------------------
ConnectorDAImplTest::ConnectorDAImplTest(TString strName) :
	TestCaseType(strName), fCallBack(0), fAcceptorThread(0) {
	StartTrace(ConnectorDAImplTest.ConnectorDAImplTest);
}

TString ConnectorDAImplTest::getConfigFileName() {
	return "ConnectorDAImplTestConfig";
}

ConnectorDAImplTest::~ConnectorDAImplTest() {
	StartTrace(ConnectorDAImplTest.Dtor);
}

void ConnectorDAImplTest::setUp() {
	StartTrace(ConnectorDAImplTest.setUp);
	TraceAny(GetTestCaseConfig(), "TestCaseConfig");
	fCallBack = new EchoMsgCallBack();
	fAcceptorThread = new (Storage::Global()) AcceptorThread(new Acceptor(GetTestCaseConfig()["Address"].AsString(), GetTestCaseConfig()["Port"].AsLong(), 2, fCallBack));
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

void ConnectorDAImplTest::tearDown() {
	StartTrace(ConnectorDAImplTest.tearDown);
	if (fAcceptorThread) {
		fAcceptorThread->Terminate();
		t_assert(fAcceptorThread->CheckState(Thread::eTerminated, 3));
		delete fAcceptorThread;
		fAcceptorThread = 0;
	}
}

bool ConnectorDAImplTest::IsSocketValid(int socketFd) {
	StartTrace(ConnectorDAImplTest.IsSocketValid);
	int error = 0;
	bool bSuccess = Socket::GetSockOptInt(socketFd, SO_ERROR, error);
	if (!bSuccess) {
		SYSWARNING("socketFd " << (long)socketFd << " not valid anymore, error:" << (long)error);
		return false;
	}
	return true;
}

void ConnectorDAImplTest::SendReceiveOnceTest() {
	StartTrace(ConnectorDAImplTest.SendReceiveOnceTest);
	TraceAny(GetTestCaseConfig(), "TestCaseConfig");
	Anything anyEnv = GetTestCaseConfig().DeepClone();
	Context ctx(anyEnv);
	DataAccess aDA("SendReceiveOnce");
	if ( t_assert(aDA.StdExec(ctx)) ) {
		Anything anyExpected;
		String strIn = GetTestCaseConfig()["Input"].AsString();
		IStringStream stream(strIn);
		anyExpected.Import(stream);
		assertAnyEqual(anyExpected, fCallBack->GetLastRequest());
	}
	TraceAny(ctx.GetTmpStore(), "TmpStore");
}

void ConnectorDAImplTest::RecreateSocketTest() {
	StartTrace(ConnectorDAImplTest.RecreateSocketTest);
	TraceAny(GetTestCaseConfig(), "TestCaseConfig");
	Anything anyEnv = GetTestCaseConfig().DeepClone();
	Context ctx(anyEnv);
	{
		DataAccess aDA("RecreateSocket");
		ctx.GetTmpStore()["Input"] = "{ \"Kurt hat recht\" }";
		if ( t_assert(aDA.StdExec(ctx)) ) {
			Anything anyExpected;
			String strIn = ctx.GetTmpStore()["Input"].AsString();
			IStringStream stream(strIn);
			anyExpected.Import(stream);
			assertAnyEqual(anyExpected, fCallBack->GetLastRequest());
		}
		TraceAny(ctx.GetTmpStore(), "TmpStore");
	}
	// test if socketFd is still valid
	int socketFd = ctx.GetTmpStore()["SocketParams"]["SocketFd"].AsLong(0L);
	if (assertCompare(0, not_equal_to, socketFd)) {
		t_assertm(IsSocketValid(socketFd), "expected socket to be valid");
		DataAccess aDA("SendReceiveOnce");
		ctx.GetTmpStore()["Input"] = "{ \"Marcel auch\" }";
		if ( t_assert(aDA.StdExec(ctx)) ) {
			Anything anyExpected;
			String strIn(ctx.GetTmpStore()["Input"].AsString());
			IStringStream stream(strIn);
			anyExpected.Import(stream);
			assertAnyEqual(anyExpected, fCallBack->GetLastRequest());
		}
		TraceAny(ctx.GetTmpStore(), "TmpStore");
	}
	// test if socketFd is not valid any longer
	t_assertm(!IsSocketValid(socketFd), "expected socket to be invalid");
}

void ConnectorDAImplTest::RecreateClosedSocketTest() {
	StartTrace(ConnectorDAImplTest.RecreateSocketTest);
	TraceAny(GetTestCaseConfig(), "TestCaseConfig");
	Anything anyEnv = GetTestCaseConfig().DeepClone();
	Context ctx(anyEnv);
	{
		DataAccess aDA("RecreateSocket");
		ctx.GetTmpStore()["Input"] = "{ \"Kurt hat recht\" }";
		if ( t_assert(aDA.StdExec(ctx)) ) {
			Anything anyExpected;
			String strIn = ctx.GetTmpStore()["Input"].AsString();
			IStringStream stream(strIn);
			anyExpected.Import(stream);
			assertAnyEqual(anyExpected, fCallBack->GetLastRequest());
		}
		TraceAny(ctx.GetTmpStore(), "TmpStore");
	}

	// test if socketFd is still valid
	int socketFd = ctx.GetTmpStore()["SocketParams"]["SocketFd"].AsLong(0L);
	if (assertCompare(0, not_equal_to, socketFd)) {
		t_assertm(IsSocketValid(socketFd), "expected socket to be valid");
		// force EchoMsgCallBack to close the socket from the server side
		DataAccess aDA("RecreateSocket");
		Anything anyMessage;
		anyMessage["CloseConnection"] = 1;
		ctx.GetTmpStore()["Input"] = anyMessage;
		if ( t_assert(aDA.StdExec(ctx)) ) {
			assertAnyEqual(anyMessage, fCallBack->GetLastRequest());
		}
		TraceAny(ctx.GetTmpStore(), "TmpStore");
	}

	// although the socket is closed from the server side, the socketFd should still seem to be valid
	socketFd = ctx.GetTmpStore()["SocketParams"]["SocketFd"].AsLong(0L);
	if (assertCompare(0, not_equal_to, socketFd)) {
		t_assertm(IsSocketValid(socketFd), "expected socket to be valid");
		DataAccess aDA("SendReceiveOnce");
		ctx.GetTmpStore()["Input"] = "{ \"Marcel auch\" }";
		if ( t_assert(aDA.StdExec(ctx)) ) {
			Anything anyExpected;
			String strIn(ctx.GetTmpStore()["Input"].AsString());
			IStringStream stream(strIn);
			anyExpected.Import(stream);
			assertAnyEqual(anyExpected, fCallBack->GetLastRequest());
		}
		TraceAny(ctx.GetTmpStore(), "TmpStore");
	}
	// test if socketFd is not valid any longer
	t_assertm(!IsSocketValid(socketFd), "expected socket to be invalid");
}

// builds up a suite of testcases, add a line for each testmethod
Test *ConnectorDAImplTest::suite() {
	StartTrace(ConnectorDAImplTest.suite);
	TestSuite *testSuite = new TestSuite;
	ADD_CASE(testSuite, ConnectorDAImplTest, SendReceiveOnceTest);
	ADD_CASE(testSuite, ConnectorDAImplTest, RecreateSocketTest);
	ADD_CASE(testSuite, ConnectorDAImplTest, RecreateClosedSocketTest);
	return testSuite;
}
