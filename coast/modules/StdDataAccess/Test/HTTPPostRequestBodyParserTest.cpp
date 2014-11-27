/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#include "HTTPPostRequestBodyParserTest.h"
#include "TestSuite.h"
#include "HTTPPostRequestBodyParser.h"
#include "MIMEHeader.h"
#include "SystemFile.h"
#include "Tracer.h"
#include "Renderer.h"
#include "SystemLog.h"

namespace {
	// allow access to protected methods for testing
	struct TestPostRequestBodyParser : public HTTPPostRequestBodyParser {
		TestPostRequestBodyParser(MIMEHeader &mainheader) :
			HTTPPostRequestBodyParser(mainheader) {
		}
		virtual bool DoParseMultiPart(std::istream &input, const String &bound) {
			return HTTPPostRequestBodyParser::DoParseMultiPart(input, bound);
		}
		virtual bool DoReadToBoundary(std::istream &input, const String &bound, String &body) {
			return HTTPPostRequestBodyParser::DoReadToBoundary(input, bound, body);
		}
	};
}

HTTPPostRequestBodyParserTest::HTTPPostRequestBodyParserTest(TString tname)
	: TestCaseType(tname)
{
	StartTrace(HTTPPostRequestBodyParserTest.HTTPPostRequestBodyParserTest);
}

TString HTTPPostRequestBodyParserTest::getConfigFileName()
{
	return "HTTPPostRequestBodyParserTestConfig";
}

void HTTPPostRequestBodyParserTest::ReadMultiPartPost()
{
	StartTrace(HTTPPostRequestBodyParserTest.ReadMultiPartPost);
	std::iostream *is = coast::system::OpenStream("MultiPartBody.txt", 0);

	t_assertm(is != 0, "expected 'MultiPartBody.txt' to be there");
	if ( is ) {
		MIMEHeader mh;
		t_assertm(mh.ParseHeaders(*is, 4096, 4096), "expected global header parsing to succeed");
		assertEqual(586L, mh.GetParsedHeaderLength());
		HTTPPostRequestBodyParser sm(mh);
		t_assert(sm.Parse(*is));
		Anything expected;
		Anything result = sm.GetContent();

		delete is;

		t_assert(!GetConfig().IsNull());
		assertAnyEqual(GetConfig()["MultiPartResult"], result);
	}
	// This sequence takes advantage of the String::SubString
	// feature to return the whole remainder of the string starting
	// from the first occurrence the substring-pattern has been found.
	// This relieves us from the need to create an expected result file
	// which we would use to verify the results.

	is =  coast::system::OpenStream("MultiPartBody.txt", 0);
	if ( is ) {
		MIMEHeader mh;
		t_assertm(mh.ParseHeaders(*is, 4096, 4096), "expected global header parsing to succeed");
		assertEqual(586L, mh.GetParsedHeaderLength());
		HTTPPostRequestBodyParser sm(mh);
		t_assert(sm.Parse(*is));
		String unparsedContent = sm.GetUnparsedContent();
		delete is;

		is =  coast::system::OpenStream("MultiPartBody.txt", 0);
		if (is) {
			char c;
			String tmp;
			while ((!!(*is).get(c))) {
				tmp.Append(c);
			}
			String final;
			final = tmp.SubString(unparsedContent);
			assertEqual(final, unparsedContent);
			delete is;
		}
	}
}

void HTTPPostRequestBodyParserTest::ReadToBoundaryTestWithStreamFailure()
{
	StartTrace(HTTPPostRequestBodyParserTest.ReadToBoundaryTestWithStreamFailure);

	ROAnything cConfig;
	AnyExtensions::Iterator<ROAnything, ROAnything, TString> aEntryIterator(GetTestCaseConfig());
	while ( aEntryIterator.Next(cConfig) ) {
		TString cName;
		aEntryIterator.SlotName(cName);
		String result, strInput(cConfig["Input"].AsCharPtr());
		Trace("input to parse [" << strInput << "]");
		StringStream tiss(strInput);
		MIMEHeader mh;
		TestPostRequestBodyParser sm(mh);
		std::ios::iostate iState = tiss.rdstate();
		Trace("original iState:" << (long)(iState));

		// test failbit detection
		tiss.clear(std::ios::failbit | iState);
		Trace("iState with failbit:" << (long)(iState | std::ios::failbit));
		bool res = sm.DoReadToBoundary(tiss, cConfig["Boundary"].AsCharPtr(), result);
		assertEqualm("", result, cName);
		assertEqualm(false, res, cName);
		iState = tiss.rdstate() ^ std::ios::failbit;
		Trace("iState failbit cleared:" << (long)iState);
		tiss.clear(iState);

		// test badbit detection
		iState = iState | std::ios::badbit;
		tiss.clear(iState);
		Trace("iState with badbit:" << (long)iState);
		res = sm.DoReadToBoundary(tiss, cConfig["Boundary"].AsCharPtr(), result);
		assertEqualm("", result, cName);
		assertEqualm(false, res, cName);
		iState = tiss.rdstate() ^ std::ios::badbit;
		Trace("iState badbit cleared:" << (long)iState);
		tiss.clear(iState);

		// test eofbit detection
		iState = iState | std::ios::eofbit;
		tiss.clear(iState);
		Trace("iState with eofbit:" << (long)iState);
		res = sm.DoReadToBoundary(tiss, cConfig["Boundary"].AsCharPtr(), result);
		assertEqualm("", result, cName);
		assertEqualm(false, res, cName);
		iState = tiss.rdstate() ^ std::ios::eofbit;
		Trace("iState eofbit cleared:" << (long)iState);
		tiss.clear(iState);

		// clean tests
		res = sm.DoReadToBoundary(tiss, cConfig["Boundary"].AsCharPtr(), result);
		assertEqualm(cConfig["ExpectedResult"].AsCharPtr(), result, cName);
		assertEqualm(cConfig["ExpectedEndReached"].AsLong(), (long) res, cName);
		assertEqualm(cConfig["ExpectedUnparsedContent"].AsCharPtr(), sm.GetUnparsedContent(), cName);
	}
}

void HTTPPostRequestBodyParserTest::ReadToBoundaryTest()
{
	Context ctx;
	ROAnything cConfig;
	AnyExtensions::Iterator<ROAnything, ROAnything, TString> aEntryIterator(GetTestCaseConfig());
	while ( aEntryIterator.Next(cConfig) ) {
		TString cName;
		aEntryIterator.SlotName(cName);
		String result, strIn(Renderer::RenderToString(ctx, cConfig["Input"]));
		IStringStream tiss(strIn);
		MIMEHeader mh;
		TestPostRequestBodyParser sm(mh);

		bool res = sm.DoReadToBoundary(tiss, cConfig["Boundary"].AsString(), result);
		assertEqualm(Renderer::RenderToString(ctx, cConfig["ExpectedResult"]), result, cName);
		assertEqualm(cConfig["ExpectedEndReached"].AsLong(), (long) res, cName);

		String expUnparsed(Renderer::RenderToString(ctx, cConfig["ExpectedUnparsedContent"]));
		String realUnparsed(sm.GetUnparsedContent());
		if (!assertEqualm(expUnparsed, realUnparsed, cName)) {
			for (long ii = 0; ii < expUnparsed.Length() && ii < realUnparsed.Length(); ii++) {
				if (!assertEqualm(expUnparsed[ii], realUnparsed[ii], TString("Position: ") << ii << " " << expUnparsed[ii])) {
					break;
				}
			}
		}
	}

	StartTrace(HTTPPostRequestBodyParserTest.ReadToBoundaryTest);
	String testinput, testinput1;
	String testboundary("980");
	String result;
	String expected;

	// normal case with two parts
	{
		result = "";
		testinput = "--";
		testinput << testboundary << "\r\nsome content1\r\n--" << testboundary;
		testinput << "\r\nsome content2\r\n--" << testboundary;
		{
			IStringStream is(testinput);
			MIMEHeader mh;
			TestPostRequestBodyParser sm(mh);

			t_assertm(!sm.DoReadToBoundary(is, testboundary, result), "expected end reached with simple input");
			assertEqual("some content1", result);

			result = "";
			t_assertm(!sm.DoReadToBoundary(is, testboundary, result), "expected end reached with simple input");
			assertEqual("some content2", result);
		}
		{
			IStringStream is(testinput);
			MIMEHeader mh;
			TestPostRequestBodyParser sm(mh);
			sm.DoReadToBoundary(is, testboundary, result);
			String unparsedContent;
			unparsedContent = sm.GetUnparsedContent();
			assertEqual("--980\r\n", unparsedContent);
			sm.DoReadToBoundary(is, testboundary, result);
			unparsedContent = sm.GetUnparsedContent();
			assertEqual("--980\r\nsome content1\r\n--980\r\n", unparsedContent);
		}
	}
	// normal case with two parts and end indication
	{
		result = "";
		testinput = "--";
		testinput << testboundary << "\r\nsome content1\r\n--" << testboundary;
		testinput << "\r\nsome content2\r\n--" << testboundary << "--";
		{
			IStringStream is(testinput);
			MIMEHeader mh;
			TestPostRequestBodyParser sm(mh);

			t_assertm(!sm.DoReadToBoundary(is, testboundary, result), "expected  end not reached with simple input");
			assertEqual("some content1", result);

			result = "";
			t_assertm(sm.DoReadToBoundary(is, testboundary, result), "expected end reached with simple input");
			assertEqual("some content2", result);
		}
		{
			IStringStream is(testinput);
			MIMEHeader mh;
			TestPostRequestBodyParser sm(mh);
			sm.DoReadToBoundary(is, testboundary, result);
			String unparsedContent;
			unparsedContent = sm.GetUnparsedContent();
			assertEqual("--980\r\n", unparsedContent);
			sm.DoReadToBoundary(is, testboundary, result);
			unparsedContent = sm.GetUnparsedContent();
			assertEqual("--980\r\nsome content1\r\n--980\r\n", unparsedContent);
			sm.DoReadToBoundary(is, testboundary, result);
			unparsedContent = sm.GetUnparsedContent();
			assertEqual("--980\r\nsome content1\r\n--980\r\nsome content2\r\n--980--", unparsedContent);
		}
	}
}

void HTTPPostRequestBodyParserTest::ParseMultiPartTest()
{
	StartTrace(HTTPPostRequestBodyParserTest.ParseMultiPartTest);
	String testinput, testinput1;
	String testboundary("---------------------------210003122518197");
	Anything result;
	String expected;

	testinput =
		"-----------------------------210003122518197\r\n"
		"Content-Disposition: form-data; name=\"Datei3\"; filename=\"G:\\DEVELOP\\coast\\foundation\\Test\\config\\len5.tst\"\r\n"
		"\r\n"
		"01234\r\n"
		"-----------------------------210003122518197--";

	// The newlines are "\n", in the test above they where "\r\n" because they where compared
	// to an anything

	testinput1 =
		"-----------------------------210003122518197\n"
		"Content-Disposition: inline; name=\"kimdojo1\"; filename=\"C:\\karate\\pics\\hank.gif\"\n"
		"\n"
		"A492993f39393939f393939393f393939r393\n"
		"-----------------------------210003122518197--\n";

	result[0L]["header"]["CONTENT-DISPOSITION"][0L] = "form-data";
	result[0L]["header"]["CONTENT-DISPOSITION"]["NAME"] = "Datei3";
	result[0L]["header"]["CONTENT-DISPOSITION"]["FILENAME"] = "G:\\DEVELOP\\coast\\foundation\\Test\\config\\len5.tst";
	result[0L]["header"]["CONTENT-TYPE"] = "multipart/part";
	result[0L]["body"].Append("01234");
	{
		IStringStream is(testinput);
		MIMEHeader mh;
		TestPostRequestBodyParser sm(mh);
		assertAnyEqualm(Anything(), sm.GetContent(), "expected fContent to be empty" );
		t_assert(sm.DoParseMultiPart(is, testboundary));
		assertAnyEqualm(result, sm.GetContent(), "expected valid Content" );
	}
	{
		IStringStream is(testinput1);
		MIMEHeader mh;
		TestPostRequestBodyParser sm(mh);
		try {
			sm.DoParseMultiPart(is, testboundary);
		} catch (MIMEHeader::InvalidLineException &e) {
			t_assertm(true, "expected invalid line exception to happen");
		} catch (...) {
			t_assertm(false,"did not expect other exceptions to be thrown");
		}

		// see RFC 1806 for details about the content disposition header
		String unparsedContent;
		unparsedContent = sm.GetUnparsedContent();
		assertCharPtrEqual(testinput1, unparsedContent);
	}
}

Test *HTTPPostRequestBodyParserTest::suite ()
{
	StartTrace(HTTPPostRequestBodyParserTest.suite);
	TestSuite *testSuite = new TestSuite;
	ADD_CASE(testSuite, HTTPPostRequestBodyParserTest, ReadToBoundaryTest);
	ADD_CASE(testSuite, HTTPPostRequestBodyParserTest, ReadToBoundaryTestWithStreamFailure);
	ADD_CASE(testSuite, HTTPPostRequestBodyParserTest, ParseMultiPartTest);
	ADD_CASE(testSuite, HTTPPostRequestBodyParserTest, ReadMultiPartPost);
	return testSuite;
}
