/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _ConnectorArgsTest_H
#define _ConnectorArgsTest_H

#include "TestCase.h"//lint !e537

//! <B>single line description of the class</B>
/*!
further explanation of the purpose of the class
this may contain <B>HTML-Tags</B>
*/
class ConnectorArgsTest : public testframework::TestCase
{
public:
	//--- constructors

	/*! \param name name of the test and its configuration file */
	ConnectorArgsTest(TString tstrName);

	//! destroys the test case
	~ConnectorArgsTest();

	//--- public api

	//! builds up a suite of tests
	static Test *suite ();

	//! describe this Test
	void ArgsTest();
};

#endif
