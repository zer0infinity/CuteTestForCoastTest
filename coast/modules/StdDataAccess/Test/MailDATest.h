/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _MailDATest_H
#define _MailDATest_H

//---- baseclass include -------------------------------------------------
#include "ConfiguredActionTest.h"

//---- MailDATest ----------------------------------------------------------
//!TestCases description
class MailDATest : public ConfiguredActionTest
{
public:
	//--- constructors

	//!TestCase constructor
	//! \param name name of the test
	MailDATest(TString tstrName);

	//!destroys the test case
	~MailDATest();

	//--- public api

	//!builds up a suite of testcases for this test
	static Test *suite ();

protected:

};
#endif
