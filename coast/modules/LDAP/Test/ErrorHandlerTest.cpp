/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

//--- interface include --------------------------------------------------------
#include "ErrorHandlerTest.h"

//--- test modules used --------------------------------------------------------
#include "TestSuite.h"

//--- module under test --------------------------------------------------------
#include "LDAPErrorHandler.h"

//--- standard modules used ----------------------------------------------------
#include "Dbg.h"
#include "LDAPConnection.h"

//--- c-modules used -----------------------------------------------------------

//---- ErrorHandlerTest ----------------------------------------------------------------
ErrorHandlerTest::ErrorHandlerTest(TString name) : TestCase(name)
{
	StartTrace(ErrorHandlerTest.Ctor);
}

ErrorHandlerTest::~ErrorHandlerTest()
{
	StartTrace(ErrorHandlerTest.Dtor);
}

// setup for this TestCase
void ErrorHandlerTest::setUp ()
{
	StartTrace(ErrorHandlerTest.setUp);
	fCtx = new Context();
	fPut = new RootMapper("");
	fGet = new ParameterMapper("");
}

void ErrorHandlerTest::tearDown ()
{
	StartTrace(ErrorHandlerTest.tearDown);
	delete fCtx;
	delete fPut;
	delete fGet;
}

void ErrorHandlerTest::testHandleConnectionError()
{
	StartTrace(ErrorHandlerTest.testHandleConnectionError);

	// set up LDAP connection
	String daName("Dummy");
	Anything cp;

	// define server only, for everything else take defaults
	cp["Server"] = "nonexisting.server.li";
	LDAPConnection lc(cp);

	// do test (connection params will provoke error which will be handled)
	LDAPErrorHandler eh(*fCtx, fGet, fPut, daName);
	eh.PutConnectionParams(cp);
	t_assertm( !lc.Connect(cp, eh), "Could connect, but shouldn't!");

	Anything error;
	t_assertm(eh.GetError(error), "Found no error, but should!");
	TraceAny(error, "connection error:");

	if ( !error.IsNull() ) {
		assertAnyEqual(error["LdapConnectionParams"], cp);
		assertEqual(error["LdapCode"].AsLong(), 91L);
	}
}

void ErrorHandlerTest::testHandleError()
{
	StartTrace(ErrorHandlerTest.testHandleError);

	LDAPErrorHandler eh(*fCtx, fGet, fPut, "TestHandleError");

	Anything info;
	info["msg"] = "a msg";
	info["num"] = 123;

	Anything error;
	t_assert(!eh.GetError(error));

	eh.HandleError("HandleError1");
	t_assert(eh.GetError(error));
	TraceAny(error, "Error1");
	assertEqual(error["Msg"].AsString(), "HandleError1");
	assertEqual(error.GetSize(), 2L);
	fCtx->GetTmpStore().Remove("LDAPError");

	eh.HandleError("HandleError2", info);
	t_assert(eh.GetError(error));
	TraceAny(error, "Error2");
	assertEqual(error["Msg"].AsString(), "HandleError2");
	assertAnyEqual(error["AdditionalInfo"], info);
	fCtx->GetTmpStore().Remove("LDAPError");

	eh.HandleError("HandleError3", info, "MyErrorArgs");
	t_assert(eh.GetError(error));
	TraceAny(error, "Error3");
	assertEqual(error["Msg"].AsString(), "HandleError3");
	assertAnyEqual(error["MyErrorArgs"], info);
}

void ErrorHandlerTest::testParamAccess()
{
	StartTrace(ErrorHandlerTest.testParamAccess);

	Anything qp, cp;
	Anything rqp, rcp;
	LDAPErrorHandler eh(*fCtx, fGet, fPut, "TestParamAccess");
	String notFound("None available, sorry.");

	// just a dummy slot for each
	qp["Base"] = "cn=CH,dc=any.server.com";
	cp["Server"] = "any.server.com";

	rqp = eh.GetQueryParams();
	assertEqualm(notFound, rqp.AsString(), "Found query params, but shouldn't!");

	rcp = eh.GetConnectionParams();
	assertEqualm(notFound, rcp.AsString(), "Found connection params, but shouldn't!");

	eh.PutConnectionParams(cp);
	rcp = eh.GetConnectionParams();
	assertAnyEqual(cp, rcp);

	eh.PutQueryParams(qp);
	rqp = eh.GetQueryParams();
	assertAnyEqual(qp, rqp);
}

// builds up a suite of testcases, add a line for each testmethod
Test *ErrorHandlerTest::suite ()
{
	StartTrace(ErrorHandlerTest.suite);
	TestSuite *testSuite = new TestSuite;

	ADD_CASE(testSuite, ErrorHandlerTest, testParamAccess);
	ADD_CASE(testSuite, ErrorHandlerTest, testHandleError);
	ADD_CASE(testSuite, ErrorHandlerTest, testHandleConnectionError);

	return testSuite;
}
