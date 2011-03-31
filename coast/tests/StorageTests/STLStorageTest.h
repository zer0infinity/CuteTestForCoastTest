/*
 * Copyright (c) 2007, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _STLStorageTest_H
#define _STLStorageTest_H

#include "TestCase.h"

//---- STLStorageTest ----------------------------------------------------------
//! <B>really brief class description</B>
/*!
further explanation of the purpose of the class
this may contain <B>HTML-Tags</B>
*/
class STLStorageTest : public TestFramework::TestCase
{
public:
	//--- constructors

	/*! \param tstrName name of the test */
	STLStorageTest(TString tstrName);

	//! destroys the test case
	~STLStorageTest();

	//--- public api

	//! builds up a suite of testcases for this test
	static Test *suite ();

// uncomment if really needed or delete, base class does almost everything
//	//! sets the environment for this test
//	void setUp ();
//
//	//! deletes the environment for this test
//	void tearDown ();

	//! describe this testcase
	void GlobalStorageTest();
	void PoolStorageTest();
	void AllocatorUsingSMartPtrTest();
};

#endif
