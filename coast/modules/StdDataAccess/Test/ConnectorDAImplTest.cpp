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
#include "SysLog.h"
#include "System.h"
#include "DataAccess.h"

//--- c-modules used -----------------------------------------------------------
#if defined(WIN32)
#include <io.h>				// for SO_ERROR
#else
#include <sys/socket.h>		// for SO_ERROR
#endif

class AcceptorThread : public Thread
{
public:
	AcceptorThread(Acceptor *acceptor) : Thread("AcceptorThread"), fAcceptor(acceptor)
	{ }
	~AcceptorThread();
	void Run();

protected:
	bool DoStartRequestedHook(ROAnything args);
	void DoTerminationRequestHook(ROAnything);

private:
	Acceptor *fAcceptor;
};

class EchoMsgCallBack : public AcceptorCallBack
{
public:

	//!default constructor is empty since class provides only protocol no implementation
	EchoMsgCallBack() : fReplyMessage("Accepted") {};

	//!default destructor is empty since class provides only protocol no implementation
	virtual ~EchoMsgCallBack() {};

	//!call back functionality provides the socket object
	//! \param socket the Socket object resulting from the accepted connection
	virtual void CallBack(Socket *socket);

	//!I return the last message I read
	virtual Anything &GetLastRequest();

	//!Set the message that I shall return on subsequent requests
	void SetReplyMessage(Anything &reply);

protected:
	// no locking as only one thread initiates actions and the second thread
	// only accesses while the first one waits for I/O
	Anything	fLastRequest;
	Anything	fReplyMessage;
};

//---- ConnectorDAImplTest ----------------------------------------------------------------
ConnectorDAImplTest::ConnectorDAImplTest(TString strName)
	: ConfiguredTestCase(strName, "ConnectorDAImplTestConfig")
	, fCallBack(0)
	, fAcceptorThread(0)
{
	StartTrace(ConnectorDAImplTest.Ctor);
}

ConnectorDAImplTest::~ConnectorDAImplTest()
{
	StartTrace(ConnectorDAImplTest.Dtor);
}

// setup for this TestCase
void ConnectorDAImplTest::setUp ()
{
	StartTrace(ConnectorDAImplTest.setUp);
	ConfiguredTestCase::setUp();

	TraceAny(fTestCaseConfig, "TestCaseConfig");
	ROAnything roaConfig(fTestCaseConfig);
	String strHost = roaConfig["Address"].AsString();
	long   lPort = roaConfig["Port"].AsLong();

	fCallBack = new EchoMsgCallBack();
	fAcceptorThread = new AcceptorThread(new Acceptor(strHost, lPort, 2, fCallBack));
	t_assert(fCallBack != 0);
	t_assert(fAcceptorThread != 0);
	if ( (fCallBack) && (fAcceptorThread) ) {
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

void ConnectorDAImplTest::tearDown ()
{
	StartTrace(ConnectorDAImplTest.tearDown);
	ConfiguredTestCase::tearDown();

	if (fAcceptorThread) {
		fAcceptorThread->Terminate();
		t_assert(fAcceptorThread->CheckState(Thread::eTerminated, 3));
		delete fAcceptorThread;
		fAcceptorThread = 0;
	}
}

bool ConnectorDAImplTest::IsSocketValid(int socketFd)
{
	StartTrace(ConnectorDAImplTest.IsSocketValid);
	int error = 0;
	bool bSuccess = Socket::GetSockOptInt(socketFd, SO_ERROR, error);
	if ( !bSuccess ) {
		SYSWARNING("socketFd " << (long)socketFd << " not valid anymore, error:" << (long)error);
		return false;
	}
	return true;
}

void ConnectorDAImplTest::SendReceiveOnceTest()
{
	StartTrace(ConnectorDAImplTest.SendReceiveOnceTest);
	TraceAny(fTestCaseConfig, "TestCaseConfig");

	Context ctx(fTestCaseConfig);
	DataAccess aDA("SendReceiveOnce");
	t_assert(aDA.StdExec(ctx));
	Anything anyExpected;
	StringStream stream(fTestCaseConfig["Input"].AsString());
	anyExpected.Import(stream);
	assertAnyEqual(anyExpected, fCallBack->GetLastRequest());
	TraceAny(ctx.GetTmpStore(), "TmpStore");
}

void ConnectorDAImplTest::RecreateSocketTest()
{
	StartTrace(ConnectorDAImplTest.RecreateSocketTest);
	TraceAny(fTestCaseConfig, "TestCaseConfig");

	Context ctx(fTestCaseConfig);
	{
		DataAccess aDA("RecreateSocket");
		ctx.GetTmpStore()["Input"] = "{ \"Kurt hat recht\" }";
		t_assert(aDA.StdExec(ctx));
		Anything anyExpected;
		StringStream stream(ctx.GetTmpStore()["Input"].AsString());
		anyExpected.Import(stream);
		assertAnyEqual(anyExpected, fCallBack->GetLastRequest());
	}
	// test if socketFd is still valid
	int socketFd = ctx.GetTmpStore()["SocketParams"]["SocketFd"].AsLong(0L);
	if ( t_assert(socketFd != 0L) ) {
		t_assertm(IsSocketValid(socketFd), "expected socket to be valid");
		{
			DataAccess aDA("SendReceiveOnce");
			ctx.GetTmpStore()["Input"] = "{ \"Marcel auch\" }";
			t_assert(aDA.StdExec(ctx));
			Anything anyExpected;
			StringStream stream(ctx.GetTmpStore()["Input"].AsString());
			anyExpected.Import(stream);
			assertAnyEqual(anyExpected, fCallBack->GetLastRequest());
		}
	}
	// test if socketFd is not valid any longer
	t_assertm(!IsSocketValid(socketFd), "expected socket to be invalid");
}

Anything &EchoMsgCallBack::GetLastRequest()
{
	StartTrace(EchoMsgCallBack.GetLastRequest);
	return fLastRequest;
}

void EchoMsgCallBack::SetReplyMessage( Anything &reply )
{
	StartTrace(EchoMsgCallBack.SetReplyMessage);
	fReplyMessage = reply;
}

void EchoMsgCallBack::CallBack(Socket *pSocket)
{
	StartTrace(EchoMsgCallBack.CallBack);
	if (pSocket != 0) {	// might be 0 if we run out of memory, not likely
		while ( true ) {
			iostream *Ios = pSocket->GetStream();
			if ( Ios != 0 ) {
				TimeoutModifier aTimeoutModifier((SocketStream *) Ios, 10 * 1000);
				aTimeoutModifier.Use();
				Trace("stream valid");
				// get character from stream, blocking call
				int ch = Ios->get();
				if ( (ch == EOF) || !Ios->good() ) {
					SYSINFO("termination requested due to closed or bad stream");
					break;
				}
				// put back character to allow reading of whole message
				Ios->putback(ch);
				Trace("First character from stream " << String("0x").AppendAsHex(ch) << ":[" << (char)ch << "]");

				Anything anyMessage;
				(*Ios ) >> anyMessage;
				TraceAny(anyMessage, "Message");
				fLastRequest = anyMessage;
				long lRetCode;
				if ( pSocket->IsReadyForWriting(10 * 1000, lRetCode) ) {
					Trace("sending back reply message");
					fReplyMessage.PrintOn(*Ios, false) << flush;
				} else {
					SYSWARNING("socket not ready for writing!");
				}
			}
		}
		pSocket->ShutDown();
	}
	delete pSocket;
}

// ---------- AcceptorThread ------------------------------
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
	}
}

void AcceptorThread::DoTerminationRequestHook(ROAnything)
{
	StartTrace(AcceptorThread.DoTerminationRequestHook);
	if (fAcceptor) {
		fAcceptor->StopAcceptLoop();
	}
}

// builds up a suite of testcases, add a line for each testmethod
Test *ConnectorDAImplTest::suite ()
{
	StartTrace(ConnectorDAImplTest.suite);
	TestSuite *testSuite = new TestSuite;

	ADD_CASE(testSuite, ConnectorDAImplTest, SendReceiveOnceTest);
	ADD_CASE(testSuite, ConnectorDAImplTest, RecreateSocketTest);

	return testSuite;
}
