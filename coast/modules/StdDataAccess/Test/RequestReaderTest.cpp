/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

//--- interface include --------------------------------------------------------
#include "RequestReaderTest.h"

//--- module under test --------------------------------------------------------
#include "RequestReader.h"

//--- test modules used --------------------------------------------------------
#include "TestSuite.h"

//--- project modules used -----------------------------------------------------

//--- standard modules used ----------------------------------------------------
#include "Dbg.h"
#include "StringStream.h"
#include "StringStreamSocket.h"

//--- c-modules used -----------------------------------------------------------

//---- RequestReaderTest ----------------------------------------------------------------
RequestReaderTest::RequestReaderTest(TString name)
	: ConfiguredTestCase(name, "RequestReaderTestConfig")
{
	StartTrace(RequestReaderTest.Ctor);
}

RequestReaderTest::~RequestReaderTest()
{
	StartTrace(RequestReaderTest.Dtor);
}

// setup for this TestCase
void RequestReaderTest::setUp ()
{
	StartTrace(RequestReaderTest.setUp);
	ConfiguredTestCase::setUp();
}

void RequestReaderTest::tearDown ()
{
	StartTrace(RequestReaderTest.tearDown);
	ConfiguredTestCase::tearDown();
}

void RequestReaderTest::testCase()
{
	StartTrace(RequestReaderTest.testCase);
}

void RequestReaderTest::testCleanupRequestLine()
{
	StartTrace(RequestReaderTest.CleanupTest);

	FOREACH_ENTRY("TestCleanupRequestLine", cConfig, cName) {
		Trace("TestCleanupRequestLine At testindex: " << i << " --------------------------------------------");

		Context ctx;
		MIMEHeader header(0);
		HTTPProcessor httpProc("HTTPProc");
		httpProc.fCheckUrlEncodingOverride						= cConfig["CheckUrlEncodingOverride"].AsString();
		httpProc.fCheckUrlPathContainsUnsafeCharsOverride 		= cConfig["CheckUrlPathContainsUnsafeCharsOverride"].AsString();
		httpProc.fCheckUrlPathContainsUnsafeCharsAsciiOverride	= cConfig["CheckUrlPathContainsUnsafeCharsAsciiOverride"].AsString();
		httpProc.fUrlExhaustiveDecode							= cConfig["URLExhaustiveDecode"].AsLong(0);
		httpProc.fFixDirectoryTraversial						= cConfig["FixDirectoryTraversial"].AsLong(0);
		httpProc.fURLEncodeExclude								= cConfig["URLEncodeExclude"].AsString("/?");
		RequestReader 	reader(&httpProc, header);
		String uri(cConfig["RequestLine"].AsString());
		StringStreamSocket ss(uri);
		reader.ReadRequest(*(ss.GetStream()), ss.ClientInfo());
		Anything request(reader.GetRequest());
		TraceAny(request, "request:");
		long hasErrors = reader.HasErrors();
		Anything errors = reader.GetErrors();
		Trace("Checking HasErrors");
		assertEquals(cConfig["HasErrors"].AsLong(0), hasErrors);
		Trace("Checking Reason");
		assertEquals(cConfig["Reason"].AsString(), errors["Reason"].AsString(""));
		Trace("Checking ExpectedRequest");
		assertEquals(cConfig["ExpectedRequest"].AsString(), request["REQUEST_URI"].AsString());
		Trace("Resulting REQUEST_URI: " << request["REQUEST_URI"].AsString());
	}
}

void RequestReaderTest::testReadMinimalInput()
{
	StartTrace(RequestReaderTest.testReadInput);

	// Fill ts object
	FOREACH_ENTRY("TestReadMinimalInput", cConfig, cName) {
		Trace("TestReadMinimalInput At testindex: " << i << " --------------------------------------------");
		Context ctx;
		MIMEHeader header(0);
		HTTPProcessor httpProc("HTTPProc");
		httpProc.fRequestSizeLimit 	= cConfig["RequestSizeLimit"].AsLong(0);
		httpProc.fLineSizeLimit 	= cConfig["LineSizeLimit"].AsLong(0);
		httpProc.fURISizeLimit		= cConfig["UriSizeLimit"].AsLong(0);
		RequestReader 	reader(&httpProc, header);

		String uri(cConfig["RequestLine"].AsString());
		StringStreamSocket ss(uri);
		bool ret(reader.ReadRequest(*(ss.GetStream()), ss.ClientInfo()));
		t_assert(cConfig["Expected"]["Return"].AsBool(0) == ret);
		Trace("Returning: " << uri);
		Anything request(reader.GetRequest());

		if ( cConfig["Expected"].IsDefined("REQUEST_METHOD") ) {
			assertEqualm(cConfig["Expected"]["REQUEST_METHOD"].AsString(), request["REQUEST_METHOD"].AsString(), TString("At index: ") << i);
		}
		if ( cConfig["Expected"].IsDefined("FirstResponseLine") ) {
			StringTokenizer2 st(uri, "\r\n");
			String tok;
			st.NextToken(tok);
			assertEqualm(cConfig["Expected"]["FirstResponseLine"].AsString(), tok, TString("At index: ") << i);
		}
	}
}

Test *RequestReaderTest::suite ()
{
	StartTrace(RequestBodyParserTest.suite);
	TestSuite *testSuite = new TestSuite;

	testSuite->addTest (NEW_CASE(RequestReaderTest, testCase));
//	testSuite->addTest (NEW_CASE(RequestReaderTest, testReadMinimalInput));
	testSuite->addTest (NEW_CASE(RequestReaderTest, testCleanupRequestLine));
	return testSuite;
} // suite
