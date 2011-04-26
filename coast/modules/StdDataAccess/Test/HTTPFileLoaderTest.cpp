/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#include "HTTPFileLoaderTest.h"
#include "TestSuite.h"
#include "FoundationTestTypes.h"
#include "HTTPFileLoader.h"
#include "URI2FileNameMapper.h"
#include "Context.h"
#include "HTTPConstants.h"

void HTTPFileLoaderTest::ReplyHeaderTest() {
	StartTrace(HTTPFileLoaderTest.ReplyHeaderTest);
	HTTPFileLoader hfl("test");
	ParameterMapper mpi("test");
	ResultMapper mpo("test");
	t_assert(hfl.Initialize("DataAccessImpl"));
	t_assert(mpi.Initialize("ParameterMapper"));
	t_assert(mpo.Initialize("ResultMapper"));

	Context ctx;
	String filename("defaultFileName");
	Anything tmpStore(ctx.GetTmpStore());

	t_assertm(hfl.GenReplyHeader(ctx, &mpi, &mpo), "expected success of header generation");

	Anything statusSpec;

	Anything verSpec;
	verSpec[0L]["ContextLookupRenderer"] = String("Mapper.").Append(Coast::HTTP::_httpProtocolVersionSlotname);
	statusSpec[Coast::HTTP::_httpProtocolVersionSlotname] = verSpec;

	Anything resCodeSpec;
	resCodeSpec[0L]["ContextLookupRenderer"] = String("Mapper.").Append(Coast::HTTP::_httpProtocolCodeSlotname);
	statusSpec[Coast::HTTP::_httpProtocolCodeSlotname] = resCodeSpec;

	Anything resMsgSpec;
	resMsgSpec[0L]["ContextLookupRenderer"] = String("Mapper.").Append(Coast::HTTP::_httpProtocolMsgSlotname);
	statusSpec[Coast::HTTP::_httpProtocolMsgSlotname] = resMsgSpec;
	// PS: no longer render endl here: statusSpec[5L]= ENDL;
	SubTraceAny(HTTPStatus, statusSpec, "HTTPStatus:");
	Anything httpStatus;
	t_assertm(tmpStore.LookupPath(httpStatus, "Mapper.HTTPStatus"), "expected HTTPStatus field in tmpStore");
	assertAnyEqual(statusSpec, httpStatus);

	Anything headerSpec;
#if old /* PS: insert ENDL just at the beginning */
	headerSpec[0L] = "Content-Type: ";
	headerSpec[1L]["ContextLookupRenderer"] = "Mapper.content-type";
	headerSpec[2L] = ENDL;
	headerSpec[3L] = "Content-Length: ";
	headerSpec[4L]["ContextLookupRenderer"] = "Mapper.content-length";
	headerSpec[5L] = ENDL;
#else
	Anything contentLengthSpec;
	contentLengthSpec[0L] = "Content-Length: ";
	contentLengthSpec[1L]["ContextLookupRenderer"] = "Mapper.content-length";
	contentLengthSpec[2L] = ENDL;

	Anything condSpec;
	condSpec["ContextCondition"] = "Mapper.content-length";
	condSpec["Defined"] = contentLengthSpec;

	headerSpec[0L] = "Content-Type: ";
	headerSpec[1L]["ContextLookupRenderer"] = "Mapper.content-type";
	headerSpec[2L] = ENDL;
	headerSpec[3L]["ConditionalRenderer"] = condSpec;
#endif
	SubTraceAny(HTTPHeader, headerSpec, "HTTPHeader:");
	Anything httpHeader;
	t_assertm(tmpStore.LookupPath(httpHeader, "Mapper.HTTPHeader"), "expected HTTPHeader field in tmpStore");
	assertAnyEqual(headerSpec, httpHeader);
}

void HTTPFileLoaderTest::ExecTest() {
	HTTPFileLoader hfl("test");
	URI2FileNameMapper mapin("test");
	ResultMapper mout("ExecTestOut");
	t_assert(hfl.Initialize("DataAccessImpl"));
	t_assert(mapin.Initialize("ParameterMapper"));
	t_assert(mout.Initialize("ResultMapper"));

	Context ctx;
	Anything tmpStore(ctx.GetTmpStore());
	String filename("defaultFileName");
	String ext("defaultExt");
	Anything httpHeader;

	tmpStore["DocumentRoot"] = ""; // dummy root should use absolute path
	tmpStore["REQUEST_URI"] = "/config/TestFile.html";

	t_assertm(hfl.Exec(ctx, &mapin, &mout), "expected success of file loading");

	t_assertm(tmpStore.LookupPath(httpHeader, "Mapper.HTTPHeader"), "expected Mapper.HTTPHeader field in tmpStore");
	assertEqual(200L, ctx.Lookup(String("Mapper.").Append(Coast::HTTP::_httpProtocolCodeSlotname), 400L));
	assertEqual("Ok", ctx.Lookup(String("Mapper.").Append(Coast::HTTP::_httpProtocolMsgSlotname), "Not found"));
	assertEqual("<html>\n<h1>Test</h1>\nsome html test data\n</html>\n", ctx.Lookup("Mapper.HTTPBody", "Not found"));

	tmpStore.Remove("Mapper");
	tmpStore["REQUEST_URI"] = "/config/NotThere<script>alert(\"gugus\")</script>";
	t_assertm(!hfl.Exec(ctx, &mapin, &mout), "expected failure of file loading");
	String body(ctx.Lookup("Mapper.HTTPBody", "<"));
	t_assertm(body.Contains(_QUOTE_(<p>The requested URL <b>/config/NotThere<script>alert("gugus")</script></b> is invalid.</p>)) >= 0, "No tainted content expected.");

	assertEqual(404, ctx.Lookup(String("Mapper.").Append(Coast::HTTP::_httpProtocolCodeSlotname), 200L));
	assertEqual("Not Found", ctx.Lookup(String("Mapper.").Append(Coast::HTTP::_httpProtocolMsgSlotname), "Ok"));

#if !defined(WIN32)
	tmpStore.Remove("Mapper");
	tmpStore["REQUEST_URI"] = "/config/NotReadable";
	t_assertm(!hfl.Exec(ctx, &mapin, &mout), "expected failure of file loading");

	assertEqualm(403, ctx.Lookup(String("Mapper.").Append(Coast::HTTP::_httpProtocolCodeSlotname), 200L), "Make sure the file config/NotReadable is not readable");
	assertEqualm("Forbidden", ctx.Lookup(String("Mapper.").Append(Coast::HTTP::_httpProtocolMsgSlotname), "Ok"), "Make sure the file config/NotReadable is not readable");
	assertEqualm("<html><head>\n<title>403 Forbidden</title>\n</head><body>\n<h1>Forbidden</h1>\n<p>The requested URL <b>/config/NotReadable</b> is invalid.</p>\n<hr />\n<address>Coast 2.0 Server</address>\n</body></html>\n", ctx.Lookup("Mapper.HTTPBody", "Ok"), "Wrong error message supplied!");
#else
	cerr << "\nFIXME: file hiding on WIN32" << endl;
#endif

	tmpStore.Remove("Mapper");
	tmpStore["REQUEST_URI"] = "/config/TestGif.gif";
	tmpStore["Ext2MIMETypeMap"]["gif"] = "image/gif";

	t_assertm(hfl.Exec(ctx, &mapin, &mout), "expected success of file loading");

	t_assertm(tmpStore.LookupPath(httpHeader, "Mapper.HTTPBody"), "expected Mapper.HTTPHeader field in tmpStore");
	assertEqual(200L, ctx.Lookup(String("Mapper.").Append(Coast::HTTP::_httpProtocolCodeSlotname), 400L));
	assertEqual("Ok", ctx.Lookup(String("Mapper.").Append(Coast::HTTP::_httpProtocolMsgSlotname), "Not found"));
	assertEqual("image/gif", ctx.Lookup("Mapper.content-type", "Not found"));
}

// builds up a suite of testcases, add a line for each testmethod
Test *HTTPFileLoaderTest::suite() {
	StartTrace(HTTPFileLoaderTest.suite);
	TestSuite *testSuite = new TestSuite;

	ADD_CASE(testSuite, HTTPFileLoaderTest, ReplyHeaderTest);
	ADD_CASE(testSuite, HTTPFileLoaderTest, ExecTest);

	return testSuite;

}
