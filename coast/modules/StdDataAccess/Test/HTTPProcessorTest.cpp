/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

//--- standard modules used ----------------------------------------------------
#include "StringStream.h"
#include "StringStreamSocket.h"
#include "System.h"
#include "Context.h"
#include "URLUtils.h"
#include "MIMEHeader.h"
#include "RequestBodyParser.h"
#include "Dbg.h"
#include "Server.h"

//--- test modules used --------------------------------------------------------
#include "TestSuite.h"

//--- interface include --------------------------------------------------------
#include "HTTPProcessorTest.h"

static String common =
	String("") << "</head><body bgcolor=\"silver\">\n"
	<< "<center>\n<h1>Page not found.</h1>\nPress the back button to return to the previous page!<br><br>\n"
	<< "<FORM><input type=button value=\"Back\" onClick=\"javascript:history.back(1)\"></FORM>\n</center>\n</body></html>";

//---- HTTPProcessorTest ----------------------------------------------------------------
HTTPProcessorTest::HTTPProcessorTest(TString tname)
	: TestCaseType(tname)
{
	StartTrace(HTTPProcessorTest.HTTPProcessorTest);
}

TString HTTPProcessorTest::getConfigFileName()
{
	return "HTTPProcessorTestConfig";
}

HTTPProcessorTest::~HTTPProcessorTest()
{
	StartTrace(HTTPProcessorTest.Dtor);
}

void HTTPProcessorTest::setUp ()
{
	StartTrace(HTTPProcessorTest.setUp);

	fArgTemplate["env"]["REMOTE_ADDR"] = "127.0.0.1";
	fArgTemplate["env"]["HTTP_USER_AGENT"] = "Testframework";
	fArgTemplate["query"]["adr"] = "127.0.0.2";
	fArgTemplate["query"]["port"] = 2412;
}

void HTTPProcessorTest::DoReadInputWithErrorTest()
{
	StartTrace(HTTPProcessorTest.DoReadInputWithErrorTest);
	HTTPProcessor fds("test");
	String expected = "";

	fds.fRequestSizeLimit = 5120L;
	fds.fLineSizeLimit = 1024L;
	fds.fURISizeLimit = 1024L;
	testDoReadInputWithCfg( fds, expected );

	expected =    String("") << "HTTP/1.1 414 Request-URL Too Large" << ENDL
				  << "Connection: close\r\ncontent-type: text/html" << ENDL << ENDL
				  << "<html><head>\n<title>414 Request-URL Too Large</title>\n"
				  << common;
	fds.fRequestSizeLimit = 5120L;
	fds.fLineSizeLimit = 1024L;
	fds.fURISizeLimit = 20L;
	testDoReadInputWithCfg( fds, expected, false );

	expected =    String("") << "HTTP/1.1 413 Request Entity Too Large" << ENDL
				  << "Connection: close\r\ncontent-type: text/html" << ENDL << ENDL
				  << "<html><head>\n<title>413 Request Entity Too Large</title>\n"
				  << common;
	fds.fRequestSizeLimit = 5120L;
	fds.fLineSizeLimit = 30L;
	fds.fURISizeLimit = 120L;
	testDoReadInputWithCfg( fds, expected, false);

	expected =    String("") << "HTTP/1.1 413 Request Entity Too Large" << ENDL
				  << "Connection: close\r\ncontent-type: text/html" << ENDL << ENDL
				  << "<html><head>\n<title>413 Request Entity Too Large</title>\n"
				  << common;
	fds.fRequestSizeLimit = 5120L;
	fds.fLineSizeLimit = 20L;
	fds.fURISizeLimit = 20L;
	testDoReadInputWithCfg( fds, expected, false );

	expected =    String("") << "HTTP/1.1 413 Request Entity Too Large" << ENDL
				  << "Connection: close\r\ncontent-type: text/html" << ENDL << ENDL
				  << "<html><head>\n<title>413 Request Entity Too Large</title>\n"
				  << common;
	fds.fRequestSizeLimit = 200L;
	fds.fLineSizeLimit = 1024L;
	fds.fURISizeLimit = 1024L;
	testDoReadInputWithCfg( fds, expected, false );

}

void HTTPProcessorTest::testDoReadInputWithCfg( HTTPProcessor &fds, String expected, const bool expectedRequest )
{
	StartTrace(HTTPProcessorTest.testDoReadInputWithCfg);
	Context ctx;
	Trace("fRequestSizeLimit:" << fds.fRequestSizeLimit << ":");
	Trace("fLineSizeLimit:" << fds.fLineSizeLimit << ":");
	Trace("fURISizeLimit:" << fds.fURISizeLimit << ":");

	String uri;
	uri = 	"GET /hallo123456 HTTP/1.0\r\n"
			"Connection: Keep-Alive\r\n"
			"User-Agent: Mozilla/4.06 [en] (X11; U; SunOS 5.6 sun4u)\r\n"
			"Host: sentosa.hsr.loc:1919\r\n"
			"Accept: image/gif, image/x-xbitmap, image/jpeg, image/pjpeg, image/png, */*\r\n"
			"Accept-Encoding: gzip\r\n"
			"Accept-Language: en\r\n"
			"Accept-Charset: iso-8859-1,*,utf-8\r\n"
			"Cookie: FDState=b64:YnMwOh8tQnIEs1uC2rU3V3fpDOlWqwZeyeSV8xcTT9-m55gkcdmUwEAX5yBjiXEiji1cn63s81FKkm7fz0Sjwnxg-DX7awEQ723P7SKNLpzc4RLj3QBa5t7t6rKawa-X8Y-PYBnJvNByMH3jWcBY3ewDGXM=; FINXSCUSTNO=CH10601\r\n";

	Anything env;

	StringStreamSocket ss(uri);
	fds.DoReadInput(*(ss.GetStream()), ctx);
	String reply, fullreply;
	BuildResult(reply, fullreply, uri);
	assertEqual( expected, reply );

	Anything result = ctx.GetRequest();
	t_assert(expectedRequest == result.LookupPath(env, "env"));
	t_assert(expectedRequest == result.IsDefined("query"));
	TraceAny(env, "env");
	TraceAny(result, "result1");
}

void HTTPProcessorTest::BuildResult( String &reply, String &fullreply, String result )
{
	StartTrace(HTTPProcessorTest.BuildResult);
	fullreply << reply << result;
	long pos = fullreply.Contains("</html>" );
	if (pos >= 0) {

		Trace("pos" << pos << ":");
		pos += 7;
		reply = fullreply.SubString(0L, pos);
	}
	Trace("fullreply: " << fullreply);
	Trace("reply: " 	<< reply);
}

void HTTPProcessorTest::DoReadInputTest()
{
	Context ctx;
	HTTPProcessor fds("test");

	String uri1;
	uri1 = 	"GET / HTTP/1.0\r\n"
			"Connection: Keep-Alive\r\n"
			"User-Agent: Mozilla/4.06 [en] (X11; U; SunOS 5.6 sun4u)\r\n"
			"Host: sentosa.hsr.loc:1919\r\n"
			"Accept: image/gif, image/x-xbitmap, image/jpeg, image/pjpeg, image/png, */*\r\n"
			"Accept-Encoding: gzip\r\n"
			"Accept-Language: en\r\n"
			"Accept-Charset: iso-8859-1,*,utf-8\r\n"
			"Cookie: FDState=b64:YnMwOh8tQnIEs1uC2rU3V3fpDOlWqwZeyeSV8xcTT9-m55gkcdmUwEAX5yBjiXEiji1cn63s81FKkm7fz0Sjwnxg-DX7awEQ723P7SKNLpzc4RLj3QBa5t7t6rKawa-X8Y-PYBnJvNByMH3jWcBY3ewDGXM=; FINXSCUSTNO=CH10601\r\n";
	Anything result1;
	Anything env;

	StringStream is1(&uri1);
	fds.DoReadInput(is1, ctx);
	result1 = ctx.GetRequest();
	t_assert(result1.LookupPath(env, "env"));
	t_assert(result1.IsDefined("query"));

	Anything item;

	t_assert(env.LookupPath(item, "REQUEST_METHOD"));
	assertEqual("GET", item.AsCharPtr(""));

	t_assert(env.LookupPath(item, "REQUEST_URI"));
	assertEqual("/", item.AsCharPtr(""));

	t_assert(env.LookupPath(item, "SERVER_PROTOCOL"));
	assertEqual("HTTP/1.0", item.AsCharPtr(""));

	t_assert(env.LookupPath(item, "header.CONNECTION"));
	assertEqual("Keep-Alive", item.AsCharPtr(""));

	t_assert(env.LookupPath(item, "header.USER-AGENT"));
	assertEqual("Mozilla/4.06 [en] (X11; U; SunOS 5.6 sun4u)", item.AsCharPtr(""));

	t_assert(env.LookupPath(item, "header.HOST"));
	assertEqual("sentosa.hsr.loc:1919", item.AsCharPtr(""));

	t_assert(env.LookupPath(item, "header.COOKIE"));
	Anything result;
	result["FDState"] = 	"b64:YnMwOh8tQnIEs1uC2rU3V3fpDOlWqwZeyeSV8xcTT9-"
							"m55gkcdmUwEAX5yBjiXEiji1cn63s81FKkm7fz0Sjwnxg-DX"
							"7awEQ723P7SKNLpzc4RLj3QBa5t7t6rKawa-X8Y-PYBnJvNBy"
							"MH3jWcBY3ewDGXM=";
	result["FINXSCUSTNO"] = "CH10601";
	assertAnyEqual(result, item);

	t_assert(env.LookupPath(item, "header.REMOTE-ADDR"));
	assertEqual("localhost", item.AsCharPtr(""));

	t_assert(env.LookupPath(item, "header.CONTENT-LENGTH"));
	assertEqual("0", item.AsCharPtr(""));

	String uri2;
	uri2 =	"POST /fda?X=b64:YnMwOjnX7XfCfhsxQ-zGeo9l3FXutcXQmypVV$bIM77lo2Bpm$jxzXT5U0Acg2XkIxFwcBwS8KFKsAhIZC45Uow9Zexx8gNV8o7HyTyaGXRKL4ql HTTP/1.0" ENDL
			"Referer: https://sentosa.hsr.loc:2020/"
			"fda?X=b64:YnMwOpZ1bi1lgJb0S3c54qaMZfApOdV9RnF4HfbIM77lo2Bpm$"
			"jxzXT5U0Acg2XkIxFwcBwS8KFKsAhIZC45Uow9Zexx8gNV8o7HyaK5$RoW8E"
			"2vZ1QqXEF$MQRiDd6g4Qb107gGPNEy6vr3pYZtdXBnJpc=" ENDL
			"Connection: Keep-Alive" ENDL
			"User-Agent: Mozilla/4.06 [en] (X11; U; SunOS 5.6 sun4u)" ENDL
			"Host: sentosa.hsr.loc:2020" ENDL
			"Accept: image/gif, image/x-xbitmap, image/jpeg, image/pjpeg, image/png, */*" ENDL
			"Accept-Encoding: gzip" ENDL
			"Accept-Language: en" ENDL
			"Accept-Charset: iso-8859-1,*,utf-8" ENDL
			"Cookie: FDState=b64:YnMwOsbP70f8fX3Gai-aMy5tiaAwtnNHIKR82hcTT9-m55"
			"gkcdmUwEAX5yBjiXEiji1cn63s81FKkm7f8QLITZy6d7UUYDrQsGR80QK21q8f"
			"vV1Of9UjJG52ELWawa-X8Y-PYBnJvNByMH3jWcBY3ewDGXM=" ENDL
			"Content-type: application/x-www-form-urlencoded" ENDL
			"Content-length: 148" ENDL ENDL
			"fld_TKCountryCode=CH&fld_TKCustNumber=10601"
			"&fld_TKUserID=tk+bul&fld_TKPassword=gaef"
			"&b_DoLoginAction=Login&fld_origSessionId=none&fld_origDelayedIn=0" ENDL;

	Anything result2;
	result2["ClientInfo"]["REMOTE_ADDR"] = "128.34.125.23";
	result2["ClientInfo"]["HTTPS"] = true;
	ctx.PushRequest(result2);

	StringStream is2(&uri2);
	fds.DoReadInput(is2, ctx);

	t_assert(result2.LookupPath(env, "env"));
	t_assert(result2.IsDefined("query"));

	t_assert(env.LookupPath(item, "REQUEST_METHOD"));
	assertEqual("POST", item.AsCharPtr(""));

	t_assert(env.LookupPath(item, "header.CONTENT-LENGTH"));
	assertEqual("148", item.AsCharPtr(""));

	t_assert(env.LookupPath(item, "header.REMOTE-ADDR"));
	assertEqual("128.34.125.23", item.AsCharPtr(""));
	t_assert(env.LookupPath(item, "header.REMOTE_ADDR"));
	assertEqual("128.34.125.23", item.AsCharPtr(""));
	t_assert(env.LookupPath(item, "header.HTTPS"));
	t_assert(item.AsBool(false));

	t_assert(env.LookupPath(item, "REQUEST_BODY"));
	assertEqual("CH", item["fld_TKCountryCode"].AsCharPtr(""));
	assertEqual("10601", item["fld_TKCustNumber"].AsCharPtr(""));
	assertEqual("tk bul", item["fld_TKUserID"].AsCharPtr(""));
	assertEqual("gaef", item["fld_TKPassword"].AsCharPtr(""));
	assertEqual("Login", item["b_DoLoginAction"].AsCharPtr(""));
	assertEqual("none", item["fld_origSessionId"].AsCharPtr(""));
	assertEqual("0", item["fld_origDelayedIn"].AsCharPtr(""));
	t_assert(env.LookupPath(item, "WHOLE_REQUEST_BODY"));
	String reqBody =	"fld_TKCountryCode=CH&fld_TKCustNumber=10601"
						"&fld_TKUserID=tk+bul&fld_TKPassword=gaef"
						"&b_DoLoginAction=Login&fld_origSessionId=none&fld_origDelayedIn=0";
	assertEqual(reqBody, item.AsCharPtr(""));

}

void HTTPProcessorTest::DoReadMinimalInputTest()
{
	StartTrace(HTTPProcessorTest.DoReadMinimalInputTest);
	Context ctx;
	HTTPProcessor fds("test");
	// ((errornous) Minimal request, the two blanks after GET are critically
	String uri;
	uri = 	"GET  HTTP/1.0\r\n\r\n";

	String expected =    String("") << "HTTP/1.1 400 Bad Request" << ENDL
						 << "Connection: close\r\ncontent-type: text/html" << ENDL << ENDL
						 << "<html><head>\n<title>400 Bad Request</title>\n"
						 << common;

	StringStreamSocket ss(uri);
	fds.DoReadInput(*(ss.GetStream()), ctx);
	String reply, fullreply;
	BuildResult(reply, fullreply, uri);
	assertEqual( expected, reply );
	Trace(uri);
	Trace(expected);
}

void HTTPProcessorTest::FileUploadTest()
{
	StartTrace(HTTPProcessorTest.FileUploadTest);

	HTTPProcessor hp("TestMe");
	Context ctx;
	String input(GetTestCaseConfig()["Request"].AsString());
	StringStream is1(input);
	hp.DoReadInput(is1, ctx);

	assertAnyEqualm(GetTestCaseConfig()["Results"]["REQUEST_BODY"], ctx.GetEnvStore()["REQUEST_BODY"], name());
}

void HTTPProcessorTest::RenderProtocolStatusWithoutHTTPStatus()
{
	StartTrace(HTTPProcessorTest.RenderProtocolStatus);

	HTTPProcessor hp("TestMe");
	Context ctx;
	OStringStream os1;
	hp.DoRenderProtocolStatus(os1, ctx);
	t_assert( os1.str() == "HTTP/1.1 200 OK\r\nConnection: close\r\n" );

	Anything tmpStore = ctx.GetTmpStore();
	tmpStore["HTTPStatus"]["ResponseCode"] = 599L;
	OStringStream os2;
	hp.DoRenderProtocolStatus(os2, ctx);
	t_assert( os2.str() == "HTTP/1.1 599 Unknown Error\r\nConnection: close\r\n" );

	tmpStore["HTTPStatus"]["ResponseCode"] = 413L;

	OStringStream os3;
	tmpStore["HTTPStatus"]["ResponseMsg"] = "BlaBla";

	hp.DoRenderProtocolStatus(os3, ctx);
	t_assert( os3.str() == "HTTP/1.1 413 BlaBla\r\nConnection: close\r\n" );
}

void HTTPProcessorTest::KeepConnection()
{
	StartTrace(HTTPProcessorTest.KeepConnection);

	HTTPProcessor hp("TestMe");
	Context ctx;
	Anything req;

	req["env"]["SERVER_PROTOCOL"] = "HTTP/1.1";
	req["env"]["header"]["CONNECTION"] = "Keep-Alive";
	ctx.PushRequest(req);
	t_assert(hp.DoKeepConnectionAlive(ctx));

	req["env"]["SERVER_PROTOCOL"] = "HTTP/1.1";
	req["env"]["header"]["CONNECTION"] = "Close";
	ctx.PushRequest(req);
	t_assert(!hp.DoKeepConnectionAlive(ctx));

	req["env"]["SERVER_PROTOCOL"] = "HTTP/1.0";
	req["env"]["header"]["CONNECTION"] = "Keep-Alive";
	ctx.PushRequest(req);
	t_assert(!hp.DoKeepConnectionAlive(ctx));

	req["env"]["SERVER_PROTOCOL"] = "HTTP/1.0";
	req["env"]["header"]["CONNECTION"] = "Close";
	ctx.PushRequest(req);
	t_assert(!hp.DoKeepConnectionAlive(ctx));
}

void HTTPProcessorTest::IsZipEncodingAcceptedByClientTest()
{
	ROAnything caseConfig;
	AnyExtensions::Iterator<ROAnything, ROAnything, TString> aEntryIterator(GetTestCaseConfig());
	while ( aEntryIterator.Next(caseConfig) ) {
		TString caseName;
		aEntryIterator.SlotName(caseName);
		Context ctx;
		ctx.GetTmpStore() = caseConfig["TmpStore"].DeepClone();
		ctx.GetRequest() = caseConfig["Request"].DeepClone();
		bool res = HTTPProcessor::IsZipEncodingAcceptedByClient(ctx);
		t_assertm(caseConfig["Expected"].AsBool() == res, caseName);
		assertAnyEqualm(caseConfig["Request"], ctx.GetRequest(), caseName);
	}
}

// builds up a suite of testcases, add a line for each testmethod
Test *HTTPProcessorTest::suite ()
{
	StartTrace(HTTPProcessorTest.suite);
	TestSuite *testSuite = new TestSuite;
	ADD_CASE(testSuite, HTTPProcessorTest, IsZipEncodingAcceptedByClientTest);
	ADD_CASE(testSuite, HTTPProcessorTest, DoReadInputTest);
	ADD_CASE(testSuite, HTTPProcessorTest, DoReadInputWithErrorTest);
	ADD_CASE(testSuite, HTTPProcessorTest, DoReadMinimalInputTest);
	ADD_CASE(testSuite, HTTPProcessorTest, FileUploadTest);
	ADD_CASE(testSuite, HTTPProcessorTest, RenderProtocolStatusWithoutHTTPStatus);
	ADD_CASE(testSuite, HTTPProcessorTest, KeepConnection);
	return testSuite;
}
