/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

//--- interface include --------------------------------------------------------
#include "WebAppServiceTest.h"

//--- module under test --------------------------------------------------------
#include "WebAppService.h"

//--- test modules used --------------------------------------------------------
#include "TestSuite.h"

//--- standard modules used ----------------------------------------------------
#include "Dbg.h"

//--- c-modules used -----------------------------------------------------------

//---- WebAppServiceTest ----------------------------------------------------------------
WebAppServiceTest::WebAppServiceTest(TString name)
	: ConfiguredTestCase(name, "WebAppServiceTestConfig")
{
	StartTrace(WebAppServiceTest.Ctor);
}

WebAppServiceTest::~WebAppServiceTest()
{
	StartTrace(WebAppServiceTest.Dtor);
}

// setup for this ConfiguredTestCase
void WebAppServiceTest::setUp ()
{
	StartTrace(WebAppServiceTest.setUp);
	ConfiguredTestCase::setUp();
}

void WebAppServiceTest::tearDown ()
{
	StartTrace(WebAppServiceTest.tearDown);
	ConfiguredTestCase::tearDown();
}

void WebAppServiceTest::BuildQueryTest()
{
	StartTrace(WebAppServiceTest.BuildQueryTest);
	WebAppService *pService = SafeCast(ServiceHandler::FindServiceHandler("WebAppService"), WebAppService);
	if (t_assert(pService != NULL)) {
		RUN_ENTRY("BuildQueryTest", cConfig) {
			Anything query;
			ROAnything expected(cConfig["Expected"]);
			String pathString(cConfig["Path"].AsString()), queryString(cConfig["Query"].AsString());
			query = pService->BuildQuery(pathString, queryString);
			assertAnyEqual(expected, query);
		}
	}
}

void WebAppServiceTest::Add2QueryTest()
{
	StartTrace(WebAppServiceTest.Add2QueryTest);
	WebAppService *pService = SafeCast(ServiceHandler::FindServiceHandler("WebAppService"), WebAppService);
	if (t_assert(pService != NULL)) {
		RUN_ENTRY("Add2QueryTest", cConfig) {
			Anything query(cConfig["Query"].DeepClone()), anyItems(cConfig["Add2Query"].DeepClone());
			bool bOverWrite = cConfig["OverWrite"].AsBool(false);
			ROAnything expectedQuery(cConfig["Expected"]);
			pService->Add2Query(query, anyItems, bOverWrite);
			assertAnyEqual(expectedQuery, query);
		}
	}
}

void WebAppServiceTest::DecodeWDQueryTest()
{
	StartTrace(WebAppServiceTest.DecodeWDQueryTest);

	WebAppService *pService = SafeCast(ServiceHandler::FindServiceHandler("WebAppService"), WebAppService);
	if (t_assert(pService != NULL)) {
		RUN_ENTRY("DecodeWDQueryTest", cConfig) {
			Anything query(cConfig["Query"].DeepClone()), request(cConfig["Request"].DeepClone());
			ROAnything expectedQuery(cConfig["Expected"]);
			pService->SplitURI2PathAndQuery(request);
			pService->DecodeWDQuery(query, request);
			assertAnyEqual(expectedQuery, query);
		}
	}
}

void WebAppServiceTest::PrepareRequestTest()
{
	StartTrace(WebAppServiceTest.PrepareRequestTest);

	WebAppService *pService = SafeCast(ServiceHandler::FindServiceHandler("WebAppService"), WebAppService);
	if (t_assert(pService != NULL)) {
		RUN_ENTRY("PrepareRequestTest", cConfig) {
			Anything request(cConfig["Request"].DeepClone());
			Context ctx(request);
			ROAnything expected(cConfig["Expected"]);
			pService->PrepareRequest(ctx);
			assertAnyEqual(expected, ctx.GetRequest());
		}
	}
}

void WebAppServiceTest::SplitURI2PathAndQueryTest()
{
	StartTrace(WebAppServiceTest.SplitURI2PathAndQueryTest);

	WebAppService *pService = SafeCast(ServiceHandler::FindServiceHandler("WebAppService"), WebAppService);
	if (t_assert(pService != NULL)) {
		Anything request;
		request["REQUEST_URI"] = "/fda?X=ABC&foo=bah&zzz";
		pService->SplitURI2PathAndQuery(request);

		assertEqual("/fda", request["PATH_INFO"].AsCharPtr());
		assertEqual("X=ABC&foo=bah&zzz", request["QUERY_STRING"].AsCharPtr());

		request = Anything();
		request["REQUEST_URI"] = "/finVAL/XS.ISA/foo/bah?action=Go&role=Guest";
		pService->SplitURI2PathAndQuery(request);

		assertEqual("/finVAL/XS.ISA/foo/bah", request["PATH_INFO"].AsCharPtr(""));
		assertEqual("action=Go&role=Guest", request["QUERY_STRING"].AsCharPtr(""));

		request = Anything();
		request["REQUEST_URI"] = "/finVAL/XS.ISA/foo/bah/X1=action=Go&role=Guest/X2=blabla=gugu";
		pService->SplitURI2PathAndQuery(request);

		assertEqual("/finVAL/XS.ISA/foo/bah/X1=action=Go&role=Guest/X2=blabla=gugu", request["PATH_INFO"].AsCharPtr(""));
		t_assert(!request.IsDefined("QUERY_STRING"));

		request = Anything();
		request["REQUEST_URI"] = "/";
		pService->SplitURI2PathAndQuery(request);
		assertEqual("/", request["PATH_INFO"].AsCharPtr(""));
		t_assert(!request.IsDefined("QUERY_STRING"));
	}
}

// builds up a suite of ConfiguredTestCases, add a line for each testmethod
Test *WebAppServiceTest::suite ()
{
	StartTrace(WebAppServiceTest.suite);
	TestSuite *testSuite = new TestSuite;

	ADD_CASE(testSuite, WebAppServiceTest, PrepareRequestTest);
	ADD_CASE(testSuite, WebAppServiceTest, BuildQueryTest);
	ADD_CASE(testSuite, WebAppServiceTest, Add2QueryTest);
	ADD_CASE(testSuite, WebAppServiceTest, DecodeWDQueryTest);
	ADD_CASE(testSuite, WebAppServiceTest, SplitURI2PathAndQueryTest);

	return testSuite;
}
