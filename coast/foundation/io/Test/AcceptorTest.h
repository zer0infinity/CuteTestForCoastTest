/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _AcceptorTest_H
#define _AcceptorTest_H

#include "FoundationTestTypes.h"//lint !e537

class Socket;

//---- AcceptorTest ----------------------------------------------------------
//!TestClass for Acceptor functionality
//! further explanation of the purpose of the class
//! this may contain <B>HTML-Tags</B>
//! ...
class AcceptorTest : public TestFramework::TestCaseWithConfig
{
public:
	//--- constructors
	AcceptorTest(TString tstrName);
	~AcceptorTest();

	//--- public api
	//!generate Acceptor test suite
	//! this method generates the tests for the Acceptor classed
	//! \return a new test is created by this method
	static Test *suite ();

	//!tests setting of ephemeral port by os
	void getPort();

	//!tests accept once functionality single threaded
	//! caution don't try to use accept loop single threaded without once flag
	void acceptOnceTest();

protected:
	friend class TestCallBack;
	void TestSocket(Socket *s);
};

#endif
