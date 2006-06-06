/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _AppBooterTest_H
#define _AppBooterTest_H

//---- baseclass include -------------------------------------------------
#include "TestCase.h"

class Anything;

//---- AppBooterTest ----------------------------------------------------------
//!TestCases description
class AppBooterTest : public TestFramework::TestCase
{
public:
	//--- constructors

	//!TestCase constructor
	//! \param name name of the test
	AppBooterTest(TString tstrName);

	//!destroys the test case
	~AppBooterTest();

	//--- public api

	//!builds up a suite of testcases for this test
	static Test *suite ();

	//!test HandleArgs method with empty arguments
	void HandleNullArgsTest();

	//!test HandleArgs method with simple arguments
	void HandleUnstructuredArgsTest();

	//!test HandleArgs method with complex arguments
	void HandleStructuredArgsTest();

	//!test Bootfile loading parameters
	void PrepareBootFileLoadingTest();

	//!test merging of configuration with commandline arguments
	void MergeConfigWithArgsTest();

	//!test opening of libraries
	void OpenLibsTest();

	//!tests sequencing of initialization and termination of application api
	void RunTest();

	void Method(const String &str);

protected:
	Anything fSequence;
	long fStep;
};

#endif
