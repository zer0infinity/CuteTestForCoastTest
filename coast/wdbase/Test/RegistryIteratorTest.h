/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _RegistryIteratorTest_H
#define _RegistryIteratorTest_H

//---- baseclass include -------------------------------------------------
#include "TestCase.h"

//---- RegistryIteratorTest ----------------------------------------------------------
//!TestCases description
class RegistryIteratorTest : public TestFramework::TestCase
{
public:
	//--- constructors

	//!TestCase constructor
	//! \param name name of the test
	RegistryIteratorTest(TString tstrName);

	//!destroys the test case
	~RegistryIteratorTest();

	//--- public api

	//!builds up a suite of testcases for this test
	static Test *suite ();

	//!tests iteration over null registry
	void IterateOverNullRegistry();

	//!tests iteration over empty registry
	void IterateOverEmptyRegistry();

	//!tests iteration over empty registry
	void IterateOverOneElementRegistry();

	//!tests iteration over empty registry
	void IterateOverNElementRegistry();

	//!tests iteration over empty registry
	void IterateWhileRemoving();

protected:

};

#endif
