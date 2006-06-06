/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _HTTPStreamStackTest_H
#define _HTTPStreamStackTest_H

//---- baseclass include -------------------------------------------------
#include "TestCase.h"

//---- HTTPStreamStackTest ----------------------------------------------------------
//! TestCases description
class HTTPStreamStackTest : public TestFramework::TestCase
{
public:
	//--- constructors

	//! TestCase constructor
	//! \param name name of the test
	HTTPStreamStackTest(TString tstrName);

	//! destroys the test case
	~HTTPStreamStackTest();

	//--- public api

	//! builds up a suite of testcases for this test
	static Test *suite ();

	//! Test with no special encoding
	void NoEncoding();

	//! Test with chunked encoding only
	void ChunkedOnlyEncoding();

	//! Test with gzip encoding only
	void GzipOnlyEncoding();

	//! Test with chunked and gzip encoding
	void ChunkedGzipEncoding();
};

#endif
