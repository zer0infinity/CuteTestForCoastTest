/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _SimpleDAServiceTest_H
#define _SimpleDAServiceTest_H

//---- baseclass include -------------------------------------------------
#include "TestCase.h"

//---- SimpleDAServiceTest ----------------------------------------------------------
//!TestCases description
class SimpleDAServiceTest : public TestCase
{
public:
	//--- constructors

	//!TestCase constructor
	//! \param name name of the test
	SimpleDAServiceTest(TString tstrName);

	//!destroys the test case
	~SimpleDAServiceTest();

	//--- public api

	//!builds up a suite of testcases for this test
	static Test *suite ();

	//!sets the environment for this test
	void setUp ();

	//!deletes the environment for this test
	void tearDown ();

	//!use the ServiceDispatcher to obtain a simple service
	void SimpleDispatch();
	//! try to use it with a very simple DataAccess backend
	void SimpleServiceCall();
	//! try to use it with a missing DataAccess backend
	void FailedServiceCall();

protected:

};

#endif
