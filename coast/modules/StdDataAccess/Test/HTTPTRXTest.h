/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _HTTPTRXTest_H
#define _HTTPTRXTest_H

//---- baseclass include -------------------------------------------------
#include "TestCase.h"

//---- forward declaration -----------------------------------------------
class HTTPTRX;

//---- HTTPTRXTest ----------------------------------------------------------
//!TestClass for for HTMLParser functionality
//! further explanation of the purpose of the class
//! this may contain <B>HTML-Tags</B>
//! ...
class HTTPTRXTest : public TestCase
{
public:

	//--- constructors
	HTTPTRXTest(TString name);
	~HTTPTRXTest();

	//--- public api
	//!generate HTMLParser test suite
	//! this method generates the tests for the HTMLParser classed
	//! \return a new test is created by this method
	static Test *suite ();

	//!generate set up for connector test does nothing so far
	void setUp ();

	//!delete set up for connector test does nothing so far
	void tearDown ();

protected:
	//--- tests for public api of class connector

	//!tests htmlparser with a file from config
	void fdPutArgsTest();
	void webAppPutArgsTest();

private:
	// use careful, you inhibit subclass use
	//--- private class api

	//--- private member variables

};

//---- HTTPDAImplTest ----------------------------------------------------------
//!TestClass for for HTMLParser functionality
//! further explanation of the purpose of the class
//! this may contain <B>HTML-Tags</B>
//! ...
class HTTPDAImplTest : public TestCase
{
public:

	//--- constructors
	HTTPDAImplTest(TString name);
	~HTTPDAImplTest();

	//--- public api
	//!generate HTMLParser test suite
	//! this method generates the tests for the HTMLParser classed
	//! \return a new test is created by this method
	static Test *suite ();

	//!generate set up for connector test does nothing so far
	void setUp ();

	//!delete set up for connector test does nothing so far
	void tearDown ();

protected:
	//--- tests for public api of class connector

	//!tests htmlparser with a file from config
	void useSSLTest();

	//!tests htmlparser with a file from config
	void ErrorHandlingTest();
};

#endif
