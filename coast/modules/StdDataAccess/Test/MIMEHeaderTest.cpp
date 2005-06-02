/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

//--- standard modules used ----------------------------------------------------
#include "Anything.h"
#include "StringStream.h"
#include "URLUtils.h"
#include "Dbg.h"

//--- test modules used --------------------------------------------------------
#include "TestSuite.h"

//--- module under test --------------------------------------------------------
#include "MIMEHeader.h"

//--- interface include --------------------------------------------------------
#include "MIMEHeaderTest.h"

//---- MIMEHeaderTest ----------------------------------------------------------------
MIMEHeaderTest::MIMEHeaderTest(TString tname) : TestCase(tname)
{
	StartTrace(MIMEHeaderTest.Ctor);
}

MIMEHeaderTest::~MIMEHeaderTest()
{
	StartTrace(MIMEHeaderTest.Dtor);
}

// setup for this TestCase
void MIMEHeaderTest::setUp ()
{
	StartTrace(MIMEHeaderTest.setUp);
} // setUp

void MIMEHeaderTest::tearDown ()
{
	StartTrace(MIMEHeaderTest.tearDown);
} // tearDown

void MIMEHeaderTest::SimpleHeaderTest()
{
	StartTrace(MIMEHeaderTest.SimpleHeaderTest);

	String testinput;
	Anything result, result1;

	// some bad data tests
	testinput = "nonsense";
	{
		MIMEHeader mh;
		StringStream is(testinput);

		// basic checks of success
		t_assertm(!mh.DoReadHeader(is), "expected header parsing to fail");

		// sanity checks
		t_assertm(mh.IsMultiPart() == false, "expected no multipart");
		t_assertm(mh.GetBoundary().Length() == 0, "expected no multipart seperator");
		t_assertm(mh.GetContentLength() == -1, "expected -1, since field is not set");
		assertEqualm("", mh.Lookup("NotThere", ""), "expected 'NotThere' to be emtpy");
		assertAnyEqual(result, mh.GetInfo());
		TraceAny(mh.GetInfo(), "Header: ");
	}

	// test a simple header
	testinput =
		"Connection: Keep-Alive\r\n"
		"User-Agent: Mozilla/4.7 [en] (WinNT; U)\r\n"
		"Host: sentosa.hsr.loc:2929\r\n"
		"Accept: image/gif, image/x-xbitmap, image/jpeg, image/pjpeg, image/png, */*\r\n"
		"Accept-Encoding: gzip\r\n"
		"Accept-Language: de-CH,en\r\n"
		"Accept-Charset: iso-8859-1,*,utf-8\r\n"
		"\r\n";
	Trace("TestInput: <" << testinput << ">");
	result["CONNECTION"] = "Keep-Alive";
	result["USER-AGENT"] = "Mozilla/4.7 [en] (WinNT; U)";
	result["HOST"] = "sentosa.hsr.loc:2929";
	result["ACCEPT"].Append("image/gif");
	result["ACCEPT"].Append("image/x-xbitmap");
	result["ACCEPT"].Append("image/jpeg");
	result["ACCEPT"].Append("image/pjpeg");
	result["ACCEPT"].Append("image/png");
	result["ACCEPT"].Append("*/*");
	result["ACCEPT-ENCODING"] = "gzip";
	result["ACCEPT-LANGUAGE"].Append("de-CH");
	result["ACCEPT-LANGUAGE"].Append("en");
	result["ACCEPT-CHARSET"].Append("iso-8859-1");
	result["ACCEPT-CHARSET"].Append("*");
	result["ACCEPT-CHARSET"].Append("utf-8");

	result1["CONNECTION"] = "Keep-Alive";
	result1["USER-AGENT"] = "Mozilla/4.7 [en] (WinNT; U)";
	result1["HOST"] = "sentosa.hsr.loc:2929";
	result1["ACCEPT"] = "image/gif, image/x-xbitmap, image/jpeg, image/pjpeg, image/png, */*";
	result1["ACCEPT-ENCODING"] = "gzip";
	result1["ACCEPT-LANGUAGE"] = "de-CH,en";
	result1["ACCEPT-CHARSET"] = "iso-8859-1,*,utf-8";

	{
		MIMEHeader mh;
		StringStream is(testinput);

		// basic checks of success
		t_assertm(mh.DoReadHeader(is), "expected header parsing to succeed");

		// sanity checks
		t_assertm(mh.IsMultiPart() == false, "expected no multipart");
		t_assertm(mh.GetBoundary().Length() == 0, "expected no multipart seperator");
		t_assertm(mh.GetContentLength() == -1, "expected -1, since field is not set");
		assertEqualm("", mh.Lookup("NotThere", ""), "expected 'NotThere' to be emtpy");

		assertAnyEqual(result, mh.GetInfo());
	}
	{
		MIMEHeader mh(URLUtils::eUpshift, MIMEHeader::eDoNotSplitHeaderFields);
		StringStream is(testinput);

		// basic checks of success
		t_assertm(mh.DoReadHeader(is), "expected header parsing to succeed");

		// sanity checks
		t_assertm(mh.IsMultiPart() == false, "expected no multipart");
		t_assertm(mh.GetBoundary().Length() == 0, "expected no multipart seperator");
		t_assertm(mh.GetContentLength() == -1, "expected -1, since field is not set");
		assertEqualm("", mh.Lookup("NotThere", ""), "expected 'NotThere' to be emtpy");

		assertAnyEqual(result1, mh.GetInfo());
	}

}

void MIMEHeaderTest::SetCookieTest()
{
	StartTrace(MIMEHeaderTest.SetCookieTest);

	String testinput;
	Anything result;

	// test a simple header
	testinput =
		"Connection: Keep-Alive\r\n"
		"User-Agent: Mozilla/4.7 [en] (WinNT; U)\r\n"
		"Host: sentosa.hsr.ch:2929\r\n"
		"Set-Cookie: Test1=test_value1; expires=Sat, 01-Jan-2001 01:01:01 GMT; path=/;\r\n"
		"Set-Cookie: Test2=test_value2; expires=Sat, 02-Jan-2002 02:02:02 GMT; path=/;\r\n"
		"Set-Cookie: Test3=test_value3; expires=Sat, 03-Jan-2003 03:03:03 GMT; path=/;\r\n"
		"\r\n";
	Trace("TestInput: <" << testinput << ">");
	result["CONNECTION"] = "Keep-Alive";
	result["USER-AGENT"] = "Mozilla/4.7 [en] (WinNT; U)";
	result["HOST"] = "sentosa.hsr.ch:2929";
	result["SET-COOKIE"][0L] = "Test1=test_value1; expires=Sat, 01-Jan-2001 01:01:01 GMT; path=/;";
	result["SET-COOKIE"][1L] = "Test2=test_value2; expires=Sat, 02-Jan-2002 02:02:02 GMT; path=/;";
	result["SET-COOKIE"][2L] = "Test3=test_value3; expires=Sat, 03-Jan-2003 03:03:03 GMT; path=/;";

	{
		MIMEHeader mh;
		StringStream is(testinput);

		// basic checks of success
		t_assertm(mh.DoReadHeader(is), "expected header parsing to succeed");

		// sanity checks
		t_assertm(mh.IsMultiPart() == false, "expected no multipart");
		t_assertm(mh.GetBoundary().Length() == 0, "expected no multipart seperator");
		t_assertm(mh.GetContentLength() == -1, "expected -1, since field is not set");
		assertEqualm("", mh.Lookup("NotThere", ""), "expected 'NotThere' to be emtpy");

		assertAnyEqual(result, mh.GetInfo());
	}
	{
		MIMEHeader mh(URLUtils::eUpshift, MIMEHeader::eDoNotSplitHeaderFields);
		StringStream is(testinput);

		// basic checks of success
		t_assertm(mh.DoReadHeader(is), "expected header parsing to succeed");

		// sanity checks
		t_assertm(mh.IsMultiPart() == false, "expected no multipart");
		t_assertm(mh.GetBoundary().Length() == 0, "expected no multipart seperator");
		t_assertm(mh.GetContentLength() == -1, "expected -1, since field is not set");
		assertEqualm("", mh.Lookup("NotThere", ""), "expected 'NotThere' to be emtpy");

		assertAnyEqual(result, mh.GetInfo());
	}

	Anything result1;
	// test a simple header
	testinput =
		"Connection: Keep-Alive\r\n"
		"User-Agent: Mozilla/4.7 [en] (WinNT; U)\r\n"
		"Host: sentosa.hsr.ch:2929\r\n"
		"Set-Cookie2: Test1=test_value1; expires=Sat, 01-Jan-2001 01:01:01 GMT; path=/;\r\n"
		"Set-Cookie2: Test2=test_value2; expires=Sat, 02-Jan-2002 02:02:02 GMT; path=/;\r\n"
		"Set-Cookie2: Test3=test_value3; expires=Sat, 03-Jan-2003 03:03:03 GMT; path=/;\r\n"
		"\r\n";
	Trace("TestInput: <" << testinput << ">");
	result1["CONNECTION"] = "Keep-Alive";
	result1["USER-AGENT"] = "Mozilla/4.7 [en] (WinNT; U)";
	result1["HOST"] = "sentosa.hsr.ch:2929";
	result1["SET-COOKIE2"][0L] = "Test1=test_value1; expires=Sat, 01-Jan-2001 01:01:01 GMT; path=/;";
	result1["SET-COOKIE2"][1L] = "Test2=test_value2; expires=Sat, 02-Jan-2002 02:02:02 GMT; path=/;";
	result1["SET-COOKIE2"][2L] = "Test3=test_value3; expires=Sat, 03-Jan-2003 03:03:03 GMT; path=/;";

	{
		MIMEHeader mh;
		StringStream is(testinput);

		// basic checks of success
		t_assertm(mh.DoReadHeader(is), "expected header parsing to succeed");

		// sanity checks
		t_assertm(mh.IsMultiPart() == false, "expected no multipart");
		t_assertm(mh.GetBoundary().Length() == 0, "expected no multipart seperator");
		t_assertm(mh.GetContentLength() == -1, "expected -1, since field is not set");
		assertEqualm("", mh.Lookup("NotThere", ""), "expected 'NotThere' to be emtpy");

		assertAnyEqual(result1, mh.GetInfo());
	}
}

void MIMEHeaderTest::MultiPartHeaderTest()
{
	StartTrace(MIMEHeaderTest.MultiPartHeaderTest);

	String testinput, testinput1;

	// test a simple header
	testinput =
		"Referer: http://sentosa.hsr.loc:1919/birrer/htdocs/uploadTest.html\r\n"
		"Connection: Keep-Alive\r\n"
		"User-Agent: Mozilla/4.7 [en] (WinNT; U)\r\n"
		"Host: sentosa.hsr.loc:2929\r\n"
		"Accept: image/gif, image/x-xbitmap, image/jpeg, image/pjpeg, image/png, */*\r\n"
		"Accept-Encoding: gzip\r\n"
		"Accept-Language: de-CH,en\r\n"
		"Accept-Charset: iso-8859-1,*,utf-8\r\n"
		"Cookie: FDState=b64:YnMwOgcSizsIZcv8pnvq7; FINXSCUSTNO=CH10601\r\n"
		"Content-type: multipart/form-data; boundary=---------------------------61400283883149348477195787\r\n"
		"Content-Length: 542\r\n"
		"\r\n";

	// Accept-Encoding appears twice and contains field delimiter (,)
	testinput1 =
		"Referer: http://sentosa.hsr.loc:1919/birrer/htdocs/uploadTest.html\r\n"
		"Connection: Keep-Alive\r\n"
		"User-Agent: Mozilla/4.7 [en] (WinNT; U)\r\n"
		"Host: sentosa.hsr.loc:2929\r\n"
		"Accept: image/gif, image/x-xbitmap, image/jpeg, image/pjpeg, image/png, */*\r\n"
		"Accept-Encoding: gzip\r\n"
		"Accept-Encoding: super-gzip,super-gzip-param=1,2.3\r\n"
		"Accept-Language: de-CH,en\r\n"
		"Accept-Charset: iso-8859-1,*,utf-8\r\n"
		"Cookie: FDState=b64:YnMwOgcSizsIZcv8pnvq7; FINXSCUSTNO=CH10601\r\n"
		"Content-type: multipart/form-data; boundary=---------------------------61400283883149348477195787\r\n"
		"Content-Length: 542\r\n"
		"\r\n";

	Trace("TestInput: <" << testinput << ">");
	Anything result, result1, result2, result3;

	result["REFERER"] = "http://sentosa.hsr.loc:1919/birrer/htdocs/uploadTest.html";
	result["CONNECTION"] = "Keep-Alive";
	result["USER-AGENT"] = "Mozilla/4.7 [en] (WinNT; U)";
	result["HOST"] = "sentosa.hsr.loc:2929";
	result["ACCEPT"].Append("image/gif");
	result["ACCEPT"].Append("image/x-xbitmap");
	result["ACCEPT"].Append("image/jpeg");
	result["ACCEPT"].Append("image/pjpeg");
	result["ACCEPT"].Append("image/png");
	result["ACCEPT"].Append("*/*");
	result["ACCEPT-ENCODING"] = "gzip";
	result["ACCEPT-LANGUAGE"].Append("de-CH");
	result["ACCEPT-LANGUAGE"].Append("en");
	result["ACCEPT-CHARSET"].Append("iso-8859-1");
	result["ACCEPT-CHARSET"].Append("*");
	result["ACCEPT-CHARSET"].Append("utf-8");
	result["COOKIE"]["FDState"] = 	"b64:YnMwOgcSizsIZcv8pnvq7";
	result["COOKIE"]["FINXSCUSTNO"] = "CH10601";
	result["CONTENT-TYPE"] = "multipart/form-data; boundary=---------------------------61400283883149348477195787";
	result["CONTENT-LENGTH"] = "542";
	result["BOUNDARY"] = "---------------------------61400283883149348477195787";

	result1["REFERER"] = "http://sentosa.hsr.loc:1919/birrer/htdocs/uploadTest.html";
	result1["CONNECTION"] = "Keep-Alive";
	result1["USER-AGENT"] = "Mozilla/4.7 [en] (WinNT; U)";
	result1["HOST"] = "sentosa.hsr.loc:2929";
	result1["ACCEPT"].Append("image/gif");
	result1["ACCEPT"].Append("image/x-xbitmap");
	result1["ACCEPT"].Append("image/jpeg");
	result1["ACCEPT"].Append("image/pjpeg");
	result1["ACCEPT"].Append("image/png");
	result1["ACCEPT"].Append("*/*");
	result1["ACCEPT-ENCODING"].Append("gzip");
	result1["ACCEPT-ENCODING"].Append("super-gzip");
	result1["ACCEPT-ENCODING"].Append("super-gzip-param=1");
	result1["ACCEPT-ENCODING"].Append("2.3");
	result1["ACCEPT-LANGUAGE"].Append("de-CH");
	result1["ACCEPT-LANGUAGE"].Append("en");
	result1["ACCEPT-CHARSET"].Append("iso-8859-1");
	result1["ACCEPT-CHARSET"].Append("*");
	result1["ACCEPT-CHARSET"].Append("utf-8");
	result1["COOKIE"]["FDState"] = 	"b64:YnMwOgcSizsIZcv8pnvq7";
	result1["COOKIE"]["FINXSCUSTNO"] = "CH10601";
	result1["CONTENT-TYPE"] = "multipart/form-data; boundary=---------------------------61400283883149348477195787";
	result1["CONTENT-LENGTH"] = "542";
	result1["BOUNDARY"] = "---------------------------61400283883149348477195787";

	result2["REFERER"] = "http://sentosa.hsr.loc:1919/birrer/htdocs/uploadTest.html";
	result2["CONNECTION"] = "Keep-Alive";
	result2["USER-AGENT"] = "Mozilla/4.7 [en] (WinNT; U)";
	result2["HOST"] = "sentosa.hsr.loc:2929";
	result2["ACCEPT"] =  "image/gif, image/x-xbitmap, image/jpeg, image/pjpeg, image/png, */*";
	result2["ACCEPT-ENCODING"] =  "gzip";
	result2["ACCEPT-LANGUAGE"] =  "de-CH,en";
	result2["ACCEPT-CHARSET"] =  "iso-8859-1,*,utf-8";
	result2["COOKIE"]["FDState"] = 	"b64:YnMwOgcSizsIZcv8pnvq7";
	result2["COOKIE"]["FINXSCUSTNO"] = "CH10601";
	result2["CONTENT-TYPE"] = "multipart/form-data; boundary=---------------------------61400283883149348477195787";
	result2["CONTENT-LENGTH"] = "542";
	result2["BOUNDARY"] = "---------------------------61400283883149348477195787";

	result3["REFERER"] = "http://sentosa.hsr.loc:1919/birrer/htdocs/uploadTest.html";
	result3["CONNECTION"] = "Keep-Alive";
	result3["USER-AGENT"] = "Mozilla/4.7 [en] (WinNT; U)";
	result3["HOST"] = "sentosa.hsr.loc:2929";
	result3["ACCEPT"] =  "image/gif, image/x-xbitmap, image/jpeg, image/pjpeg, image/png, */*";
	result3["ACCEPT-ENCODING"].Append("gzip");
	result3["ACCEPT-ENCODING"].Append("super-gzip,super-gzip-param=1,2.3");
	result3["ACCEPT-LANGUAGE"] =  "de-CH,en";
	result3["ACCEPT-CHARSET"] =  "iso-8859-1,*,utf-8";
	result3["COOKIE"]["FDState"] = 	"b64:YnMwOgcSizsIZcv8pnvq7";
	result3["COOKIE"]["FINXSCUSTNO"] = "CH10601";
	result3["CONTENT-TYPE"] = "multipart/form-data; boundary=---------------------------61400283883149348477195787";
	result3["CONTENT-LENGTH"] = "542";
	result3["BOUNDARY"] = "---------------------------61400283883149348477195787";

	{
		MIMEHeader mh;
		StringStream is(testinput);

		// basic checks of success
		t_assertm(mh.DoReadHeader(is), "expected header parsing to succeed");

		// sanity checks
		t_assertm(mh.IsMultiPart() == true, "expected to be multipart");
		assertEqualm("---------------------------61400283883149348477195787", mh.GetBoundary(), "expected multipart seperator");
		t_assertm(mh.GetContentLength() == 542, "expected 542, since field should be set");
		assertEqualm("", mh.Lookup("NotThere", ""), "expected 'NotThere' to be emtpy");

		assertAnyEqual(result, mh.GetInfo());
	}
	{
		MIMEHeader mh;
		StringStream is(testinput1);

		// basic checks of success
		t_assertm(mh.DoReadHeader(is), "expected header parsing to succeed");

		// sanity checks
		t_assertm(mh.IsMultiPart() == true, "expected to be multipart");
		assertEqualm("---------------------------61400283883149348477195787", mh.GetBoundary(), "expected multipart seperator");
		t_assertm(mh.GetContentLength() == 542, "expected 542, since field should be set");
		assertEqualm("", mh.Lookup("NotThere", ""), "expected 'NotThere' to be emtpy");

		assertAnyEqual(result1, mh.GetInfo());
	}
	{
		MIMEHeader mh(URLUtils::eUpshift, MIMEHeader::eDoNotSplitHeaderFields);
		StringStream is(testinput);

		// basic checks of success
		t_assertm(mh.DoReadHeader(is), "expected header parsing to succeed");

		// sanity checks
		t_assertm(mh.IsMultiPart() == true, "expected to be multipart");
		assertEqualm("---------------------------61400283883149348477195787", mh.GetBoundary(), "expected multipart seperator");
		t_assertm(mh.GetContentLength() == 542, "expected 542, since field should be set");
		assertEqualm("", mh.Lookup("NotThere", ""), "expected 'NotThere' to be emtpy");

		assertAnyEqual(result2, mh.GetInfo());
	}
	{
		MIMEHeader mh(URLUtils::eUpshift, MIMEHeader::eDoNotSplitHeaderFields);
		StringStream is(testinput1);

		// basic checks of success
		t_assertm(mh.DoReadHeader(is), "expected header parsing to succeed");

		// sanity checks
		t_assertm(mh.IsMultiPart() == true, "expected to be multipart");
		assertEqualm("---------------------------61400283883149348477195787", mh.GetBoundary(), "expected multipart seperator");
		t_assertm(mh.GetContentLength() == 542, "expected 542, since field should be set");
		assertEqualm("", mh.Lookup("NotThere", ""), "expected 'NotThere' to be emtpy");

		assertAnyEqual(result3, mh.GetInfo());
	}

}

void MIMEHeaderTest::PartHeaderTest()
{
	StartTrace(MIMEHeaderTest.PartHeaderTest);

	String testinput;
	MIMEHeader mh;

	// test a multipart/form-data part header
	testinput =
		"Content-Disposition: form-data; name=Datei; "
		"filename=G:\\DEVELOP\\coast\\wdbase\\Application.h\r\n"
		"Content-Type: application/x-unknown-content-type-hfile\r\n"
		"\r\n";

	Trace("TestInput: <" << testinput << ">");
	Anything result;
	result["CONTENT-DISPOSITION"][0L] = "form-data";
	result["CONTENT-DISPOSITION"]["NAME"] = "Datei";
	result["CONTENT-DISPOSITION"]["FILENAME"] = "G:\\DEVELOP\\coast\\wdbase\\Application.h";
	result["CONTENT-TYPE"] = "application/x-unknown-content-type-hfile";

	{
		StringStream is(testinput);

		// basic checks of success
		t_assertm(mh.DoReadHeader(is), "expected header parsing to succeed");

		// sanity checks
		t_assertm(mh.IsMultiPart() == false, "expected not to be multipart");
		assertEqualm("", mh.GetBoundary(), "expected no multipart seperator");
		t_assertm(mh.GetContentLength() == -1, "expected -1, since field is not set");
		assertEqualm("", mh.Lookup("NotThere", ""), "expected 'NotThere' to be emtpy");
		assertEqualm("application/x-unknown-content-type-hfile", mh.Lookup("CONTENT-TYPE", ""), "expected Content-type to be set");
		assertAnyEqual(result, mh.GetInfo());
	}
}

// builds up a suite of testcases, add a line for each testmethod
Test *MIMEHeaderTest::suite ()
{
	StartTrace(MIMEHeaderTest.suite);
	TestSuite *testSuite = new TestSuite;

	testSuite->addTest (NEW_CASE(MIMEHeaderTest, SimpleHeaderTest));
	testSuite->addTest (NEW_CASE(MIMEHeaderTest, SetCookieTest));
	testSuite->addTest (NEW_CASE(MIMEHeaderTest, MultiPartHeaderTest));
	testSuite->addTest (NEW_CASE(MIMEHeaderTest, PartHeaderTest));

	return testSuite;

} // suite
