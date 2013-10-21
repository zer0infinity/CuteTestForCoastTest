/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _HTTPChunkedOStreamTest_H
#define _HTTPChunkedOStreamTest_H

#include "TestCase.h"

class HTTPChunkedOStreamTest : public testframework::TestCase
{
public:
	//--- constructors

	//! TestCase constructor
	//! \param name name of the test
	HTTPChunkedOStreamTest(TString tstrName);

	//! destroys the test case
	~HTTPChunkedOStreamTest();

	//--- public api

	//! builds up a suite of testcases for this test
	static Test *suite ();

	//! make a small chunk size and overflow it
	void OverflowTest();
	//! just close the stream without a flush
	void SimpleCloseNoFlush();
	//! test the last empty chunk
	void SimpleWithClose();
	//! create a HTTPChunkedOStream and try successful small output
	void SimpleFlush();
	//! test if the hex manipulator is switched off after chunk header
	void HexManipulator();
};

#endif
