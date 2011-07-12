/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#include "HTTPProcessorTest.h"
#include "HTTPProcessor.h"
#include "TestSuite.h"
#include "FoundationTestTypes.h"
#include "StringStream.h"
#include "StringStreamSocket.h"
#include "Context.h"
#include "URLUtils.h"
#include "Tracer.h"
#include "Server.h"
#include "AnyLookupInterfaceAdapter.h"
#include "AnyUtils.h"
#include "Renderer.h"
#include "AnyIterators.h"
#include "HTTPConstants.h"

namespace {
	String GetFirstResponseLine(String const &result) {
		return result.SubString(0L, result.Contains(ENDL));
	}
}

void HTTPProcessorTest::DoReadInputWithErrorTest() {
	StartTrace(HTTPProcessorTest.DoReadInputWithErrorTest);

	RequestProcessor *httpProcessor = RequestProcessor::FindRequestProcessor("HTTPProcessor");
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
			assertCharPtrEqualm(caseConfig["Expected"]["Reply"].AsString(), GetFirstResponseLine(strInOut), caseName);
			assertAnyCompareEqual(caseConfig["Expected"]["Request"], ctx.GetRequest(), caseName, '.', ':');
			OStringStream oss;
			ctx.DebugStores("bla", oss, true);
			Trace(oss.str());
			if (caseConfig["Expected"].IsDefined("TmpStore") ) {
				assertAnyCompareEqual(caseConfig["Expected"]["TmpStore"], ctx.GetTmpStore(), caseName, '.', ':');
			}
		}
	}
}

void HTTPProcessorTest::FileUploadTest() {
	StartTrace(HTTPProcessorTest.FileUploadTest);
	RequestProcessor *httpProcessor = RequestProcessor::FindRequestProcessor("HTTPProcessor");
	String uri = GetTestCaseConfig()["Request"].AsString();
	StringStreamSocket ss(uri);
	Context ctx(&ss);
	httpProcessor->ProcessRequest(ctx);
	assertAnyEqualm(GetTestCaseConfig()["Results"]["REQUEST_BODY"], ctx.GetEnvStore()["REQUEST_BODY"], name());
}

void HTTPProcessorTest::RenderProtocolStatusWithoutHTTPStatus() {
	StartTrace(HTTPProcessorTest.RenderProtocolStatus);

	RequestProcessor *httpProcessor = RequestProcessor::FindRequestProcessor("HTTPProcessor");
	Context ctx;
	Anything anyProcessorName = httpProcessor->GetName();
	Context::PushPopEntry<Anything> aRPEntry(ctx, "RPName", anyProcessorName, "RequestProcessor");
	{
		OStringStream os;
		httpProcessor->RenderProtocolStatus(os, ctx);
		assertCharPtrEqual( "HTTP/1.1 200 OK" ENDL "Connection: close" ENDL, os.str() );
	}
	{
		Anything tmpStore;
		OStringStream os;
		tmpStore[Coast::HTTP::_httpProtocolCodeSlotname] = 599L;
		Context::PushPopEntry<Anything> aEntry(ctx, "blub", tmpStore, "HTTPStatus");
		httpProcessor->RenderProtocolStatus(os, ctx);
		assertCharPtrEqual( "HTTP/1.1 599 Unknown Error" ENDL "Connection: close" ENDL, os.str() );
	}
	{
		OStringStream os;
		Anything tmpStore;
		tmpStore[Coast::HTTP::_httpProtocolCodeSlotname] = 413L;
		tmpStore[Coast::HTTP::_httpProtocolMsgSlotname] = "BlaBla";
		Context::PushPopEntry<Anything> aEntry(ctx, "blub", tmpStore, "HTTPStatus");
		httpProcessor->RenderProtocolStatus(os, ctx);
		assertCharPtrEqual( "HTTP/1.1 413 BlaBla" ENDL "Connection: close" ENDL, os.str() );
	}
	{
		OStringStream os;
		Anything tmpStore;
		tmpStore["ProtocolReplyRenderer"] = "HTTP/8.7 768 BLUB" ENDL;
		Context::PushPopEntry<Anything> aEntry(ctx, "blub", tmpStore);
		httpProcessor->RenderProtocolStatus(os, ctx);
		assertCharPtrEqual( "HTTP/8.7 768 BLUB" ENDL, os.str() );
	}
}

void HTTPProcessorTest::KeepConnection() {
	StartTrace(HTTPProcessorTest.KeepConnection);

	RequestProcessor *httpProcessor = RequestProcessor::FindRequestProcessor("HTTPProcessor");
	Anything anyConfig;
	anyConfig["RequestProcessor"] = httpProcessor->GetName();
	anyConfig["PersistentConnections"] = 1L;
	{
		Context ctx;
		Context::PushPopEntry<Anything> aRPEntry(ctx, "RPName", anyConfig);
		Anything req;
		req["env"]["SERVER_PROTOCOL"] = "HTTP/1.0";
		req["env"]["header"]["CONNECTION"][0L] = "Keep-Alive";
		ctx.PushRequest(req);
		t_assert(!httpProcessor->KeepConnectionAlive(ctx));
	}
	{
		Context ctx;
		Context::PushPopEntry<Anything> aRPEntry(ctx, "RPName", anyConfig);
		Anything req;
		req["env"]["SERVER_PROTOCOL"] = "HTTP/1.0";
		req["env"]["header"]["CONNECTION"][0L] = "Close";
		ctx.PushRequest(req);
		t_assert(!httpProcessor->KeepConnectionAlive(ctx));
	}
	{
		Context ctx;
		Context::PushPopEntry<Anything> aRPEntry(ctx, "RPName", anyConfig);
		Anything req;
		req["env"]["SERVER_PROTOCOL"] = "HTTP/1.1";
		req["env"]["header"]["CONNECTION"][0L] = "Keep-Alive";
		ctx.PushRequest(req);
		t_assert(httpProcessor->KeepConnectionAlive(ctx));
	}
	{
		Context ctx;
		Context::PushPopEntry<Anything> aRPEntry(ctx, "RPName", anyConfig);
		Anything req;
		req["env"]["SERVER_PROTOCOL"] = "HTTP/1.1";
		req["env"]["header"]["CONNECTION"][0L] = "Close";
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
	ADD_CASE(testSuite, HTTPProcessorTest, DoReadInputWithErrorTest);
	ADD_CASE(testSuite, HTTPProcessorTest, FileUploadTest);
	ADD_CASE(testSuite, HTTPProcessorTest, RenderProtocolStatusWithoutHTTPStatus);
	ADD_CASE(testSuite, HTTPProcessorTest, KeepConnection);
	return testSuite;
}
