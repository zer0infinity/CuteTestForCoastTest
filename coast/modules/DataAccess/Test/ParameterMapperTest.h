/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _ParameterMapperTest_H
#define _ParameterMapperTest_H

//---- baseclass include -------------------------------------------------
#include "TestCase.h"

//---- ParameterMapperTest ----------------------------------------------------------
//! <B>Tests Functionality of ParameterMapper and EagerParameterMapper.</B>
/*!
To understand the results of those tests, you should additionally
consult "InputMapperMeta.any" and "MapperTestScripts.any".
*/

class ParameterMapperTest : public TestFramework::TestCase
{
public:
	//--- constructors

	/*! \param name name of the test */
	ParameterMapperTest(TString tstrName);

	//! destroys the test case
	~ParameterMapperTest();

	//--- public api

	//! builds up a suite of testcases for this test
	static Test *suite ();

	void DoSelectScriptTest();
	void DoLoadConfigTest();
	void DoGetConfigNameTest();
	void PlaceIntoAnyOrAppendIfNotEmptyTest();
	void DoFinalGetAnyTest();
	void DoFinalGetStreamTest();
	void DoGetAnyTest();
	void DoGetStreamTest();
	void GetTest();
	void DoSetSourceSlotDynamicallyTest();
	void DoGetSourceSlotWithPathTest();

	void EagerDoSelectScriptTest();
	void EagerGetTest();
};

#endif
