/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _DateTimeTest_H
#define _DateTimeTest_H

#include "FoundationTestTypes.h"

//---- DateTimeTest ----------------------------------------------------------
//! <B>single line description of the class</B>
/*!
further explanation of the purpose of the class
this may contain <B>HTML-Tags</B>
*/
class DateTimeTest : public testframework::TestCaseWithConfig
{
public:
	//--- constructors

	/*! \param name name of the test and its configuration file */
	DateTimeTest(TString tstrName);

	//! destroys the test case
	~DateTimeTest();

	//--- public api

	//! builds up a suite of tests
	static Test *suite ();

	TString getConfigFileName();

	//! describe this Test
	void GetTimeZoneTest();
};

#endif
