/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _URLFilterTest_H
#define _URLFilterTest_H

//---- baseclass include -------------------------------------------------
#include "ConfiguredTestCase.h"

//---- forward declaration -----------------------------------------------

//---- URLFilterTest ----------------------------------------------------------
//!TestCases for the Registry class
class URLFilterTest : public ConfiguredTestCase
{
public:
	//--- constructors

	//!TestCases for the Registry classes
	//! \param name name of the test
	URLFilterTest(TString name);

	//!destroys the test case
	~URLFilterTest();

	//--- public api

	//!builds up a suite of testcases for this test
	static Test *suite ();

	//!sets the environment for this test
	void setUp ();

	//!deletes the environment for this test
	void tearDown ();

	// handle simple things first
	void DoFilterStateTest();

	// handle simple things first
	void DoUnscrambleStateTest();

	void FilterStateTest();
	void UnscrambleStateTest();

	void HandleCookieTest();
	void HandleQueryTest();

protected:

	//--- member variables declaration

private:
	// use careful, you inhibit subclass use
	//--- private class api

	//--- private member variables

};

#endif
