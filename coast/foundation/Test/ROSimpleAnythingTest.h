/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _ROSimpleAnythingTest_H
#define _ROSimpleAnythingTest_H

//---- baseclass include -------------------------------------------------
#include "TestCase.h"

//---- forward declaration -----------------------------------------------
#include "config_foundation.h"	// for definition of EXPORTDECL_FOUNDATION
class EXPORTDECL_FOUNDATION ROAnything;

//---- ROAnythingSimpleTest ----------------------------------------------------------
//!TestCases for the ROAnything class Simple types part
class ROAnythingSimpleTest : public TestCase
{
public:
	//--- constructors

	//!TestCases for the Socket classes
	//! \param name name of the test
	ROAnythingSimpleTest(TString name);

	//!destroys the test case
	~ROAnythingSimpleTest();

	//--- public api

	//!builds up a suite of testcases for this test
	static Test *suite ();

	//!sets the environment for this test
	void setUp ();

	//!deletes the environment for this test
	void tearDown ();

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
