/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _ConnectorAcceptorTest_h_
#define _ConnectorAcceptorTest_h_

#include "TestCase.h"
#include "Socket.h"
#include "Threads.h"

//!Callback for echo or return of a fixed message
//! echos incoming messages if the length of the return message is 0 or
//! returns a fixed message if one has been set
class EBCDICEchoMsgCallBack : public AcceptorCallBack
{
public:

	//!default constructor is empty since class provides only protocol no implementation
	EBCDICEchoMsgCallBack() : fLastRequest(), fReplyMessage()	{ }

	//!default destructor is empty since class provides only protocol no implementation
	virtual ~EBCDICEchoMsgCallBack() { }

	//!call back functionality provides the socket object
	//! \param socket the Socket object resulting from the accepted connection
	virtual void CallBack(Socket *socket);

	//!I return the last message I read
	virtual String &GetLastRequest();

	//!Set the message that I shall return on subsequent requests
	void SetReplyMessage( String &reply);

protected:
	// no locking as only one thread initiates actions and the second thread
	// only accesses while the first one waits for I/O
	String fLastRequest;
	String fReplyMessage;
};

//!a thread to run a specific acceptor
//! very simple server thread that allows one connection, useful for testing
//! Sockets
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

//---- ConnectorAcceptorTest ---------------------------------------------------

class ConnectorAcceptorTest : public TestCase
{
public:
	ConnectorAcceptorTest(TString tstrName);
	virtual ~ConnectorAcceptorTest();

	virtual void setUp ();
	virtual void tearDown ();
	static Test *suite ();
	void basicOperation ();
	void basicOperationWithAllocator ();
	void differentReply ();

protected:
	EBCDICEchoMsgCallBack *fCallBack;
	AcceptorThread *fAcceptorThread;
};
#endif
