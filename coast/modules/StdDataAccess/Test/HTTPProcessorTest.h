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
#include "ConfiguredTestCase.h"

//--- module under test --------------------------------------------------------
#include "HTTPProcessor.h"

//---- HTTPProcessorTest ----------------------------------------------------------
//!Test the HTTPProcessor
class HTTPProcessorTest : public ConfiguredTestCase
{
public:
	//--- constructors

	//!TestCase constructor
	//! \param name name of the test
	HTTPProcessorTest(TString tstrName);

	//!destroys the test case
	~HTTPProcessorTest();

	//--- public api

	//!builds up a suite of testcases for this test
	static Test *suite ();

	//!sets the environment for this test
	void setUp ();

	void TestIsZipEncodingAcceptedByClient();

	//!tests input read method
	void DoReadInputTest();
	void testDoReadInputWithError();

	//!tests input read method too
	void DoReadMinimalInputTest();

	//!check multipart handling if content-disposition contains filename
	void FileUploadTest();

	//! Check HTTP status code and message.
	//! new variant with slot "HTTPStatusCode" and "HTTPReasonPhrase"
	void RenderProtocolStatusWithoutHTTPStatus();

	//! KeepConnection must be set for HTTP 1.1 with Connection=keep-alive
	void KeepConnection();

protected:

	void testDoReadInputWithCfg( HTTPProcessor &fds, String expected, const bool expectedRequest = true );
	void BuildResult( String &reply, String &fullreply, String result );

	Anything fArgTemplate;	// an argument template used to initialize args
};

#endif
