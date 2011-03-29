/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _AuthenticationServiceTest_H
#define _AuthenticationServiceTest_H

#include "ConfiguredActionTest.h"

class Context;
class ServiceHandler;
//---- AuthenticationServiceTest ----------------------------------------------------------
//!ConfiguredTestCases description
class AuthenticationServiceTest: public ConfiguredActionTest {
public:
	AuthenticationServiceTest(TString tstrName) :
		ConfiguredActionTest(tstrName) {
	}

	TString getConfigFileName() {
		return "AuthenticationServiceTestConfig";
	}

	//!builds up a suite of tests
	static Test *suite();

	//!Authentication is Ok
	void OkTest();
	void OkTest1();
	//!Authentication fails
	void AuthenticationFailedTest();
	//!User is ok , but service is not configured well
	void ServiceNotFoundTest();
	//!service is not configured at all
	void NoConfigTest();

protected:
	void ConnCloseMessage(String &strMsg, String strRealm, String strOutput);
	void DoTest(ServiceHandler &sh, Context &ctx, String expectedMsg);
	void DoTest(ServiceHandler &sh, Context &ctx, Anything expectedAny);
	//! utility method to set user info into Context
	//! \post Env in ctx has AUTHENTICATION header with encoded user / pw
	virtual void MakeAuthenticationInfo(Context &ctx, String user, String pw);
};

#endif
