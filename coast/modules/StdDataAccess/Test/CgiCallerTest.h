/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _CgiCallerTest_H
#define _CgiCallerTest_H

//---- baseclass include -------------------------------------------------
#include "TestCase.h"
class Context;
//---- CgiCallerTest ----------------------------------------------------------
//!Tests the cgi caller
class CgiCallerTest : public TestCase
{
public:
	//--- constructors

	//!TestCase constructor
	//! \param name name of the test
	CgiCallerTest(TString name);

	//!destroys the test case
	~CgiCallerTest();

	//--- public api

	//!builds up a suite of testcases for this test
	static Test *suite ();

	//!call a CGI program created with PrepareScript
	void ExecOkTests();
	void TestSplitPath();

protected:
	//!call a CGI program created with the context
	void DoExecTest(Context &ctx);

};

#endif
