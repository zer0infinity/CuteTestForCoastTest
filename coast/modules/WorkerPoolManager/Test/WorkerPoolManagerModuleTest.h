/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _WorkerPoolManagerModuleTest_H
#define _WorkerPoolManagerModuleTest_H

//---- baseclass include -------------------------------------------------
#include "ConfiguredTestCase.h"
#include "WorkerPoolManagerModule.h"

//---- WorkerPoolManagerModuleTest ----------------------------------------------------------
//! <B>single line description of the class</B>
/*!
further explanation of the purpose of the class
this may contain <B>HTML-Tags</B>
*/
class WorkerPoolManagerModuleTest : public ConfiguredTestCase
{
public:
	//--- constructors

	/*! \param name name of the test and its configuration file */
	WorkerPoolManagerModuleTest(TString tstrName);

	//! destroys the test case
	~WorkerPoolManagerModuleTest();

	//--- public api

	//! builds up a suite of ConfiguredTestCases for this test
	static Test *suite ();

	//! sets the environment for this test
	void setUp ();

	//! deletes the environment for this test
	void tearDown ();

	//! describe this Test
	void test();
	void TestWorkerOne();
	void TestWorkerTwo();
	void TestReset();
private:
	void Check(ROAnything cConfig, WorkerPoolManagerModule *pModule);
};

#endif
