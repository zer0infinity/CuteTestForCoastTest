/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _SessionTest_H
#define _SessionTest_H

#include "WDBaseTestPolicies.h"
#include "Context.h"

class STTestSession;

class SessionTest: public testframework::TestCaseWithGlobalConfigDllAndModuleLoading {
public:
	SessionTest(TString tstrName) :
		TestCaseType(tstrName) {
	}
	static Test *suite();
	void setUp();
	//!test usage of session store with context
	void UseSessionStoreTest();

	//!test Session::Verify method
	void VerifyTest();

	//!test the sessions info functionality
	void InfoTest();

	//!Checks the removability of a session
	void IsDeletableTest();

	//!Checks the context initialization
	void SetupContextTest();

	//!test method CheckRoleExchange
	void CheckRoleExchangeTest();

protected:
	void CheckInstalled();
	void TestInit();
	void RenderNextPageError();
	void RenderNextPageInsertAPage();
	void DoFindNextPageLogin();
	void SetGetRole();
	void RetrieveFromDelayed();
	void IntCheckRoleExchange(const char *source_role, const char *target_role, const char *transition, STTestSession &s, Context &theCtx,
			bool should_succeed);
};

#endif

