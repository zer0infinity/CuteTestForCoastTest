/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

//--- interface include --------------------------------------------------------
#include "HTTPProcessorTest.h"

#include "HTTPProcessor.h"

//--- test modules used --------------------------------------------------------
#include "TestSuite.h"

//--- standard modules used ----------------------------------------------------
#include "StringStream.h"
#include "StringStreamSocket.h"
#include "Context.h"
#include "URLUtils.h"
#include "MIMEHeader.h"
#include "HTTPPostRequestBodyParser.h"
#include "Dbg.h"
#include "Server.h"
#include "AnyLookupInterfaceAdapter.h"

namespace {
	const String common = "</head><body bgcolor=\"silver\">\n"
		"<center>\n<h1>Page not found.</h1>\nPress the back button to return to the previous page!<br><br>\n"
		"<FORM><input type=button value=\"Back\" onClick=\"javascript:history.back(1)\"></FORM>\n</center>\n</body></html>";
}

//---- HTTPProcessorTest ----------------------------------------------------------------
HTTPProcessorTest::HTTPProcessorTest(TString tname) :
	TestCaseType(tname) {
	StartTrace(HTTPProcessorTest.HTTPProcessorTest);
}

TString HTTPProcessorTest::getConfigFileName() {
	return "HTTPProcessorTestConfig";
}

HTTPProcessorTest::~HTTPProcessorTest() {
	StartTrace(HTTPProcessorTest.Dtor);
}

void HTTPProcessorTest::setUp() {
	StartTrace(HTTPProcessorTest.setUp);

	fArgTemplate["env"]["header"]["REMOTE_ADDR"] = "127.0.0.1";
	fArgTemplate["env"]["header"]["USER-AGENT"] = "Testframework";
	fArgTemplate["query"]["adr"] = "127.0.0.2";
	fArgTemplate["query"]["port"] = 2412;
}

void HTTPProcessorTest::DoReadInputWithErrorTest() {
	StartTrace(HTTPProcessorTest.DoReadInputWithErrorTest);

	RequestProcessor *httpProcessor = HTTPProcessor::FindRequestProcessor("HTTPProcessor");
	Anything anyTmpStore, anyRequest, env;

	String
			uri =
					"GET /hallo123456 HTTP/1.0" ENDL
						"Connection: Keep-Alive" ENDL
						"User-Agent: Mozilla/4.06 [en] (X11; U; SunOS 5.6 sun4u)" ENDL
						"Host: sentosa.hsr.loc:1919" ENDL
						"Accept: image/gif, image/x-xbitmap, image/jpeg, image/pjpeg, image/png, */*" ENDL
						"Accept-Encoding: gzip" ENDL
						"Accept-Language: en" ENDL
						"Accept-Charset: iso-8859-1,*,utf-8" ENDL
						"Cookie: FDState=b64:YnMwOh8tQnIEs1uC2rU3V3fpDOlWqwZeyeSV8xcTT9-m55gkcdmUwEAX5yBjiXEiji1cn63s81FKkm7fz0Sjwnxg-DX7awEQ723P7SKNLpzc4RLj3QBa5t7t6rKawa-X8Y-PYBnJvNByMH3jWcBY3ewDGXM=; FINXSCUSTNO=CH10601" ENDL ENDL;

	Anything anyParams;
	anyParams["RequestSizeLimit"] = 5120L;
	anyParams["LineSizeLimit"] = 1024L;
	anyParams["URISizeLimit"] = 1024L;
	String expected;

	anyTmpStore = DoReadMinimalInputTestHelper(httpProcessor, anyParams, uri, expected, anyRequest);
	assertCompare(true, equal_to, anyRequest.IsDefined("query"));

	expected = "HTTP/1.1 414 Request-URL Too Large" ENDL "Connection: close" ENDL "content-type: text/html" ENDL ENDL "<html><head>\n<title>414 Request-URL Too Large</title>\n";
	expected << common;

	anyParams["RequestSizeLimit"] = 5120L;
	anyParams["LineSizeLimit"] = 1024L;
	anyParams["URISizeLimit"] = 20L;
	anyTmpStore = DoReadMinimalInputTestHelper(httpProcessor, anyParams, uri, expected, anyRequest);
	assertCompare(false, equal_to, anyRequest.IsDefined("query"));

	expected = "HTTP/1.1 413 Request Entity Too Large" ENDL "Connection: close" ENDL "content-type: text/html" ENDL ENDL "<html><head>\n<title>413 Request Entity Too Large</title>\n";
	expected << common;

	anyParams["RequestSizeLimit"] = 5120L;
	anyParams["LineSizeLimit"] = 30L;
	anyParams["URISizeLimit"] = 120L;
	anyTmpStore = DoReadMinimalInputTestHelper(httpProcessor, anyParams, uri, expected, anyRequest);
	assertCompare(false, equal_to, anyRequest.IsDefined("query"));

	expected = "HTTP/1.1 413 Request Entity Too Large" ENDL "Connection: close" ENDL "content-type: text/html" ENDL ENDL "<html><head>\n<title>413 Request Entity Too Large</title>\n";
	expected << common;

	anyParams["RequestSizeLimit"] = 5120L;
	anyParams["LineSizeLimit"] = 20L;
	anyParams["URISizeLimit"] = 20L;
	anyTmpStore = DoReadMinimalInputTestHelper(httpProcessor, anyParams, uri, expected, anyRequest);
	assertCompare(false, equal_to, anyRequest.IsDefined("query"));

	expected = "HTTP/1.1 413 Request Entity Too Large" ENDL "Connection: close" ENDL "content-type: text/html" ENDL ENDL "<html><head>\n<title>413 Request Entity Too Large</title>\n";
	expected << common;

	anyParams["RequestSizeLimit"] = 200L;
	anyParams["LineSizeLimit"] = 1024L;
	anyParams["URISizeLimit"] = 1024L;
	anyTmpStore = DoReadMinimalInputTestHelper(httpProcessor, anyParams, uri, expected, anyRequest);
	assertCompare(false, equal_to, anyRequest.IsDefined("query"));
}

void HTTPProcessorTest::BuildResult(String &reply, String &fullreply, String result) {
	StartTrace(HTTPProcessorTest.BuildResult);
	fullreply << reply << result;
	long pos = fullreply.Contains("</html>");
	if (pos >= 0) {

		Trace("pos" << pos << ":");
		pos += 7;
		reply = fullreply.SubString(0L, pos);
	}
	Trace("fullreply: " << fullreply);
	Trace("reply: " << reply);
}

void HTTPProcessorTest::DoReadInputTest() {
	RequestProcessor *httpProcessor = HTTPProcessor::FindRequestProcessor("HTTPProcessor");
	{
		String
				uri =
						"GET / HTTP/1.0" ENDL
							"Connection: Keep-Alive" ENDL
							"User-Agent: Mozilla/4.06 [en] (X11; U; SunOS 5.6 sun4u)" ENDL
							"Host: sentosa.hsr.loc:1919" ENDL
							"Accept: image/gif, image/x-xbitmap, image/jpeg, image/pjpeg, image/png, */*" ENDL
							"Accept-Encoding: gzip" ENDL
							"Accept-Language: en" ENDL
							"Accept-Charset: iso-8859-1,*,utf-8" ENDL
							"Cookie: FDState=b64:YnMwOh8tQnIEs1uC2rU3V3fpDOlWqwZeyeSV8xcTT9-m55gkcdmUwEAX5yBjiXEiji1cn63s81FKkm7fz0Sjwnxg-DX7awEQ723P7SKNLpzc4RLj3QBa5t7t6rKawa-X8Y-PYBnJvNByMH3jWcBY3ewDGXM=; FINXSCUSTNO=CH10601" ENDL ENDL;

		StringStreamSocket ss(uri);
		Context ctx(&ss);

		httpProcessor->ProcessRequest(ctx);

		Anything item, env, result1 = ctx.GetRequest();

		t_assert(result1.LookupPath(env, "env"));
		t_assert(result1.IsDefined("query"));

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
		result["FDState"] = "b64:YnMwOh8tQnIEs1uC2rU3V3fpDOlWqwZeyeSV8xcTT9-"
			"m55gkcdmUwEAX5yBjiXEiji1cn63s81FKkm7fz0Sjwnxg-DX"
			"7awEQ723P7SKNLpzc4RLj3QBa5t7t6rKawa-X8Y-PYBnJvNBy"
			"MH3jWcBY3ewDGXM=";
		result["FINXSCUSTNO"] = "CH10601";
		assertAnyEqual(result, item);

		t_assert(env.LookupPath(item, "header.REMOTE_ADDR"));
		assertEqual("localhost", item.AsCharPtr(""));

		t_assert(env.LookupPath(item, "header.CONTENT-LENGTH"));
		assertEqual("0", item.AsCharPtr(""));
	}
	{
		String
				uri =
						"POST /fda?X=b64:YnMwOjnX7XfCfhsxQ-zGeo9l3FXutcXQmypVV$bIM77lo2Bpm$jxzXT5U0Acg2XkIxFwcBwS8KFKsAhIZC45Uow9Zexx8gNV8o7HyTyaGXRKL4ql HTTP/1.0" ENDL
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

		StringStreamSocket ss(uri);
		Context ctx(&ss);

		Anything item, env, result2;
		result2["ClientInfo"]["REMOTE_ADDR"] = "128.34.125.23";
		result2["ClientInfo"]["HTTPS"] = true;
		ctx.PushRequest(result2);

		httpProcessor->ProcessRequest(ctx);

		t_assert(result2.LookupPath(env, "env"));
		t_assert(result2.IsDefined("query"));

		t_assert(env.LookupPath(item, "REQUEST_METHOD"));
		assertEqual("POST", item.AsCharPtr(""));

		t_assert(env.LookupPath(item, "header.CONTENT-LENGTH"));
		assertEqual("148", item.AsCharPtr(""));

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
		String reqBody = "fld_TKCountryCode=CH&fld_TKCustNumber=10601"
			"&fld_TKUserID=tk+bul&fld_TKPassword=gaef"
			"&b_DoLoginAction=Login&fld_origSessionId=none&fld_origDelayedIn=0";
		assertEqual(reqBody, item.AsCharPtr(""));
	}
}

void HTTPProcessorTest::DoReadMinimalInputTest() {
	StartTrace(HTTPProcessorTest.DoReadMinimalInputTest);
	// ((errornous) Minimal request, the two blanks after GET are critically
	RequestProcessor *httpProcessor = HTTPProcessor::FindRequestProcessor("HTTPProcessor");
	String expected = "HTTP/1.1 400 Bad Request" ENDL "Connection: close" ENDL "content-type: text/html" ENDL ENDL "<html><head>\n<title>400 Bad Request</title>\n";
	expected.Append(common);
	Anything anyParams, anyRequest;
	anyParams["CheckHeaderFields"] = true;
	anyParams["RejectRequestsWithInvalidHeaders"] = true;

	// Form (post) request containing evil header
	String uri = "POST /alibaba HTTP/1.0" ENDL "X-Evil: GET /gaga HTTP/1.0" ENDL "Content-type: application/x-www-form-urlencoded" ENDL "key=value&Content-length: 11" ENDL ENDL;
	DoReadMinimalInputTestHelper(httpProcessor, anyParams, uri, expected, anyRequest);

	uri = "GET  HTTP/1.0" ENDL ENDL;
	DoReadMinimalInputTestHelper(httpProcessor, anyParams, uri, expected, anyRequest);
	uri = "GET /alibaba HTTP/1.0" ENDL "X-Evil: POST" ENDL ENDL;
	DoReadMinimalInputTestHelper(httpProcessor, anyParams, uri, expected, anyRequest);
	uri = "GET /alibaba HTTP/1.0" ENDL "X-Evil: GET" ENDL ENDL;
	DoReadMinimalInputTestHelper(httpProcessor, anyParams, uri, expected, anyRequest);
	uri = "GET /alibaba HTTP/1.0" ENDL "X-Evil: xGET" ENDL ENDL;
	expected.Trim(0L);
	DoReadMinimalInputTestHelper(httpProcessor, anyParams, uri, expected, anyRequest);

	// Mime (post) request containing evil header
	uri
			= "POST /alibaba HTTP/1.0" ENDL "Content-type: multipart/form-data; boundary=me;" ENDL "Content-length: 64" ENDL ENDL "--me" ENDL "X-Evil: GET /gaga HTTP/1.0" ENDL "This is the content." ENDL "--me--" ENDL;
	expected.Trim(0L);
	Anything tmpStore;
	tmpStore = DoReadMinimalInputTestHelper(httpProcessor, anyParams, uri, expected, anyRequest);
	TraceAny(tmpStore, "tmpStore");
	assertEqual("Possible SSL Renegotiation attack. A multipart mime header (in POST) contains a GET/POST request",
			tmpStore["ReadRequestBodyError"]["HTTPPostRequestBodyParser"]["Reason"].AsString());

	// Log header attack only
	anyParams["CheckHeaderFields"] = true;
	anyParams["RejectRequestsWithInvalidHeaders"] = false;
	uri = "GET /alibaba HTTP/1.0" ENDL "X-Evil: GET" ENDL ENDL;
	expected.Trim(0L);
	DoReadMinimalInputTestHelper(httpProcessor, anyParams, uri, expected, anyRequest);
}

Anything HTTPProcessorTest::DoReadMinimalInputTestHelper(RequestProcessor *httpProcessor, Anything const& anyParams, String uri,
		String const& expected, Anything &anyRequest) {
	StartTrace(HTTPProcessorTest.DoReadMinimalInputTestHelper);
	TraceAny(anyParams, "params");

	Trace(uri);
	AnyLookupInterfaceAdapter<Anything> lia(anyParams);
	StringStreamSocket ss(uri);
	Context ctx(&ss);
	ctx.Push("tempargs", &lia);

	httpProcessor->ProcessRequest(ctx);

	String reply, fullreply;

	BuildResult(reply, fullreply, uri);
	Trace("expected [" << expected << "]");
	assertCharPtrEqual( expected, reply );

	anyRequest = ctx.GetRequest();
	TraceAny(anyRequest, "request");
	TraceAny(ctx.GetTmpStore(), "tmpStore");
	return ctx.GetTmpStore();
}

void HTTPProcessorTest::FileUploadTest() {
	StartTrace(HTTPProcessorTest.FileUploadTest);
	RequestProcessor *httpProcessor = HTTPProcessor::FindRequestProcessor("HTTPProcessor");
	String uri = GetTestCaseConfig()["Request"].AsString();
	StringStreamSocket ss(uri);
	Context ctx(&ss);
	httpProcessor->ProcessRequest(ctx);
	assertAnyEqualm(GetTestCaseConfig()["Results"]["REQUEST_BODY"], ctx.GetEnvStore()["REQUEST_BODY"], name());
}

void HTTPProcessorTest::RenderProtocolStatusWithoutHTTPStatus() {
	StartTrace(HTTPProcessorTest.RenderProtocolStatus);

	RequestProcessor *httpProcessor = HTTPProcessor::FindRequestProcessor("HTTPProcessor");
	Context ctx;
	Anything anyProcessorName = httpProcessor->GetName();
	Context::PushPopEntry<Anything> aRPEntry(ctx, "RPName", anyProcessorName, "RequestProcessor");
	OStringStream os1;
	httpProcessor->RenderProtocolStatus(os1, ctx);
	assertCharPtrEqual( "HTTP/1.1 200 OK" ENDL "Connection: close" ENDL, os1.str() );

	Anything tmpStore = ctx.GetTmpStore();
	tmpStore["HTTPStatus"]["ResponseCode"] = 599L;
	OStringStream os2;
	httpProcessor->RenderProtocolStatus(os2, ctx);
	assertCharPtrEqual( "HTTP/1.1 599 Unknown Error" ENDL "Connection: close" ENDL, os2.str() );

	tmpStore["HTTPStatus"]["ResponseCode"] = 413L;

	OStringStream os3;
	tmpStore["HTTPStatus"]["ResponseMsg"] = "BlaBla";

	httpProcessor->RenderProtocolStatus(os3, ctx);
	assertCharPtrEqual( "HTTP/1.1 413 BlaBla" ENDL "Connection: close" ENDL, os3.str() );
}

void HTTPProcessorTest::KeepConnection() {
	StartTrace(HTTPProcessorTest.KeepConnection);

	RequestProcessor *httpProcessor = HTTPProcessor::FindRequestProcessor("HTTPProcessor");
	Anything anyConfig;
	anyConfig["RequestProcessor"] = httpProcessor->GetName();
	anyConfig["PersistentConnections"] = 1L;
	{
		Context ctx;
		Context::PushPopEntry<Anything> aRPEntry(ctx, "RPName", anyConfig);
		Anything req;
		req["env"]["SERVER_PROTOCOL"] = "HTTP/1.0";
		req["env"]["header"]["CONNECTION"] = "Keep-Alive";
		ctx.PushRequest(req);
		t_assert(!httpProcessor->KeepConnectionAlive(ctx));
	}
	{
		Context ctx;
		Context::PushPopEntry<Anything> aRPEntry(ctx, "RPName", anyConfig);
		Anything req;
		req["env"]["SERVER_PROTOCOL"] = "HTTP/1.0";
		req["env"]["header"]["CONNECTION"] = "Close";
		ctx.PushRequest(req);
		t_assert(!httpProcessor->KeepConnectionAlive(ctx));
	}
	{
		Context ctx;
		Context::PushPopEntry<Anything> aRPEntry(ctx, "RPName", anyConfig);
		Anything req;
		req["env"]["SERVER_PROTOCOL"] = "HTTP/1.1";
		req["env"]["header"]["CONNECTION"] = "Keep-Alive";
		ctx.PushRequest(req);
		t_assert(httpProcessor->KeepConnectionAlive(ctx));
	}
	{
		Context ctx;
		Context::PushPopEntry<Anything> aRPEntry(ctx, "RPName", anyConfig);
		Anything req;
		req["env"]["SERVER_PROTOCOL"] = "HTTP/1.1";
		req["env"]["header"]["CONNECTION"] = "Close";
		ctx.PushRequest(req);
		t_assert(!httpProcessor->KeepConnectionAlive(ctx));
	}
}

void HTTPProcessorTest::IsZipEncodingAcceptedByClientTest() {
	ROAnything caseConfig;
	AnyExtensions::Iterator<ROAnything, ROAnything, TString> aEntryIterator(GetTestCaseConfig());
	while (aEntryIterator.Next(caseConfig)) {
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
Test *HTTPProcessorTest::suite() {
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
