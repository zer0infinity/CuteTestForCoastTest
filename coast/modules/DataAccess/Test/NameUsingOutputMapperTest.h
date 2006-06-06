/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _NameUsingOutputMapperTest_H
#define _NameUsingOutputMapperTest_H

//---- baseclass include -------------------------------------------------
#include "TestCase.h"

class ROAnything;
class Context;
class ResultMapper;
//---- NameUsingOutputMapperTest ----------------------------------------------------------
//!Tests the NameUsingOutputMapper
class NameUsingOutputMapperTest : public TestFramework::TestCase
{
public:
	//--- constructors

	//!TestCase constructor
	//! \param name name of the test
	NameUsingOutputMapperTest(TString tstrName);

	//!destroys the test case
	~NameUsingOutputMapperTest();

	//--- public api

	//!builds up a suite of testcases for this test
	static Test *suite ();

	//!tests if the data is put into the configured Destination
	void ConfiguredDestinationTest();

	//!tests if the data is put into the tempstore under the name of the Mapper
	void NonConfiguredDestinationTest();

protected:
	void DoPut(ResultMapper &mapper, Context &c);
	void DoCheck(const ROAnything &result);
};

#endif
