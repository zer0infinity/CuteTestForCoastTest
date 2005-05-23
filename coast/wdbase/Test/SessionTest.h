/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _SessionTest_H
#define _SessionTest_H

//---- baseclass include -------------------------------------------------
#include "ConfiguredTestCase.h"
#include "Context.h"

//---- forward declaration -----------------------------------------------
class STTestSession;

//---- SessionTest ----------------------------------------------------------
//!TestCases for the Session class
class SessionTest : public ConfiguredTestCase
{
public:
	//--- constructors

	//!TestCases for the Session class
	//! \param name name of the test
	SessionTest(TString tstrName);

	//!destroys the test case
	~SessionTest();

	//--- public api

	//!builds up a suite of testcases for this test
	static Test *suite ();

	//!sets the environment for this test
	void setUp ();

	//!deletes the environment for this test
	void tearDown ();

	//!test usage of session store with context
	void    UseSessionStoreTest();

	//!test Session::Verify method
	void VerifyTest ();

	//!test the sessions info functionality
	void InfoTest();

	//!Checks the removability of a session
	void IsDeletableTest();

	//!Checks the context initialization
	void SetupContextTest();

	//!test method CheckRoleExchange
	void CheckRoleExchangeTest();

protected:

	void	CheckInstalled ();
	void    TestInit();
	void    RenderNextPageError();
	void    RenderNextPageInsertAPage();
	void    DoFindNextPageLogin();
	void    SetGetRole();
	void    RetrieveFromDelayed();
	void    IntCheckRoleExchange(char *source_role, char *target_role, char *transition, STTestSession &s, Context &theCtx, bool should_succeed);
	//--- member variables declaration

};

#endif

