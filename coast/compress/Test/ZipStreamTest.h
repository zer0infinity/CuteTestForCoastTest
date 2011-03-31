/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _ZipStreamTest_H
#define _ZipStreamTest_H

#include "FoundationTestTypes.h"

//---- ZipStreamTest ----------------------------------------------------------
//! TestCases description
class ZipStreamTest : public TestFramework::TestCaseWithConfig
{
public:
	//!TestCase constructor
	//! \param name name of the test
	ZipStreamTest(TString tstrName);

	//!destroys the test case
	~ZipStreamTest();

	//!builds up a suite of testcases for this test
	static Test *suite ();

	//!test the gzip header after init
	void GzipHdrWriteTest();
	//!test the gzip file creation and reading
	void GzipSimpleFileCheck();
	//!test empty file
	void GzipEmptyFile();
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
	//!read empty file using getline()
	void StringEmptyFileGetlineTest();

	void GzipHdrTest();
	void ReadGzipHdrFileTest();
	void ReadHeaderInfoTest();
	void WriteHeaderInfoTest();
	void CompressionModeTest();
	void SetCompressionTest();

private:
	void VerifyFile(const char *fileName);
};

#endif
