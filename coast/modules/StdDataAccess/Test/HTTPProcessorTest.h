/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _HTTPProcessorTest_H
#define _HTTPProcessorTest_H

//---- baseclass include -------------------------------------------------
#include "WDBaseTestPolicies.h"

//---- HTTPProcessorTest ----------------------------------------------------------
//!Test the HTTPProcessor
class HTTPProcessorTest : public TestFramework::TestCaseWithGlobalConfigDllAndModuleLoading
{
public:
	//!TestCase constructor
	//! \param name name of the test
	HTTPProcessorTest(TString tstrName);

	//!builds up a suite of testcases for this test
	static Test *suite ();

	TString getConfigFileName();

	void IsZipEncodingAcceptedByClientTest();

	//!tests input read method
	void DoReadInputWithErrorTest();

	//!check multipart handling if content-disposition contains filename
	void FileUploadTest();

	//! Check HTTP status code and message.
	//! new variant with slot "HTTPStatusCode" and "HTTPReasonPhrase"
	void RenderProtocolStatusWithoutHTTPStatus();

	//! KeepConnection must be set for HTTP 1.1 with Connection=keep-alive
	void KeepConnection();
};

#endif
