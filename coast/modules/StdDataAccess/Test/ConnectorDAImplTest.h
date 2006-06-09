/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _ConnectorDAImplTest_H
#define _ConnectorDAImplTest_H

//---- baseclass include -------------------------------------------------
#include "WDBaseTestPolicies.h"
#include "Socket.h"
#include "Threads.h"

class EchoMsgCallBack;
class AcceptorThread;
//---- ConnectorDAImplTest ----------------------------------------------------------
//! <B>really brief class description</B>
/*!
further explanation of the purpose of the class
this may contain <B>HTML-Tags</B>
*/
class ConnectorDAImplTest : public TestFramework::TestCaseWithGlobalConfigDllAndModuleLoading
{
public:
	//--- constructors

	/*! \param name name of the test */
	ConnectorDAImplTest(TString tname);
	//! destroys the test case
	~ConnectorDAImplTest();

	//--- public api

	//! builds up a suite of testcases for this test
	static Test *suite ();
	//! sets the environment for this test
	void setUp ();
	//! deletes the environment for this test
	void tearDown ();

	//! describe this testcase
	void SendReceiveOnceTest();
	void RecreateSocketTest();

	TString getConfigFileName();

	bool IsSocketValid(int socketFd);

protected:
	EchoMsgCallBack *fCallBack;
	AcceptorThread *fAcceptorThread;
};

#endif
