/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _HTMLCacheLoaderTest_H
#define _HTMLCacheLoaderTest_H

//---- baseclass include -------------------------------------------------
#include "TestCase.h"

#include "Anything.h"

//---- HTMLCacheLoaderTest ----------------------------------------------------------
//:TestCases description
class HTMLCacheLoaderTest : public TestCase
{
public:
	//--- constructors

	//:TestCase constructor
	//!param: name - name of the test
	HTMLCacheLoaderTest(TString name);

	//!destroys the test case
	~HTMLCacheLoaderTest();

	//--- public api

	//!builds up a suite of testcases for this test
	static Test *suite ();

	//!sets the environment for this test
	void setUp ();

	//!deletes the environment for this test
	void tearDown ();

	//!test simplest cache Loading
	void LoadEmptyCacheTest();
	//!test simple cache Building
	void SimpleBuildCacheTest();
	//!test simple macro cache Building using [[#wd foo ]]
	void SimpleMacroBuildCacheTest();
	//!test simple comment cache Building using <!--#wd Lookup foo -->
	void SimpleCommentBuildCacheTest();
	//!test two comment cache Building using <!--#wd Lookup foo --><!-- hui --><!--#wd Lookup bar -->
	void ConsecutiveCommentBuildCacheTest();

	//!test simple old way of parsing and cache building
//	void BuildCacheOldStyleTest();
	//!check if we really create a cache
	void CheckCacheIsLoaded();
	//!describe this testcase
	void testCase();
protected:
	Anything fGlobalConfig;
};

#endif
