/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

//--- interface include --------------------------------------------------------
#include "RequestBodyParserTest.h"

//--- test modules used --------------------------------------------------------
#include "TestSuite.h"

//--- module under test --------------------------------------------------------
#include "RequestBodyParser.h"

//--- project modules used -----------------------------------------------------
#include "MIMEHeader.h"

//--- standard modules used ----------------------------------------------------
#include "System.h"
#include "Dbg.h"
#include "Renderer.h"
#include "SysLog.h"

//---- RequestBodyParserTest ----------------------------------------------------------------
RequestBodyParserTest::RequestBodyParserTest(TString tname)
	: ConfiguredTestCase(tname, "RequestBodyParserTestConfig")
{
	StartTrace(RequestBodyParserTest.Ctor);
}

RequestBodyParserTest::~RequestBodyParserTest()
{
	StartTrace(RequestBodyParserTest.Dtor);
}

// setup for this TestCase
void RequestBodyParserTest::setUp ()
{
	StartTrace(RequestBodyParserTest.setUp);
	ConfiguredTestCase::setUp();
} // setUp

void RequestBodyParserTest::tearDown ()
{
	StartTrace(RequestBodyParserTest.tearDown);
	ConfiguredTestCase::tearDown();
} // tearDown

void RequestBodyParserTest::ReadMultiPartPost()
{
	StartTrace(RequestBodyParserTest.ReadMultiPartPost);
	iostream *is = System::OpenStream("MultiPartBody.txt", 0);

	t_assertm(is != 0, "expected 'MultiPartBody.txt' to be there");
	if ( is ) {
		MIMEHeader mh(0);
		t_assertm(mh.DoReadHeader(*is, 4096, 4096), "expected global header parsing to succeed");

		RequestBodyParser sm(mh, *is);
		t_assert(sm.Parse());
		Anything expected;
		Anything result = sm.GetContent();

		delete is;

		t_assert(!fConfig.IsNull());
		assertAnyEqual(fConfig["MultiPartResult"], result);
	}
	// This sequence takes advantage of the String::SubString
	// feature to return the whole remainder of the string starting
	// from the first occurrence the substring-pattern has been found.
	// This relieves us from the need to create an expected result file
	// which we would use to verify the results.

	is =  System::OpenStream("MultiPartBody.txt", 0);
	if ( is ) {
		MIMEHeader mh(0);
		t_assertm(mh.DoReadHeader(*is, 4096, 4096), "expected global header parsing to succeed");

		RequestBodyParser sm(mh, *is);
		t_assert(sm.Parse());
		String unparsedContent = sm.GetUnparsedContent();
		delete is;

		is =  System::OpenStream("MultiPartBody.txt", 0);
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

void RequestBodyParserTest::ReadToBoundaryTestWithStreamFailure()
{
	StartTrace(RequestBodyParserTest.ReadToBoundaryTestWithStreamFailure);

	FOREACH_ENTRY("ReadToBoundaryTestWithStreamFailure", cConfig, cName) {
		String result, strInput(cConfig["Input"].AsCharPtr());
		Trace("input to parse [" << strInput << "]");
		StringStream tiss(strInput);
		MIMEHeader mh(0);
		RequestBodyParser sm(mh, tiss);
#if defined(WIN32) && !defined(ONLY_STD_IOSTREAM)
		int iState = tiss.rdstate();
#else
		ios::iostate iState = tiss.rdstate();
#endif
		Trace("original iState:" << (long)(iState));

		// test failbit detection
		tiss.clear(ios::failbit | iState);
		Trace("iState with failbit:" << (long)(iState | ios::failbit));
		bool res = sm.ReadToBoundary(&tiss, cConfig["Boundary"].AsCharPtr(), result);
		assertEqualm("", result, cName);
		assertEqualm(false, res, cName);
		iState = tiss.rdstate() ^ ios::failbit;
		Trace("iState failbit cleared:" << (long)iState);
		tiss.clear(iState);

		// test badbit detection
		iState = iState | ios::badbit;
		tiss.clear(iState);
		Trace("iState with badbit:" << (long)iState);
		res = sm.ReadToBoundary(&tiss, cConfig["Boundary"].AsCharPtr(), result);
		assertEqualm("", result, cName);
		assertEqualm(false, res, cName);
		iState = tiss.rdstate() ^ ios::badbit;
		Trace("iState badbit cleared:" << (long)iState);
		tiss.clear(iState);

		// test eofbit detection
		iState = iState | ios::eofbit;
		tiss.clear(iState);
		Trace("iState with eofbit:" << (long)iState);
		res = sm.ReadToBoundary(&tiss, cConfig["Boundary"].AsCharPtr(), result);
		assertEqualm("", result, cName);
		assertEqualm(false, res, cName);
		iState = tiss.rdstate() ^ ios::eofbit;
		Trace("iState eofbit cleared:" << (long)iState);
		tiss.clear(iState);

		// clean tests
		res = sm.ReadToBoundary(&tiss, cConfig["Boundary"].AsCharPtr(), result);
		assertEqualm(cConfig["ExpectedResult"].AsCharPtr(), result, cName);
		assertEqualm(cConfig["ExpectedEndReached"].AsLong(), (long) res, cName);
		assertEqualm(cConfig["ExpectedUnparsedContent"].AsCharPtr(), sm.GetUnparsedContent(), cName);
	}
}

void RequestBodyParserTest::ReadToBoundaryTest()
{
	Context ctx;
	FOREACH_ENTRY("ReadToBoundaryTest", cConfig, cName) {
		String result;
		IStringStream tiss(Renderer::RenderToString(ctx, cConfig["Input"]));
		MIMEHeader mh(0);
		RequestBodyParser sm(mh, tiss);

		bool res = sm.ReadToBoundary(&tiss, cConfig["Boundary"].AsString(), result);
		assertEqualm(Renderer::RenderToString(ctx, cConfig["ExpectedResult"]), result, cName);
		assertEqualm(cConfig["ExpectedEndReached"].AsLong(), (long) res, cName);

		String expUnparsed(Renderer::RenderToString(ctx, cConfig["ExpectedUnparsedContent"]));
		String realUnparsed(sm.GetUnparsedContent());
		if (!assertEqualm(expUnparsed, realUnparsed, cName)) {
			for (long i = 0; i < expUnparsed.Length() && i < realUnparsed.Length(); i++) {
				if (!assertEqualm(expUnparsed[i], realUnparsed[i], TString("Position: ") << i << " " << expUnparsed[i])) {
					break;
				}
			}
		}
	}

	StartTrace(RequestBodyParserTest.ReadMultiPartPost);
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
			IStringStream is(&testinput);
			MIMEHeader mh(0);
			RequestBodyParser sm(mh, is);

			t_assertm(!sm.ReadToBoundary(&is, testboundary, result), "expected end reached with simple input");
			assertEqual("some content1", result);

			result = "";
			t_assertm(!sm.ReadToBoundary(&is, testboundary, result), "expected end reached with simple input");
			assertEqual("some content2", result);
		}
		{
			IStringStream is(&testinput);
			MIMEHeader mh(0);
			RequestBodyParser sm(mh, is);
			sm.ReadToBoundary(&is, testboundary, result);
			String unparsedContent;
			unparsedContent = sm.GetUnparsedContent();
			assertEqual("--980\r\n", unparsedContent);
			sm.ReadToBoundary(&is, testboundary, result);
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
			IStringStream is(&testinput);
			MIMEHeader mh(0);
			RequestBodyParser sm(mh, is);

			t_assertm(!sm.ReadToBoundary(&is, testboundary, result), "expected  end not reached with simple input");
			assertEqual("some content1", result);

			result = "";
			t_assertm(sm.ReadToBoundary(&is, testboundary, result), "expected end reached with simple input");
			assertEqual("some content2", result);
		}
		{
			IStringStream is(&testinput);
			MIMEHeader mh(0);
			RequestBodyParser sm(mh, is);
			sm.ReadToBoundary(&is, testboundary, result);
			String unparsedContent;
			unparsedContent = sm.GetUnparsedContent();
			assertEqual("--980\r\n", unparsedContent);
			sm.ReadToBoundary(&is, testboundary, result);
			unparsedContent = sm.GetUnparsedContent();
			assertEqual("--980\r\nsome content1\r\n--980\r\n", unparsedContent);
			sm.ReadToBoundary(&is, testboundary, result);
			unparsedContent = sm.GetUnparsedContent();
			assertEqual("--980\r\nsome content1\r\n--980\r\nsome content2\r\n--980--", unparsedContent);
		}
	}
}

void RequestBodyParserTest::ParseMultiPartTest()
{
	StartTrace(RequestBodyParserTest.ParseMultiPartTest);
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
	result[0L]["header"]["CONTENT-LENGTH"] = 125L;
	result[0L]["header"]["CONTENT-TYPE"] = "multipart/part";
	result[0L]["body"].Append("01234");
	{
		IStringStream is(testinput);
		MIMEHeader mh(0);
		RequestBodyParser sm(mh, is);

		assertAnyEqualm(Anything(), sm.GetContent(), "expected fContent to be empty" );
		t_assert(sm.ParseMultiPart(&is, testboundary));
	}
	{
		IStringStream is(testinput1);
		MIMEHeader mh(0);
		RequestBodyParser sm(mh, is);
		sm.ParseMultiPart(&is, testboundary);

		// see RFC 1806 for details about the content disposition header
		String unparsedContent;
		unparsedContent = sm.GetUnparsedContent();
		t_assert(assertEqual(testinput1, unparsedContent));
	}
}

// builds up a suite of testcases, add a line for each testmethod
Test *RequestBodyParserTest::suite ()
{
	StartTrace(RequestBodyParserTest.suite);
	TestSuite *testSuite = new TestSuite;

	testSuite->addTest (NEW_CASE(RequestBodyParserTest, ReadToBoundaryTest));
	testSuite->addTest (NEW_CASE(RequestBodyParserTest, ReadToBoundaryTestWithStreamFailure));
	testSuite->addTest (NEW_CASE(RequestBodyParserTest, ParseMultiPartTest));
	testSuite->addTest (NEW_CASE(RequestBodyParserTest, ReadMultiPartPost));
	return testSuite;
} // suite
