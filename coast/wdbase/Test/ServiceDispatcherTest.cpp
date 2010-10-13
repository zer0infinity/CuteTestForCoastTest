/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

//--- interface include --------------------------------------------------------
#include "ServiceDispatcherTest.h"

//--- module under test --------------------------------------------------------
#include "ServiceDispatcher.h"

//--- test modules used --------------------------------------------------------
#include "TestSuite.h"

//--- standard modules used ----------------------------------------------------
#include "ServiceHandler.h"

//--- c-library modules used ---------------------------------------------------
#if !defined(WIN32)
#include <stdlib.h>
#endif

//---- TestService ----------------------------------------------------------
//:simple stub class to test service dispatcher
class TestService : public ServiceHandler
{
public:
	//:standard named object constructor
	TestService(const char *serviceHandlerName);
	~TestService();

	/*! @copydoc IFAObject::Clone(Allocator *) */
	IFAObject *Clone(Allocator *a) const {
		return new (a) TestService(fName);
	}

protected:
	//:does nothing
	virtual void DoHandleService(ostream &os, Context &ctx);

};

//---- ServiceDispatcherTest ----------------------------------------------------------------
ServiceDispatcherTest::ServiceDispatcherTest(TString tname)
	: TestCaseType(tname)
{
	StartTrace(ServiceDispatcherTest.ServiceDispatcherTest);
}

ServiceDispatcherTest::~ServiceDispatcherTest()
{
	StartTrace(ServiceDispatcherTest.Dtor);
}

void ServiceDispatcherTest::setUp ()
{
	StartTrace(ServiceDispatcherTest.setUp);
	t_assert(GetConfig().IsDefined("Modules"));
	t_assert(GetConfig()["Modules"].Contains("ServiceDispatchersModule"));
}

void ServiceDispatcherTest::FindTests()
{
	StartTrace(ServiceDispatcherTest.FindTests);
	{
		// test with no configuration
		Context ctx;
		ServiceDispatcher sd("TestServiceDispatcher");
		t_assertm(sd.FindServiceHandler(ctx) != 0, "expected to find standard handler");
		assertEqualm("WebAppService", sd.FindServiceName(ctx), "expected to find standard service name");
	}
	{
		// test with invalid context entry
		Anything args;
		args["env"]["DefaultService"] = "NoService";
		Context ctx(args);
		ServiceDispatcher sd("TestServiceDispatcher");
		t_assertm(sd.FindServiceHandler(ctx) == 0, "expected to find no handler");
		assertEqualm("NoService", sd.FindServiceName(ctx), "expected to find invalid service name");
	}
	{
		// test with objects configuration
		Context ctx;
		ServiceDispatcher sd("TestServiceDispatcher");
		ctx.Push("TestServiceDispatcher", &sd);

		sd.Initialize("ServiceDispatcher");
		t_assertm(sd.FindServiceHandler(ctx) != 0, "expected to find TestService handler");
		assertEqualm("TestService", sd.FindServiceName(ctx), "expected to find test TestService name");
		String strKey;
		ctx.Pop(strKey);
	}
}

void ServiceDispatcherTest::ServiceDispatcherModuleTest()
{
	StartTrace(ServiceDispatcherTest.ServiceDispatcherModuleTest);
	WDModule *serviceDispatchersModule = SafeCast(WDModule::FindWDModule("ServiceDispatchersModule"), WDModule);
	ServiceDispatcher *sd;
	if (t_assert(serviceDispatchersModule != NULL)) {
		if (t_assert((sd = ServiceDispatcher::FindServiceDispatcher("TestDispatcher")) != NULL)) {
			assertEqual("I am the TestDispatcher", sd->Lookup("MyIdString", "none"));
		}
		t_assert(ServiceDispatcher::FindServiceDispatcher("SecondTestDipatcher") != NULL);

		Anything config;
		Anything config1;
		config1["ServiceDispatchers"]["ServiceDispatcher"].Append("NewTestDispatcher");
		config1["ServiceDispatchers"]["ServiceDispatcher"].Append("NewSecondTestDispatcher");

		if ( t_assert(serviceDispatchersModule->ResetFinis(config)) ) {
			if ( t_assert(serviceDispatchersModule->ResetInit(config1)) ) {
				if (t_assert((sd = ServiceDispatcher::FindServiceDispatcher("NewTestDispatcher")) != NULL)) {
					assertEqual("I am the NewTestDispatcher", sd->Lookup("MyIdString", "none"));
				}
				t_assert(ServiceDispatcher::FindServiceDispatcher("NewSecondTestDispatcher") != NULL);
				t_assert(ServiceDispatcher::FindServiceDispatcher("TestDispatcher") == 0);
				t_assert(ServiceDispatcher::FindServiceDispatcher("SecondTestDipatcher") == 0);
			}
		}
	}
}

// builds up a suite of testcases, add a line for each testmethod
Test *ServiceDispatcherTest::suite ()
{
	StartTrace(ServiceDispatcherTest.suite);
	TestSuite *testSuite = new TestSuite;
	ADD_CASE(testSuite, ServiceDispatcherTest, FindTests);
	ADD_CASE(testSuite, ServiceDispatcherTest, ServiceDispatcherModuleTest);
	return testSuite;
}

RegisterServiceHandler(TestService);
//---- TestService ----------------------------------------------------------------
TestService::TestService(const char *name) : ServiceHandler(name)
{
}

TestService::~TestService()
{
}

void TestService::DoHandleService(ostream &reply, Context &ctx)
{
	StartTrace(TestService.DoHandleService);
}
