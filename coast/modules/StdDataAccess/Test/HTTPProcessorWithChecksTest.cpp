/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#include "HTTPProcessorWithChecksTest.h"
#include "HTTPProcessorWithChecks.h"
#include "TestSuite.h"
#include "Context.h"
#include "HTTPProcessorWithChecks.h"
#include "Dbg.h"
#include "StringStream.h"
#include "StringStreamSocket.h"
#include "AnyIterators.h"
#include "AnyUtils.h"
#include "HTTPRequestReader.h"
#include "Server.h"
#include "AnyLookupInterfaceAdapter.h"
#include "Renderer.h"

//---- HTTPProcessorWithChecksTest ----------------------------------------------------------------
HTTPProcessorWithChecksTest::HTTPProcessorWithChecksTest(TString tstrName) :
	TestCaseType(tstrName) {
	StartTrace(HTTPProcessorWithChecksTest.HTTPProcessorWithChecksTest);
}

TString HTTPProcessorWithChecksTest::getConfigFileName() {
	return "HTTPProcessorWithChecksTestConfig";
}

HTTPProcessorWithChecksTest::~HTTPProcessorWithChecksTest() {
	StartTrace(HTTPProcessorWithChecksTest.Dtor);
}

namespace {
	String GetFirstResponseLine(String const &result) {
		return result.SubString(0L, result.Contains(ENDL));
	}
}

void HTTPProcessorWithChecksTest::CheckFaultyHeadersTest() {
	StartTrace(HTTPProcessorWithChecksTest.CheckFaultyHeadersTest);
	RequestProcessor *httpProcessor = RequestProcessor::FindRequestProcessor("HTTPProcessorWithChecks");
	Server *pServer = Server::FindServer("Server");
	if ( t_assert(pServer) ) {
		pServer->GlobalInit(0, 0, ROAnything());
		Anything anyTmpStore, anyRequest, env;
		ROAnything caseConfig;
		AnyExtensions::Iterator<ROAnything, ROAnything, TString> aEntryIterator(GetTestCaseConfig());
		while (aEntryIterator.Next(caseConfig)) {
			TString caseName;
			if ( !aEntryIterator.SlotName(caseName) ) {
				caseName << "At index: " << aEntryIterator.Index();
			}
			AnyLookupInterfaceAdapter<ROAnything> lia(caseConfig);
			String strInOut;
			{
				Context c;
				Renderer::RenderOnString(strInOut, c, caseConfig["RequestLine"]);
			}
			StringStreamSocket ss(strInOut);
			Context ctx(&ss);
			ctx.Push("tempargs", &lia);
			Anything anyValue = httpProcessor->GetName();
			ctx.SetServer(pServer);
			Context::PushPopEntry<Anything> aRPEntry(ctx, "RPName", anyValue, "RequestProcessor");
			httpProcessor->ProcessRequest(ctx);
			OStringStream oss;
			ctx.DebugStores("bla", oss, true);
			Trace(oss.str());
			assertCharPtrEqualm(caseConfig["Expected"]["Reply"].AsString(), GetFirstResponseLine(strInOut), caseName);
			if (caseConfig["Expected"].IsDefined("Request") ) {
				assertAnyCompareEqual(caseConfig["Expected"]["Request"], ctx.GetRequest(), caseName, '.', ':');
			}
			if (caseConfig["Expected"].IsDefined("TmpStore") ) {
				assertAnyCompareEqual(caseConfig["Expected"]["TmpStore"], ctx.GetTmpStore(), caseName, '.', ':');
			}
		}
	}
}

Test *HTTPProcessorWithChecksTest::suite() {
	StartTrace(HTTPProcessorWithChecksTest.suite);
	TestSuite *testSuite = new TestSuite;
	ADD_CASE(testSuite, HTTPProcessorWithChecksTest, CheckFaultyHeadersTest);
	return testSuite;
}
