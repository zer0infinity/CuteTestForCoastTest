/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _LFListenerPoolTest_H
#define _LFListenerPoolTest_H

#include "WDBaseTestPolicies.h"

class Socket;

class LFListenerPoolTest : public testframework::TestCaseWithGlobalConfigDllAndModuleLoading
{
public:
	//--- constructors

	//!TestCase constructor
	//! \param name name of the test
	LFListenerPoolTest(TString tstrName);

	//!destroys the test case
	~LFListenerPoolTest();

	//--- public api

	//!builds up a suite of testcases for this test
	static Test *suite ();

	//!sets the environment for this test
	void setUp ();

	//!describe this testcase
	void NoFactoryTest();

	//!test pool without acceptors
	void NoReactorTest();

	//!test pool with one acceptor
	void OneAcceptorTest();

	//!test pool with two acceptors
	void TwoAcceptorsTest();

	//!test pool with two acceptors
	void ManyAcceptorsTest();

	//!test pool with one invalid acceptor
	void InvalidAcceptorTest();

	//!test pool with one invalid reactor
	void InvalidReactorTest();

	//!processor callback
	bool EventProcessed(Socket *);

protected:
	//!client side api to call into pool from test
	virtual void ProcessOneEvent();
	//!client side api to call into pool from test
	virtual void ProcessTwoEvents();
	//!client side api to call into pool from test
	virtual void ProcessManyEvents();
};

#endif
