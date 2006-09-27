/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

//--- interface include --------------------------------------------------------
#include "AuthenticationServiceTest.h"

//--- test modules used --------------------------------------------------------
#include "TestSuite.h"

//--- module under test --------------------------------------------------------
#include "AuthenticationService.h"

//--- standard modules used ----------------------------------------------------
#include "SecurityModule.h"

//--- c-library modules used ---------------------------------------------------

//---- TestService ----------------------------------------------------------
//:simple stub class to test service dispatcher
class AuthTestService : public ServiceHandler
{
public:
	//:standard named object constructor
	AuthTestService(const char *serviceHandlerName): ServiceHandler("AuthTest") { }
	~AuthTestService() {}

	//:cloning interface
	IFAObject *Clone() const {
		return new AuthTestService(fName);
	}

protected:
	//:
	virtual void DoHandleService(ostream &os, Context &ctx) {
		os << "test ok";
	}
};
RegisterServiceHandler(AuthTestService);

//---- AuthenticationServiceTest ----------------------------------------------------------------
AuthenticationServiceTest::AuthenticationServiceTest(TString tstrName)
	: ConfiguredActionTest(tstrName)
{
	StartTrace(AuthenticationServiceTest.AuthenticationServiceTest);
}

TString AuthenticationServiceTest::getConfigFileName()
{
	return "AuthenticationServiceTestConfig";
}

AuthenticationServiceTest::~AuthenticationServiceTest()
{
	StartTrace(AuthenticationServiceTest.Dtor);
}

void AuthenticationServiceTest::ServiceNotFoundTest()
{
	StartTrace(AuthenticationServiceTest.ServiceNotFoundTest);

	AuthenticationService ash("AuthWrongServiceHandlerTest");
	ash.Initialize("ServiceHandler");

	Context ctx;
	MakeAuthenticationInfo(ctx, "KnownUser", "correct");
	String expectedMsg("");
	DoTest(ash, ctx, expectedMsg );
}

void AuthenticationServiceTest::OkTest()
{
	StartTrace(AuthenticationServiceTest.OkTest);

	AuthenticationService ash("AuthServiceHandlerTest");
	ash.Initialize("ServiceHandler");

	{
		Context ctx;
		String expectedMsg("HTTP/1.1 401" );
		MakeAuthenticationInfo(ctx, "KnownUser", "NOTcorrect");
		DoTest(ash, ctx, expectedMsg );
	}
	{
		Context ctx;
		String expectedMsg("HTTP/1.1 401");
		MakeAuthenticationInfo(ctx, "UnknownUser", "NOTcorrect");
		DoTest(ash, ctx, expectedMsg );
	}
	{
		Context ctx;
		String expectedMsg("test ok");
		MakeAuthenticationInfo(ctx, "KnownUser", "correct");
		DoTest(ash, ctx, expectedMsg );

	}
	{
		Context ctx;
		String expectedMsg("test ok");
		MakeAuthenticationInfo(ctx, "User Name", "Pass Word");
		DoTest(ash, ctx, expectedMsg );
	}
}

void AuthenticationServiceTest::ConnCloseMessage(String &strMsg, String strRealm, String strOutput)
{
	StartTrace(AuthenticationServiceTest.ConnCloseMessage);
	strMsg << "HTTP/1.1 401 Unauthorized" << ENDL;
	strMsg << "Connection: close" << ENDL;
	strMsg << "WWW-Authenticate: Basic realm=\"" << strRealm << "\"" << ENDL << ENDL;
	strMsg << "<html><h1>" << strOutput << "</h1></html>";
}

void AuthenticationServiceTest::NoConfigTest()
{
	StartTrace(AuthenticationServiceTest.NoConfigTest);

	AuthenticationService ash("NoConfigAuthServiceHandlerTest");
	ash.Initialize("ServiceHandler");

	Context ctx;
	MakeAuthenticationInfo(ctx, "", "");

	String expectedMsg;
	ConnCloseMessage(expectedMsg, "Coast", "Please stay away");
	DoTest( ash, ctx, expectedMsg );

	// but can also find config data elsewhere in context
	PutInStore(GetTestCaseConfig()["TmpStore"].DeepClone(), ctx.GetTmpStore());

	String errorMsg;
	ConnCloseMessage(errorMsg, "AuthTest", "AuthTest");
	DoTest( ash, ctx, errorMsg );

	MakeAuthenticationInfo(ctx, "KnownUser", "correct");
	DoTest( ash, ctx, errorMsg );
}

void AuthenticationServiceTest::AuthenticationFailedTest()
{
	StartTrace(AuthenticationServiceTest.AuthenticationFailedTest);

	AuthenticationService ash("AuthServiceHandlerTest");
	ash.Initialize("ServiceHandler");

	String expectedMsg;
	ConnCloseMessage(expectedMsg, "AuthTest", "AuthTest");
	Context ctx;
	DoTest( ash, ctx, expectedMsg );

	// Now add auth info, but unknown user
	MakeAuthenticationInfo(ctx, "Unknown", "xyz");
	DoTest( ash, ctx, expectedMsg );
	// Now add auth info, but user pw is wrong
	MakeAuthenticationInfo(ctx, "KnownUser", "wrong");
	DoTest( ash, ctx, expectedMsg );
	// Now try empty username and pw
	MakeAuthenticationInfo(ctx, "", "");
	DoTest( ash, ctx, expectedMsg );
	// user name is casesensitiv
	MakeAuthenticationInfo(ctx, "Knownuser", "correct");
	DoTest( ash, ctx, expectedMsg );
	// user password is casesensitiv
	MakeAuthenticationInfo(ctx, "KnownUser", "corRRect");
	DoTest( ash, ctx, expectedMsg );
	// strange user name and password
	MakeAuthenticationInfo(ctx, "UserWith:InItsName", "willNotWork");
	DoTest( ash, ctx, expectedMsg );
}

void AuthenticationServiceTest::DoTest(ServiceHandler &sh, Context &ctx, String expectedMsg )
{
	StartTrace(AuthenticationServiceTest.DoTest);

	String output;
	{
		OStringStream os(output);
		sh.HandleService(os, ctx);
	}
	// if output and expectedMsg are both "" (empty strings):
	// output.Contains( expectedMsg ) is -1
	// BUT: the 2 strings are equal and the test should be accepted
	// --> Therefore ( output.IsEqual(expectedMsg) ) must be done
	// long ret = ( (long)output.IsEqual(expectedMsg) || output.Contains( expectedMsg ) );
	long ret = 0L;
	ret = (long)output.IsEqual(expectedMsg);
	if ( ret <= 0 ) {
		ret = output.Contains( expectedMsg );
	}
	if ( ret < 0L ) {
		assertEqualm(expectedMsg, output, name());
	}
}

void AuthenticationServiceTest::DoTest(ServiceHandler &sh, Context &ctx, Anything expectedAny )
{
	StartTrace(AuthenticationServiceTest.DoTest);

	Anything receivedAny;
	{
		StringStream stream;
		sh.HandleService(stream, ctx);
		receivedAny.Import(stream);
	}
	assertAnyEqualm(expectedAny, receivedAny , name() );
}

void AuthenticationServiceTest::MakeAuthenticationInfo(Context &ctx, String user, String pw)
{
	StartTrace(AuthenticationServiceTest.MakeAuthenticationInfo);

	String clearText(user);
	clearText << ":" << pw;
	String namePW;
	Encoder::Encode("Base64", namePW, clearText);

	String authInfo("Basic ");
	authInfo << namePW.SubString(7); // Remove "Base64:" at the beginning
	Anything env = ctx.GetTmpStore();
	env["header"]["AUTHORIZATION"] = authInfo;
	TraceAny(env, "env with Authorization header");
}

// builds up a suite of tests, add a line for each testmethod
Test *AuthenticationServiceTest::suite ()
{
	StartTrace(AuthenticationServiceTest.suite);
	TestSuite *testSuite = new TestSuite;

	ADD_CASE(testSuite, AuthenticationServiceTest, OkTest);
	ADD_CASE(testSuite, AuthenticationServiceTest, AuthenticationFailedTest);
	ADD_CASE(testSuite, AuthenticationServiceTest, ServiceNotFoundTest);
	ADD_CASE(testSuite, AuthenticationServiceTest, NoConfigTest);

	return testSuite;
}
