/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _RegistryTest_H
#define _RegistryTest_H

#include "TestCase.h"

//---- forward declaration -----------------------------------------------
class Registry;

//---- RegistryTest ----------------------------------------------------------
//!TestCases for the Registry class
class RegistryTest : public TestFramework::TestCase
{
public:
	//--- constructors

	//!TestCases for the Registry classes
	//! \param name name of the test
	RegistryTest(TString tstrName);

	//!destroys the test case
	~RegistryTest();

	//--- public api

	//!builds up a suite of testcases for this test
	static Test *suite ();

	//!sets the environment for this test
	void setUp ();
	void tearDown();

protected:
	void	Constructor ();
	void	GetRegistry ();
	void	InstallAliases ();
	void	InstallErroneousAliases ();
	void	InstallHierarchy ();
	void	InstallHierarchyConfig ();
	void	TerminateTest ();

	Registry *fRegistry;
};

#endif
