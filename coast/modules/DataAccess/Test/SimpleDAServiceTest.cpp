/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

//--- interface include --------------------------------------------------------
#include "SimpleDAServiceTest.h"

//--- test modules used --------------------------------------------------------
#include "TestSuite.h"

//--- module under test --------------------------------------------------------
#include "SimpleDataAccessService.h"

//--- standard modules used ----------------------------------------------------
#include "System.h"
#include "StringStream.h"
#include "ServiceDispatcher.h"
#include "RequestProcessor.h"
#include "Dbg.h"

//--- c-library modules used ---------------------------------------------------

//---- SimpleDAServiceTest ----------------------------------------------------------------
SimpleDAServiceTest::SimpleDAServiceTest(TString tname)
	: TestCaseType(tname)
{
	StartTrace(SimpleDAServiceTest.Ctor);
}

SimpleDAServiceTest::~SimpleDAServiceTest()
{
	StartTrace(SimpleDAServiceTest.Dtor);
}

//:use the ServiceDispatcher to obtain a simple service
void SimpleDAServiceTest::SimpleDispatch()
{
	StartTrace(SimpleDAServiceTest.SimpleDispatch);
	{
		// test with objects configuration
		Context ctx;
		ServiceDispatcher sd("TestSimpleDAService");
		sd.Initialize("ServiceDispatcher");
		ctx.Push("TestSimpleDAService", &sd);

		ServiceHandler *sh = sd.FindServiceHandler(ctx);
		t_assertm(sh != 0, "expected to find SimpleDataAccessService handler");
		assertEqualm("SimpleDataAccessService", sd.FindServiceName(ctx), "expected to find test TestSimpleDAService name");
		if (sh) {
			String servicename;
			sh->GetName(servicename);
			assertEqualm("SimpleDataAccessService", servicename, "expected to find test TestSimpleDAService name");
		}
		String strKey;
		ctx.Pop(strKey);
	}
}
//: try to use it with a very simple DataAccess backend
void SimpleDAServiceTest::SimpleServiceCall()
{
	StartTrace(SimpleDAServiceTest.SimpleServiceCall);
}
//: try to use it with a missing DataAccess backend
void SimpleDAServiceTest::FailedServiceCall()
{
	StartTrace(SimpleDAServiceTest.FailedServiceCall);
	Anything dummy;
	Anything loopBackDATestCtx;

	Anything config;

	config["RequestProcessor"] = "TestHTTPProcessor";

	Context ctx(config);

	ServiceDispatcher sd("TestSimpleDAService");
	// do not initialize this one, this is the tests intent!! // sd.Initialize("ServiceDispatcher");
	ctx.Push("TestSimpleDAService", &sd);

	OStringStream reply;
	sd.Dispatch2Service(reply, ctx);
	assertCharPtrEqual("HTTP/1.1 200 OK\r\nContent-type: text/html\r\n\r\n<html><head><title>Error</title></head><body><h1>Error</h1>Access denied. Lookuptoken: VFSF</body></html>", reply.str());
}

// builds up a suite of testcases, add a line for each testmethod
Test *SimpleDAServiceTest::suite ()
{
	StartTrace(SimpleDAServiceTest.suite);
	TestSuite *testSuite = new TestSuite;

	ADD_CASE(testSuite, SimpleDAServiceTest, SimpleDispatch);
	ADD_CASE(testSuite, SimpleDAServiceTest, SimpleServiceCall);
	ADD_CASE(testSuite, SimpleDAServiceTest, FailedServiceCall);

	return testSuite;
}

class TestHTTPProcessor : public RequestProcessor
{
public:
	//!named object shared by all requests
	TestHTTPProcessor(const char *processorName)
		: RequestProcessor(processorName)
	{ }

protected:

	//! render the protocol specific error msg
	virtual void DoError(ostream &reply, const String &msg, Context &ctx);
};

//--- TestHTTPProcessor ----------------------------------------------------------
RegisterRequestProcessor(TestHTTPProcessor);

void TestHTTPProcessor::DoError(ostream &reply, const String &msg, Context &ctx)
{
	reply << "HTTP/1.1 200 OK" << ENDL;
	reply << "Content-type: text/html" << ENDL << ENDL;
	reply << "<html>";
	reply << "<head><title>Error</title></head>";
	reply << "<body>" << "<h1>Error</h1>" << msg << "</body></html>";

	HTMLTraceStores(reply, ctx);
}
