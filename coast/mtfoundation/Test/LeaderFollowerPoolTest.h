/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _LeaderFollowerPoolTest_H
#define _LeaderFollowerPoolTest_H

//---- baseclass include -------------------------------------------------
#include "TestCase.h"
#include "Anything.h"

class Socket;

//---- LeaderFollowerPoolTest ----------------------------------------------------------
//!tests LeaderFollowerPool
class LeaderFollowerPoolTest : public TestCase
{
public:
	//--- constructors

	//!TestCase constructor
	//! \param name name of the test
	LeaderFollowerPoolTest(TString name);

	//!destroys the test case
	~LeaderFollowerPoolTest();

	//--- public api

	//!builds up a suite of testcases for this test
	static Test *suite ();

	//!sets the environment for this test
	void setUp ();

	//!deletes the environment for this test
	void tearDown ();

	//!test pool without acceptors
	void NoReactorTest();

	//!test pool without acceptors
	void NoAcceptorsTest();

	//!test pool with one acceptor
	void OneAcceptorTest();

	//!test pool with two acceptors
	void TwoAcceptorsTest();

	//!test pool with two acceptors
	void ManyAcceptorsTest();

	//!test pool with one invalid acceptor
	void InvalidAcceptorTest();

	//!test pool with one invalid acceptor but several configured
	void InvalidAcceptorsTest();

	//!connect to one acceptor and process event
	virtual void ProcessOneEvent();

	//!connec to two acceptors and process events
	void ProcessTwoEvents();

	//!connect to several acceptors and process events
	void ProcessManyEvents();

	//!reactor callback
	virtual bool EventProcessed(Socket *);

//	void FindHandleTest();

protected:
	long fEvents;
	Anything	fConfig;
};

#endif
