/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#include "ErrorHandlerTest.h"
#include "TestSuite.h"
#include "FoundationTestTypes.h"
#include "PersistentLDAPConnection.h"
#include "Context.h"

//---- ErrorHandlerTest ----------------------------------------------------------------
ErrorHandlerTest::ErrorHandlerTest(TString tstrName)
	: TestCaseType(tstrName)
{
	StartTrace(ErrorHandlerTest.ErrorHandlerTest);
}

ErrorHandlerTest::~ErrorHandlerTest()
{
	StartTrace(ErrorHandlerTest.Dtor);
}

void ErrorHandlerTest::setUp ()
{
	StartTrace(ErrorHandlerTest.setUp);
	fCtx = new Context();
	fPut = new (Coast::Storage::Global()) RootMapper(name());
	t_assert( fPut && fPut->Initialize("ParameterMapper") );
	fGet = new (Coast::Storage::Global()) ParameterMapper(name());
	t_assert( fGet && fGet->Initialize("ResultMapper") );
	t_assert(GetConfig().IsDefined("Modules"));
}

void ErrorHandlerTest::tearDown ()
{
	StartTrace(ErrorHandlerTest.tearDown);
	delete fCtx;
	delete fPut;
	delete fGet;
}

void ErrorHandlerTest::HandleConnectionErrorTest()
{
	StartTrace(ErrorHandlerTest.HandleConnectionErrorTest);

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
#if defined(USE_OPENLDAP)
		assertEqual(error["LdapCode"].AsLong(), -1L);	//!@FIXME code on openldap
#else
		assertEqual(error["LdapCode"].AsLong(), 91L);	//!@FIXME code on iPlanet-LDAP
#endif
	}
}

void ErrorHandlerTest::HandleErrorTest()
{
	StartTrace(ErrorHandlerTest.HandleErrorTest);

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

void ErrorHandlerTest::ShouldRetryTest()
{
	StartTrace(ErrorHandlerTest.HandleErrorTest);

	LDAPErrorHandler eh(*fCtx, fGet, fPut, "TestHandleError");

	eh.SetRetryState(LDAPErrorHandler::eRetry);
	assertEqual(LDAPErrorHandler::eRetry, eh.GetRetryState());
	eh.SetRetryState(LDAPErrorHandler::eNoRetry);
	assertEqual(LDAPErrorHandler::eNoRetry, eh.GetRetryState());
	eh.SetRetryState(LDAPErrorHandler::eIsInRetrySequence);
	assertEqual(LDAPErrorHandler::eIsInRetrySequence, eh.GetRetryState());
}

void ErrorHandlerTest::ParamAccessTest()
{
	StartTrace(ErrorHandlerTest.ParamAccessTest);

	Anything qp, cp;
	ROAnything rqp, rcp;
	LDAPErrorHandler eh(*fCtx, fGet, fPut, "TestParamAccess");

	// just a dummy slot for each
	qp["Base"] = "cn=CH,dc=any.server.com";
	cp["Server"] = "any.server.com";

	rqp = eh.GetQueryParams();
	t_assertm( rqp.IsNull() , "Found query params, but shouldn't!");

	rcp = eh.GetConnectionParams();
	t_assertm( rcp.IsNull(), "Found connection params, but shouldn't!");

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
	ADD_CASE(testSuite, ErrorHandlerTest, ParamAccessTest);
	ADD_CASE(testSuite, ErrorHandlerTest, HandleErrorTest);
	ADD_CASE(testSuite, ErrorHandlerTest, HandleConnectionErrorTest);
	ADD_CASE(testSuite, ErrorHandlerTest, ShouldRetryTest);
	return testSuite;
}
