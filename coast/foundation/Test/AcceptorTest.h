/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _AcceptorTest_H
#define _AcceptorTest_H

//---- baseclass include -------------------------------------------------
#include "TestCase.h"

//---- forward declaration -----------------------------------------------
#include "config_foundation.h"	// for definition of EXPORTDECL_FOUNDATION
class EXPORTDECL_FOUNDATION Acceptor;
class EXPORTDECL_FOUNDATION Socket;
#include "Anything.h"
//---- AcceptorTest ----------------------------------------------------------
//!TestClass for Acceptor functionality
//! further explanation of the purpose of the class
//! this may contain <B>HTML-Tags</B>
//! ...
class AcceptorTest : public TestCase
{
public:

	//--- constructors
	AcceptorTest(TString name);
	~AcceptorTest();

	//--- public api
	//!generate Acceptor test suite
	//! this method generates the tests for the Acceptor classed
	//! \return a new test is created by this method
	static Test *suite ();

	//!generate set up for connector test does nothing so far
	void setUp ();

	//!delete set up for connector test does nothing so far
	void tearDown ();

	//!tests  resolution of parameter as dns name to ipaddress
	void dnsNameParam();

	//!tests setting of ephemeral port by os
	void getPort();

	//!tests accept once functionality single threaded
	//! caution don't try to use accept loop single threaded without once flag
	void acceptOnceTest();

protected:
	friend class TestCallBack;
	void TestSocket(Socket *s);

	Anything fConfig;
};

#endif
