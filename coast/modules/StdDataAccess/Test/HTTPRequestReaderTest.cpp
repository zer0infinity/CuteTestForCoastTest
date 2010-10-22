/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

//--- interface include --------------------------------------------------------
#include "HTTPRequestReaderTest.h"

//--- module under test --------------------------------------------------------
#include "HTTPRequestReader.h"

//--- test modules used --------------------------------------------------------
#include "TestSuite.h"

//--- standard modules used ----------------------------------------------------
#include "Context.h"
#include "HTTPProcessor.h"
#include "MIMEHeader.h"
#include "Dbg.h"
#include "StringStream.h"
#include "StringStreamSocket.h"
#include "AnyIterators.h"

//---- HTTPRequestReaderTest ----------------------------------------------------------------
HTTPRequestReaderTest::HTTPRequestReaderTest(TString tstrName) :
	TestCaseType(tstrName) {
	StartTrace(HTTPRequestReaderTest.HTTPRequestReaderTest);
}

TString HTTPRequestReaderTest::getConfigFileName() {
	return "HTTPRequestReaderTestConfig";
}

HTTPRequestReaderTest::~HTTPRequestReaderTest() {
	StartTrace(HTTPRequestReaderTest.Dtor);
}

void HTTPRequestReaderTest::CleanupRequestLineTest() {
	StartTrace(HTTPRequestReaderTest.CleanupRequestLineTest);

	ROAnything cConfig;
	AnyExtensions::Iterator<ROAnything> aEntryIterator(GetTestCaseConfig());
	RequestProcessor *httpProc = HTTPProcessor::FindRequestProcessor("HTTPProcessor");
	while (aEntryIterator.Next(cConfig)) {
		Anything anyParams = cConfig.DeepClone();
		AnyLookupInterfaceAdapter<Anything> lia(anyParams);
		Context ctx;
		ctx.Push("tempargs", &lia);

		MIMEHeader header;
		HTTPRequestReader reader(httpProc, header);

		String uri = cConfig["RequestLine"].AsString();
		StringStreamSocket ss(uri);
		reader.ReadRequest(ctx, *(ss.GetStream()), ss.ClientInfo());
		Anything request(reader.GetRequest());
		TraceAny(request, "request:");
		ROAnything errors = ctx.Lookup("HTTPRequestReader");
		long hasErrors = errors.GetSize() ? 1L : 0L;
		assertEqualm(cConfig["HasErrors"].AsLong(0), hasErrors, TString("failed at idx:") << aEntryIterator.Index());
		assertEqualm(cConfig["Reason"].AsString(), errors["Reason"].AsString(""), TString("failed at idx:") << aEntryIterator.Index());
		assertEqualm(cConfig["ExpectedRequest"].AsString(), request["REQUEST_URI"].AsString(), TString("failed at idx:") << aEntryIterator.Index());
		Trace("Resulting REQUEST_URI: " << request["REQUEST_URI"].AsString());
	}
}

void HTTPRequestReaderTest::ReadMinimalInputTest() {
	StartTrace(HTTPRequestReaderTest.ReadMinimalInputTest);

	ROAnything cConfig;
	AnyExtensions::Iterator<ROAnything> aEntryIterator(GetTestCaseConfig());
	RequestProcessor *httpProc = HTTPProcessor::FindRequestProcessor("HTTPProcessor");
	while (aEntryIterator.Next(cConfig)) {
		Anything anyParams = cConfig.DeepClone();
		AnyLookupInterfaceAdapter<Anything> lia(anyParams);
		Context ctx;
		ctx.Push("tempargs", &lia);

		MIMEHeader header;
		HTTPRequestReader reader(httpProc, header);

		String uri(cConfig["RequestLine"].AsString());
		Trace("Request to process [" << uri << "]");
		StringStreamSocket ss(uri);
		bool ret(reader.ReadRequest(ctx, *(ss.GetStream()), ss.ClientInfo()));
		t_assertm(cConfig["Expected"]["Return"].AsBool(0) == ret, TString("failed at idx:") << aEntryIterator.Index());
		Trace("response [" << uri << "]");
		Anything request(reader.GetRequest());

		if (cConfig["Expected"].IsDefined("REQUEST_METHOD")) {
			assertEqualm(cConfig["Expected"]["REQUEST_METHOD"].AsString(), request["REQUEST_METHOD"].AsString(), TString("At index: ") << aEntryIterator.Index());
		}

		if (cConfig["Expected"].IsDefined("FirstResponseLine")) {
			StringTokenizer2 st(uri, "\r\n");
			String tok;
			st.NextToken(tok);
			Trace("tok: [" << tok << "]");
			assertEqualm(cConfig["Expected"]["FirstResponseLine"].AsString(), tok, TString("At index: ") << aEntryIterator.Index());
		}
	}
}

Test *HTTPRequestReaderTest::suite() {
	StartTrace(HTTPRequestReaderTest.suite);
	TestSuite *testSuite = new TestSuite;
	ADD_CASE(testSuite, HTTPRequestReaderTest, ReadMinimalInputTest);
	ADD_CASE(testSuite, HTTPRequestReaderTest, CleanupRequestLineTest);
	return testSuite;
}
