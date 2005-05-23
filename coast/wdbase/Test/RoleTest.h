/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _RoleTest_H
#define _RoleTest_H

//---- baseclass include -------------------------------------------------
#include "ConfiguredTestCase.h"

//---- RoleTest ----------------------------------------------------------
//!TestCases for the Registry class
class RoleTest : public ConfiguredTestCase
{
public:
	//--- constructors

	//!TestCases for the Registry classes
	//! \param name name of the test
	RoleTest(TString tstrName);

	//!destroys the test case
	~RoleTest();

	//--- public api

	//!builds up a suite of testcases for this test
	static Test *suite ();

	//!sets the environment for this test
	void setUp ();

	//!deletes the environment for this test
	void tearDown ();

protected:
	void	CheckInstalled ();
	void GetNewPageName ();
	void VerifyLogout ();
	void VerifyLevel ();
	void Synchronize();
	void PrepareTmpStore();
	void CollectLinkState();
	void FindRoleWithDefault();
	void GetDefaultRoleName();
	//--- member variables declaration

private:
	// use careful, you inhibit subclass use
	//--- private class api
	//--- private member variables
};

#endif
