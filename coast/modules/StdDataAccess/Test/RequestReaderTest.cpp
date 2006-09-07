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
RequestReaderTest::RequestReaderTest(TString tstrName)
	: TestCaseType(tstrName)
{
	StartTrace(RequestReaderTest.RequestReaderTest);
}

TString RequestReaderTest::getConfigFileName()
{
	return "RequestReaderTestConfig";
}

RequestReaderTest::~RequestReaderTest()
{
	StartTrace(RequestReaderTest.Dtor);
}

void RequestReaderTest::CleanupRequestLineTest()
{
	StartTrace(RequestReaderTest.CleanupRequestLineTest);

	ROAnything cConfig;
	AnyExtensions::Iterator<ROAnything> aEntryIterator(GetTestCaseConfig());
	while ( aEntryIterator.Next(cConfig) ) {
		Context ctx;
		MIMEHeader header;
		HTTPProcessor httpProc("HTTPProc");
		httpProc.fLineSizeLimit												= cConfig["LineSizeLimit"].AsLong(4096L);
		httpProc.fURISizeLimit												= cConfig["URISizeLimit"].AsLong(1024L);
		httpProc.fRequestSizeLimit											= cConfig["RequestSizeLimit"].AsLong(5120L);
		httpProc.fCheckUrlEncodingOverride									= cConfig["CheckUrlEncodingOverride"].AsString();
		httpProc.fCheckUrlPathContainsUnsafeCharsOverride 					= cConfig["CheckUrlPathContainsUnsafeCharsOverride"].AsString();
		httpProc.fCheckUrlPathContainsUnsafeCharsAsciiOverride				= cConfig["CheckUrlPathContainsUnsafeCharsAsciiOverride"].AsString();
		httpProc.fCheckUrlPathContainsUnsafeCharsDoNotCheckExtendedAscii	= cConfig["CheckUrlPathContainsUnsafeCharsDoNotCheckExtendedAscii"].AsLong(0);
		httpProc.fCheckUrlArgEncodingOverride										= cConfig["CheckUrlArgEncodingOverride"].AsString();
		httpProc.fUrlExhaustiveDecode										= cConfig["URLExhaustiveDecode"].AsLong(0);
		httpProc.fFixDirectoryTraversial									= cConfig["FixDirectoryTraversial"].AsLong(0);
		httpProc.fURLEncodeExclude											= cConfig["URLEncodeExclude"].AsString("/?");
		RequestReader 	reader(&httpProc, header);
		String uri(cConfig["RequestLine"].AsString());
		StringStreamSocket ss(uri);
		reader.ReadRequest(*(ss.GetStream()), ss.ClientInfo());
		Anything request(reader.GetRequest());
		TraceAny(request, "request:");
		long hasErrors = reader.HasErrors();
		Anything errors = reader.GetErrors();
		Trace("Checking HasErrors");
		assertEqual(cConfig["HasErrors"].AsLong(0), hasErrors);
		Trace("Checking Reason");
		assertEqual(cConfig["Reason"].AsString(), errors["Reason"].AsString(""));
		Trace("Checking ExpectedRequest");
		assertEqual(cConfig["ExpectedRequest"].AsString(), request["REQUEST_URI"].AsString());
		Trace("Resulting REQUEST_URI: " << request["REQUEST_URI"].AsString());
	}
}

void RequestReaderTest::ReadMinimalInputTest()
{
	StartTrace(RequestReaderTest.ReadMinimalInputTest);

	// Fill ts object
	ROAnything cConfig;
	AnyExtensions::Iterator<ROAnything> aEntryIterator(GetTestCaseConfig());
	while ( aEntryIterator.Next(cConfig) ) {
		Context ctx;
		MIMEHeader header;
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
			assertEqualm(cConfig["Expected"]["REQUEST_METHOD"].AsString(), request["REQUEST_METHOD"].AsString(), TString("At index: ") << aEntryIterator.Index());
		}
		if ( cConfig["Expected"].IsDefined("FirstResponseLine") ) {
			StringTokenizer2 st(uri, "\r\n");
			String tok;
			st.NextToken(tok);
			assertEqualm(cConfig["Expected"]["FirstResponseLine"].AsString(), tok, TString("At index: ") << aEntryIterator.Index());
		}
	}
}

Test *RequestReaderTest::suite ()
{
	StartTrace(RequestBodyParserTest.suite);
	TestSuite *testSuite = new TestSuite;
	ADD_CASE(testSuite, RequestReaderTest, ReadMinimalInputTest);
	ADD_CASE(testSuite, RequestReaderTest, CleanupRequestLineTest);
	return testSuite;
}
