/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _ROSimpleAnythingTest_H
#define _ROSimpleAnythingTest_H

#include "TestCase.h"//lint !e537

class ROSimpleAnythingTest : public TestFramework::TestCase
{
public:
	//--- constructors

	//!TestCases for the Socket classes
	//! \param name name of the test
	ROSimpleAnythingTest(TString tstrName);

	//!destroys the test case
	~ROSimpleAnythingTest();

	//--- public api

	//!builds up a suite of testcases for this test
	static Test *suite ();

	// Testcases
	void ConstructorsAndConversions();
	void AssignmentsAndConversions();

	void EmptyConstructor();
	void AnyConstructor();
	void AnyIntConstructor();
	void AnyBoolConstructor();
	void AnyLongConstructor();
	void AnyFloatConstructor();
	void AnyDoubleConstructor();
	void AnyIFAObjConstructor();
};

#endif		//not defined _ROSimpleAnythingTest_H
