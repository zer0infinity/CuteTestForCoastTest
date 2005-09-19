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
#include "PersistentLDAPConnection.h"

//--- c-modules used -----------------------------------------------------------

//---- ErrorHandlerTest ----------------------------------------------------------------
ErrorHandlerTest::ErrorHandlerTest(TString tstrName) : TestCase(tstrName)
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
	PersistentLDAPConnection plc(cp);

	// do test (connection params will provoke error which will be handled)
	LDAPErrorHandler eh(*fCtx, fGet, fPut, daName);
	eh.PutConnectionParams(cp);
	t_assertm( !plc.Connect(cp, eh), "Could connect, but shouldn't!");

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
	assertEqual(error.GetSize(), 3L);
	eh.CleanUp();
	t_assert(!(fCtx->GetTmpStore().IsDefined("LDAPError")));

	eh.HandleError("HandleError2", info);
	t_assert(eh.GetError(error));
	TraceAny(error, "Error2");
	assertEqual(error["Msg"].AsString(), "HandleError2");
	assertAnyEqual(error["AdditionalInfo"], info);
	eh.CleanUp();
	t_assert(!(fCtx->GetTmpStore().IsDefined("LDAPError")));

	eh.HandleError("HandleError3", info, "MyErrorArgs");
	t_assert(eh.GetError(error));
	TraceAny(error, "Error3");
	assertEqual(error["Msg"].AsString(), "HandleError3");
	assertAnyEqual(error["MyErrorArgs"], info);
}

void ErrorHandlerTest::testShouldRetry()
{
	StartTrace(ErrorHandlerTest.testHandleError);

	LDAPErrorHandler eh(*fCtx, fGet, fPut, "TestHandleError");

	eh.SetRetryState(LDAPErrorHandler::eRetry);
	assertEquals(LDAPErrorHandler::eRetry, eh.GetRetryState());
	eh.SetRetryState(LDAPErrorHandler::eNoRetry);
	assertEquals(LDAPErrorHandler::eNoRetry, eh.GetRetryState());
	eh.SetRetryState(LDAPErrorHandler::eIsInRetrySequence);
	assertEquals(LDAPErrorHandler::eIsInRetrySequence, eh.GetRetryState());
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
	ADD_CASE(testSuite, ErrorHandlerTest, testShouldRetry);

	return testSuite;
}
