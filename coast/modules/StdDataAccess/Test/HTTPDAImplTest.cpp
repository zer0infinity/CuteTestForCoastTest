/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#include "HTTPDAImpl.h"
#include "HTTPDAImplTest.h"
#include "TestSuite.h"
#include "SSLModule.h"
#include "AnyIterators.h"
#include "Context.h"

//---- HTTPDAImplTest ----------------------------------------------------------------
HTTPDAImplTest::HTTPDAImplTest(TString tstrName) :
	TestCaseType(tstrName) {
	StartTrace(HTTPDAImplTest.HTTPDAImplTest);
}

TString HTTPDAImplTest::getConfigFileName() {
	return "HTTPDAImplTestConfig";
}

void HTTPDAImplTest::useSSLTest() {
	ParameterMapper in("useSSL");
	ResultMapper out("useSSL");
	HTTPDAImpl httpDAImpl("useSSLTest");
	t_assert(in.Initialize("ParameterMapper"));
	t_assert(out.Initialize("ResultMapper"));
	t_assert(httpDAImpl.Initialize("DataAccessImpl"));

	Anything dummy;
	Context ctx(dummy, dummy, 0, 0, 0, 0);

	t_assert(httpDAImpl.Exec(ctx, &in, &out));
}

void HTTPDAImplTest::ErrorHandlingTest() {
	StartTrace(HTTPDAImplTest.ErrorHandlingTest);
	ParameterMapper in("ErrorHandling");
	ResultMapper out("useSSL");
	HTTPDAImpl httpDAImpl("ErrorHandlingTest");
	t_assert(in.Initialize("ParameterMapper"));
	t_assert(out.Initialize("ResultMapper"));
	t_assert(httpDAImpl.Initialize("DataAccessImpl"));

	Anything dummy;
	Context ctx(dummy, dummy, 0, 0, 0, 0);
	t_assert(!httpDAImpl.Exec(ctx, &in, &out));
	TraceAny(ctx.GetTmpStore()["Mapper"]["Error"], "Mapper.Error");
	assertComparem(0L, less, ctx.Lookup("Mapper.Error").AsString().Length(), "expected error message length");
}

void HTTPDAImplTest::SSLTests() {
	StartTrace(HTTPDAImplTest.SSLTests);

	ROAnything cConfig;
	AnyExtensions::Iterator<ROAnything> aEntryIterator(GetTestCaseConfig());
	while (aEntryIterator.Next(cConfig)) {
		TraceAny(cConfig, "SSLTests config");
		ParameterMapper in("SSLTests");
		ResultMapper out("SSLTests");
		HTTPDAImpl httpDAImpl("SSLTests");
		t_assert(in.Initialize("ParameterMapper"));
		t_assert(out.Initialize("ResultMapper"));
		t_assert(httpDAImpl.Initialize("DataAccessImpl"));
		Context ctx;
		ctx.GetTmpStore() = cConfig["Config"].DeepClone();
		// We must set our ssl context individually, otherwise SSLOBJMGR takes care of it
		SSL_CTX *sslctx = SSLModule::GetSSLClientCtx(cConfig["Config"]["SSLModuleCfg"].DeepClone());
		ctx.GetTmpStore()["SSLContext"] = (IFAObject *) sslctx;
		assertEqual(httpDAImpl.Exec(ctx, &in, &out), cConfig["Results"]["HTTPDAImplRet"].AsBool(0));
		Anything tmp = ctx.GetTmpStore();
		Anything clientInfo;
		clientInfo = tmp["ClientInfoBackends"];
		TraceAny(clientInfo, "ClientInfo used to verify test results.");
		assertEqual(clientInfo["SSL"]["Peer"]["SSLCertVerifyStatus"]["SSL"]["Ok"].AsBool(0), cConfig["Results"]["SSLCertVerifyStatus"].AsBool(1));
		assertEqual(clientInfo["SSL"]["Peer"]["AppLevelCertVerifyStatus"].AsBool(0), cConfig["Results"]["AppLevelCertVerifyStatus"].AsBool(1));
		TraceAny(ctx.GetTmpStore(), "Tempstore after");
	}
}

void HTTPDAImplTest::SSLNirvanaConnectTests() {
	StartTrace(HTTPDAImplTest.SSLNirvanaConnectTests);

	ROAnything cConfig;
	AnyExtensions::Iterator<ROAnything> aEntryIterator(GetTestCaseConfig());
	while (aEntryIterator.Next(cConfig)) {
		TraceAny(cConfig, "SSLNirvanaConnectTests config");
		ParameterMapper in("SSLNirvanaConnectTests");
		ResultMapper out("SSLNirvanaConnectTests");
		HTTPDAImpl httpDAImpl("SSLNirvanaConnectTests");
		t_assert(in.Initialize("ParameterMapper"));
		t_assert(out.Initialize("ResultMapper"));
		t_assert(httpDAImpl.Initialize("DataAccessImpl"));
		Context ctx;
		ctx.GetTmpStore() = cConfig["Config"].DeepClone();
		for (long l = 0; l < cConfig["NumberOfRuns"].AsLong(0L); l++) {
			SSL_CTX *sslctx = NULL;
			// We must set our ssl context individually, otherwise SSLOBJMGR takes care of it
			if (cConfig["UseSSLObjectManager"].AsBool(1)) {
				sslctx = SSLModule::GetSSLClientCtx(cConfig["Config"]["SSLModuleCfg"].DeepClone());
			}
			ctx.GetTmpStore()["SSLContext"] = (IFAObject *) sslctx;
			assertEqual(httpDAImpl.Exec(ctx, &in, &out), cConfig["Results"]["HTTPDAImplRet"].AsBool(0));
			Anything tmp = ctx.GetTmpStore();
			Anything clientInfo;
			clientInfo = tmp["ClientInfoBackends"];
			TraceAny(clientInfo, "ClientInfo used to verify test results.");
			assertEqual(clientInfo["SSL"]["Peer"]["SSLCertVerifyStatus"]["SSL"]["Ok"].AsBool(0), cConfig["Results"]["SSLCertVerifyStatus"].AsBool(1));
			assertEqual(clientInfo["SSL"]["Peer"]["AppLevelCertVerifyStatus"].AsBool(0), cConfig["Results"]["AppLevelCertVerifyStatus"].AsBool(1));
			TraceAny(ctx.GetTmpStore(), "Tempstore after");
		}
	}
}

Test *HTTPDAImplTest::suite() {
	TestSuite *testSuite = new TestSuite;
	ADD_CASE(testSuite, HTTPDAImplTest, useSSLTest);
	ADD_CASE(testSuite, HTTPDAImplTest, ErrorHandlingTest);
	ADD_CASE(testSuite, HTTPDAImplTest, SSLTests);
	ADD_CASE(testSuite, HTTPDAImplTest, SSLNirvanaConnectTests);
	return testSuite;
}
