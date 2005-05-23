/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _SlotnameOutputMapperTest_H
#define _SlotnameOutputMapperTest_H

//---- baseclass include -------------------------------------------------
#include "ConfiguredActionTest.h"

//---- SlotnameOutputMapperTest ----------------------------------------------------------
//!Test the SlotnameOutputMapper
class SlotnameOutputMapperTest : public ConfiguredActionTest
{
public:
	//--- constructors

	//!ConfiguredTestCase constructor
	//! \param name name of the test
	SlotnameOutputMapperTest(TString tstrName);

	//!destroys the test case
	~SlotnameOutputMapperTest();

	//--- public api

	//!builds up a suite of ConfiguredTestCases for this test
	static Test *suite ();

	//!Performs a mapping and checks the Stores
	void BasicFunctionTest();
	//!Performs mapping test for overwrite or append and checks the Stores
	void OverwriteOrAppendTest();
};

#endif
