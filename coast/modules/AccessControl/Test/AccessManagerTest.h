/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _AccessManagerTest_H
#define _AccessManagerTest_H

//---- baseclass include -------------------------------------------------
#include "ConfiguredTestCase.h"

class AccessManager;

//---- AccessManagerTest ----------------------------------------------------------
//! <B>Test access managers with the default API.</B>
/*!
This test can test any access manager that exports the default API.
It runs configured unit tests for the following API functions:
<i>Validate, AuthenticateWeak, AuthenticateStrong, ChangePassword,
ResetPassword, IsAllowed</i> and <i>GetAllowed</i>.
AccessManagers are defined and registered in the config of the wdapp.
They have a config themselves, which in turn defines the backends
(i.e. AccessControllers) that they utilize to get access to user-
and token information.
*/
class AccessManagerTest : public ConfiguredTestCase
{
public:
	//--- constructors

	//! TestCase constructor
	//! \param name name of the test
	AccessManagerTest(TString name);

	//! destroys the test case
	~AccessManagerTest();

	//--- public api

	//! builds up a suite of testcases for this test
	static Test *suite ();

	//! sets the environment for this test
	void setUp ();

	//! deletes the environment for this test
	void tearDown ();

	// tests an access manager
	void doTestAccessManager(ROAnything config, AccessManager *am);

	// tests all regular access managers defined/registered in the config
	void testRegularAccessManagers();
};

#endif
