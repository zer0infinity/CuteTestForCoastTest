/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _ConfigMapperTest_H
#define _ConfigMapperTest_H

//---- baseclass include -------------------------------------------------
#include "FoundationTestTypes.h"

//---- ConfigMapperTest ----------------------------------------------------------
//! <B>single line description of the class</B>
/*!
further explanation of the purpose of the class
this may contain <B>HTML-Tags</B>
*/
class ConfigMapperTest : public TestFramework::TestCaseWithConfig
{
public:
	//--- constructors

	/*! \param name name of the test and its configuration file */
	ConfigMapperTest(TString tstrName);

	//! destroys the test case
	~ConfigMapperTest();

	//--- public api

	//! builds up a suite of tests
	static Test *suite ();

	TString getConfigFileName();

	//! testing config mapper
	void ConfigTest();
};

#endif
