/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _ErrorHandlerTest_H
#define _ErrorHandlerTest_H

//---- baseclass include -------------------------------------------------
#include "WDBaseTestPolicies.h"
#include "Mapper.h"

//---- ErrorHandlerTest ----------------------------------------------------------
//! <B>really brief class description</B>
/*!
further explanation of the purpose of the class
this may contain <B>HTML-Tags</B>
*/
class ErrorHandlerTest : public TestFramework::TestCaseWithGlobalConfigDllAndModuleLoading
{
public:
	//--- constructors

	/*! \param name name of the test */
	ErrorHandlerTest(TString tstrName);

	//! destroys the test case
	~ErrorHandlerTest();

	//--- public api

	//! builds up a suite of testcases for this test
	static Test *suite ();

	//! sets the environment for this test
	void setUp ();

	//! deletes the environment for this test
	void tearDown ();

	void HandleErrorTest();
	void HandleConnectionErrorTest();
	void ParamAccessTest();
	void ShouldRetryTest();

private:
	Context *fCtx;
	ParameterMapper *fGet;
	ResultMapper *fPut;
};

#endif
