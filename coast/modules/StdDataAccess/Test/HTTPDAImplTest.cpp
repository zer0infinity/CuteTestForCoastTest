/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

//--- standard modules used ----------------------------------------------------
#include "Anything.h"
#include "Threads.h"
#include "Context.h"
#include "Mapper.h"
#include "Server.h"
#include "Session.h"
#include "Role.h"
#include "SSLModule.h"
#include "Dbg.h"

//--- interface include --------------------------------------------------------
#include "HTTPDAImplTest.h"

//--- test modules used --------------------------------------------------------
#include "TestSuite.h"

//--- module under test --------------------------------------------------------
#include "HTTPDAImpl.h"

//---- HTTPDAImplTest ----------------------------------------------------------------
//---- RequestReaderTest ----------------------------------------------------------------
HTTPDAImplTest::HTTPDAImplTest(TString name)
	: ConfiguredTestCase(name, "HTTPDAImplTestConfig")
{
	StartTrace(HTTPDAImplTest.Ctor);
}

void HTTPDAImplTest::setUp ()
{
	Anything dummy;
	ConfiguredTestCase::setUp();
	WDModule *ssl = WDModule::FindWDModule("SSLModule");
	ssl->Init(dummy);
}

void HTTPDAImplTest::tearDown ()
{
	ConfiguredTestCase::tearDown();
}

void HTTPDAImplTest::useSSLTest()
{
	ParameterMapper in("useSSL");
	in.CheckConfig("ParameterMapper");
	ResultMapper out("useSSL");
	out.CheckConfig("ResultMapper");

	HTTPDAImpl httpDAImpl("useSSLTest");
	httpDAImpl.CheckConfig("DataAccessImpl");

	Anything dummy;
	Context ctx(dummy, dummy, 0, 0, 0, 0);

	t_assert(httpDAImpl.Exec(ctx, &in, &out));
}

void HTTPDAImplTest::SSLTests()
{
	StartTrace(HTTPDAImplTest.SSLTests);

	FOREACH_ENTRY("SSLTests", cConfig, cName) {
		Trace("SSLTests: At entry: " << i);
		TraceAny(cConfig, "SSLTests config");
		ParameterMapper in("SSLTests");
		in.CheckConfig("ParameterMapper");
		ResultMapper out("SSLTests");
		HTTPDAImpl httpDAImpl("SSLTests");
		Context ctx;
		ctx.GetTmpStore() = cConfig["Config"].DeepClone();
		// We must set our ssl context individually, otherwise SSLOBJMGR takes care of it
		SSL_CTX	 *sslctx = SSLModule::GetSSLClientCtx(cConfig["Config"]["SSLModuleCfg"].DeepClone());
		ctx.GetTmpStore()["SSLContext"] = (IFAObject *) sslctx;
		assertEquals(httpDAImpl.Exec(ctx, &in, &out), cConfig["Results"]["HTTPDAImplRet"].AsBool(0));
		Anything tmp = ctx.GetTmpStore();
		Anything clientInfo;
		clientInfo = tmp["ClientInfoBackends"];
		TraceAny(clientInfo, "ClientInfo used to verify test results.");
		assertEqual(clientInfo["SSL"]["Peer"]["SSLCertVerifyStatus"]["SSL"]["Ok"].AsBool(0), cConfig["Results"]["SSLCertVerifyStatus"].AsBool(1));
		assertEqual(clientInfo["SSL"]["Peer"]["AppLevelCertVerifyStatus"].AsBool(0), cConfig["Results"]["AppLevelCertVerifyStatus"].AsBool(1));
		TraceAny(ctx.GetTmpStore(), "Tempstore after");
	}
}

void HTTPDAImplTest::SSLNirvanaConnectTests()
{
	StartTrace(HTTPDAImplTest.SSLTests);

	FOREACH_ENTRY("SSLNirvanaConnectTests", cConfig, cName) {
		Trace("SSLTests: At entry: " << i);
		TraceAny(cConfig, "SSLTests config");
		ParameterMapper in("SSLTests");
		in.CheckConfig("ParameterMapper");
		ResultMapper out("SSLTests");
		HTTPDAImpl httpDAImpl("SSLTests");
		Context ctx;
		ctx.GetTmpStore() = cConfig["Config"].DeepClone();
		for (long l = 0; l < cConfig["NumberOfRuns"].AsLong(0L); l++) {
			SSL_CTX	 *sslctx = NULL;
			// We must set our ssl context individually, otherwise SSLOBJMGR takes care of it
			if (cConfig["UseSSLObjectManager"].AsBool(1)) {
				sslctx = SSLModule::GetSSLClientCtx(cConfig["Config"]["SSLModuleCfg"].DeepClone());
			}
			ctx.GetTmpStore()["SSLContext"] = (IFAObject *) sslctx;
			assertEquals(httpDAImpl.Exec(ctx, &in, &out), cConfig["Results"]["HTTPDAImplRet"].AsBool(0));
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

void HTTPDAImplTest::ErrorHandlingTest()
{
	StartTrace(HTTPDAImplTest.ErrorHandlingTest);
	ParameterMapper in("useSSL");
	in.CheckConfig("ParameterMapper");
	ResultMapper out("useSSL");
	out.CheckConfig("ResultMapper");

	HTTPDAImpl httpDAImpl("ErrorHandlingTest");
	httpDAImpl.CheckConfig("DataAccessImpl");

	Anything dummy;
	Context ctx(dummy, dummy, 0, 0, 0, 0);

	t_assert(!httpDAImpl.Exec(ctx, &in, &out));
	TString strMsg("Connection to  [Server Name:ErrorHandlingTest IP:");
	strMsg << ctx.Lookup("Backend.Server", "") << " Port:" << ctx.Lookup("Backend.Port", "") << "] failed";
	TraceAny(ctx.GetTmpStore()["Mapper"]["Error"], "Mapper.Error");
	assertEqual(strMsg, ctx.GetTmpStore()["Mapper"]["Error"].AsCharPtr("bad"));
}

Test *HTTPDAImplTest::suite ()
// collect all test cases for the SocketStream
{
	TestSuite *testSuite = new TestSuite;

	testSuite->addTest (NEW_CASE(HTTPDAImplTest, useSSLTest));
	testSuite->addTest (NEW_CASE(HTTPDAImplTest, ErrorHandlingTest));
	testSuite->addTest (NEW_CASE(HTTPDAImplTest, SSLTests));
	testSuite->addTest (NEW_CASE(HTTPDAImplTest, SSLNirvanaConnectTests));

	return testSuite;
} // suite
