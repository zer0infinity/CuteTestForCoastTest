/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _SimpleListenerPoolTest_H
#define _SimpleListenerPoolTest_H

//---- baseclass include -------------------------------------------------
#include "ListenerPoolTest.h"

//---- SimpleListenerPoolTest -------------------------------------------------
//!TestCases description
class SimpleListenerPoolTest : public ListenerPoolTest
{
public:
	//--- constructors

	//!TestCase constructor
	//! \param name name of the test
	SimpleListenerPoolTest(TString name);

	//!destroys the test case
	~SimpleListenerPoolTest();

	//--- public api
	virtual void PoolTest();

	//!builds up a suite of testcases for this test
	static Test *suite ();

protected:
	virtual void DoTestConnect();
};

#endif
