/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _LDAPMapperTest_H
#define _LDAPMapperTest_H

//---- TestCase include -------------------------------------------------
#include "TestCase.h"

#if defined(__sun) || defined(WIN32) || defined(__linux__)

//---- forward declaration -----------------------------------------------

//---- LDAPMapperTest ----------------------------------------------------------
//!single line description of the class
//! further explanation of the purpose of the class
//! this may contain <B>HTML-Tags</B>
//! ...
class LDAPMapperTest : public TestCase
{
public:
	//--- constructors
	LDAPMapperTest(TString tstrName);
	~LDAPMapperTest();

	//--- public api
	//!generate LDAPMapper test suite
	//! this method generates the tests for the LDAPMapper classes
	//! \return a new test is created by this method
	static Test *suite ();

	//!generate set up for connector test does nothing so far
	void setUp ();

	//!delete set up for connector test does nothing so far
	void tearDown ();

protected:
	//--- tests for public api of class connector

	//!tests the mapper which generates a query filter string
	void QueryFilterTest();

	//!tests the mapper which generates the basic LDAP parameters as base, timeout, sizelimit etc.
	void BasicParamTest();

	//!tests the mapper which generates a distinguished name (e.g. used in a compare dataaccess)
	void DNameTest();

	//!tests the mapper which encodes the inputvalue into a securehashed and base64 encoded value
	void FieldMapperTest1();

	//!tests the mapper which encodes the inputvalue to base64
	void FieldMapperTest2();

	//!tests the mapper which generates the modify value list
	void ModifyValsTest();

	//!tests the mapper which special dname settings
	void LDAPAddTest();

	//--- member variables declaration
private:
	// use careful, you inhibit subclass use
	//--- private class api

	//--- private member variables

};

#endif

#endif
