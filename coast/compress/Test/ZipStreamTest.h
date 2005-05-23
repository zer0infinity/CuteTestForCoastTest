/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _ZipStreamTest_H
#define _ZipStreamTest_H

//---- baseclass include -------------------------------------------------

#include "ITOString.h"
#include "TestCase.h"

//---- ZipStreamTest ----------------------------------------------------------
//! TestCases description
class ZipStreamTest : public TestCase
{
public:
	//--- constructors

	//!TestCase constructor
	//! \param name name of the test
	ZipStreamTest(TString tstrName);

	//!destroys the test case
	~ZipStreamTest();

	//--- public api

	//!builds up a suite of testcases for this test
	static Test *suite ();

	//!sets the environment for this test
	void setUp ();

	//!deletes the environment for this test
	void tearDown ();

	//!test the gzip header after init
	void GzipHeaderCheck();
	//!test the gzip file creation and reading
	void GzipSimpleFileCheck();
	//!test the gzip file creation and reading for a bigger file
	void GzipBigFileCheck();
	//!read a long file, zip and unzip it
	void GzipLongFileCheck();
	//!test with constant output/input buffers
	void GzipConstantBufferCheck();
	//! Corrupted input for ZipIStream
	void GzipCorruptInputCheck();
	//!test the gzip file creation
	void GzipZlibTest();
	//!describe this testcase
	void StringGetlineTest();

private:
	void VerifyFile(const char *fileName);
};

#endif
