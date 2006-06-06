/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

//--- test modules used --------------------------------------------------------
#include "TestSuite.h"

//--- module under test --------------------------------------------------------
#include "Session.h"

//--- interface include --------------------------------------------------------
#include "TransitionTests.h"

//--- standard modules used ----------------------------------------------------
#include "Socket.h"
#include "Role.h"
#include "Server.h"
#include "Renderer.h"
#include "SecurityModule.h"

//---- SessionInfoRenderer ----------------------------------------------------------------
class SessionInfoRenderer : public Renderer
{
public:
	SessionInfoRenderer(const char *name);

	void RenderAll(ostream &reply, Context &c, const ROAnything &data);
};

RegisterRenderer(SessionInfoRenderer);

SessionInfoRenderer::SessionInfoRenderer(const char *name) : Renderer(name)
{
}

void SessionInfoRenderer::RenderAll(ostream &reply, Context &c, const ROAnything &config)
{
	String roleName;
	c.GetRole()->GetName(roleName);
	reply << c.GetSession()->GetId() << endl << roleName;

	Anything query = c.GetQuery();
	if (query.IsDefined("delayedIndex")) {
		reply << endl << query["delayedIndex"].AsLong(-1);
	} else {
		reply << endl << "-1";
	}
}

//---- LoginAction ----------------------------------------------------------------------
//:simple login action
class LoginAction : public Action
{
public:
	LoginAction(const char *name);

	virtual bool DoAction(String &action, Context &);
	IFAObject *Clone() const {
		return new LoginAction(fName);
	}
};

RegisterAction(LoginAction);

LoginAction::LoginAction(const char *name) : Action(name)
{
}

bool LoginAction::DoAction(String &action, Context &c)
{
	Anything query = c.GetQuery();

	if (query["LoginName"] == "Coast") {
		action = "GoCustomerAction";
	} else {
		action = "Logout";
	}

	return true;
}

//---- TransitionTests ----------------------------------------------------------------
TransitionTests::TransitionTests(TString tname)
	: TestCaseType(tname)
{
	StartTrace(TransitionTests.TransitionTests);
}

TString TransitionTests::getConfigFileName()
{
	return "PBOWTypeConfig";
}

TransitionTests::~TransitionTests()
{
}

void TransitionTests::setUp ()
{
	t_assert(GetConfig().IsDefined("Roles"));
	t_assert(GetConfig().IsDefined("Modules"));
	t_assert(GetConfig().IsDefined("Servers"));

	WDModule *svcs = WDModule::FindWDModule("ServersModule");
	svcs->Init(GetConfig());

	// ensure installation of modules
	WDModule::Install(GetConfig());
}

void TransitionTests::tearDown ()
{
	WDModule::Terminate(GetConfig());
}

bool TransitionTests::EvalRequest(ROAnything request, Anything &returned)
{
	// connect to the server
	String reply;
	Connector connector("127.0.0.1", GetConfig()["testport"].AsLong());
	iostream *Ios = connector.Use()->GetStream();
	t_assert(Ios != 0);

	if (Ios) {
		String resultRole, resultPage, resultPage2, sessionId, delayed;

		// post request
		(*Ios) << request;

		// read reply
		char c;
		while ((c = Ios->get()) != EOF) {
			reply << c;
		}

		// extract infos about the received page
		StringTokenizer st(reply, '\n');
		if (st.NextToken(resultPage) && st.NextToken(resultPage2) && st.NextToken(sessionId)
			&& st.NextToken(resultRole) && st.NextToken(delayed)) {

			resultPage.Trim(resultPage.Length() - 1);	// since page is written as the response header
			// the is an additional newline that separates
			// it from the other data

			returned["page"] = resultPage;
			assertEqual(resultPage, resultPage2);
			returned["sessionId"] = sessionId;
			returned["role"] = resultRole;
			returned["delayed"] = delayed;

			return true;
		}
		return false;
	}
	return false;
}

Anything TransitionTests::AddSessionInfo(ROAnything request, Anything context)
{
	Anything result = request.DeepClone();

	Anything sessionInfo;
	sessionInfo["sessionId"] = context["sessionId"].AsCharPtr();
	sessionInfo["role"] = context["role"].AsCharPtr();
	sessionInfo["page"] = context["page"].AsCharPtr();

	String s;
	SecurityModule::ScrambleState(s, sessionInfo);

	if (GetConfig()["UseBaseURL"].AsBool(false)) {
		result["query"]["X1"] = s;
	} else {
		result["query"]["X"] = s;
	}

	long delayedIndex = context["delayed"].AsLong(-1);
	if (delayedIndex >= 0) {
		result["query"]["delayedIndex"] = delayedIndex;
	}

	return result;
}

void TransitionTests::PBOWLoginSequence1()
//: an unsuccessful login sequence:
//  default request (using default role) -> PBOWLoginPage ...
//  unsuccessful login attempt -> PBOWLoginPage (still in default role)
{
	//  (i.e. default request, then login request)
	Anything r1, r2;

	// -- get default page & session id
	t_assert(EvalRequest(GetConfig()["DefaultRequest"], r1));
	assertEqual("PBOWLoginPage", r1["page"].AsString());

	// -- perform unsuccessful login using existing session id
	Anything c1 = AddSessionInfo(GetConfig()["BadLoginRequest"], r1);
	t_assert(EvalRequest(c1, r2));
	assertEqual("PBOWLoginPage", r2["page"].AsString());

	// verify if we are still using the same session
	assertEqual(r1["sessionId"].AsString(), r2["sessionId"].AsString());
}

void TransitionTests::PBOWLoginSequence2()
//: a successful login sequence:
//  default request (using default role) -> PBOWLoginPage ...
//  successful login request -> PBOWCustomerOnlyPage (now in privileged role) ...
//  request for other privileged page -> PBOWCustomerOnlyPage2
//  logout request -> PBOWLoginPage (back to unprivileged role)
{
	// a successful login sequence (i.e. default request, then login request)
	Anything r1, r2, r3, r4;

	// -- get default page & session id
	t_assert(EvalRequest(GetConfig()["DefaultRequest"], r1));
	assertEqual("PBOWLoginPage", r1["page"].AsString());

	// -- perform successful login using existing session id
	Anything c1 = AddSessionInfo(GetConfig()["GoodLoginRequest"], r1);
	t_assert(EvalRequest(c1, r2));
	assertEqual("PBOWCustomerOnlyPage", r2["page"].AsString());

	// verify if we are still using the same session
	assertEqual(r1["sessionId"].AsString(), r2["sessionId"].AsString());

	// -- get other privileged page using existing session id
	Anything c2 = AddSessionInfo(GetConfig()["CustomerRequest2"], r2);
	t_assert(EvalRequest(c2, r3));
	assertEqual("PBOWCustomerOnlyPage2", r3["page"].AsString());

	// verify if we are still using the same session
	assertEqual(r2["sessionId"].AsString(), r3["sessionId"].AsString());

	// -- logout (overwrite r3)
	Anything c3 = AddSessionInfo(GetConfig()["LogoutRequest"], r3);
	t_assert(EvalRequest(c3, r3));
	assertEqual("PBOWLoginPage", r3["page"].AsString());

	// remember a privileged request for use in next test
	fBookmarkedRequest = c2;

	// -- perform successful login using existing session id
	Anything c4 = AddSessionInfo(GetConfig()["GoodLoginRequest"], r3);
	t_assert(EvalRequest(c4, r4));
	assertEqual("PBOWCustomerOnlyPage", r4["page"].AsString());

	// verify if we are still using the same session
	assertEqual(r4["sessionId"].AsString(), r3["sessionId"].AsString());

	// -- logout (overwrite r4)
	c3 = AddSessionInfo(GetConfig()["LogoutRequest"], r4);
	t_assert(EvalRequest(c3, r4));
	assertEqual("PBOWLoginPage", r4["page"].AsString());
}

void TransitionTests::PBOWFailedBookmarkSequence()
//: simulate a privileged bookmark with a subsequent unsuccessful login
//  privileged request -> PBOWLoginPage ...
//  failed login request -> PBOWLoginPage (still in unprivileged role) ...
{
	Anything r1, r2;
	// the session is back in unprivileged mode (or has even been removed)...
	// re-play a privileged request:

	t_assert(EvalRequest(fBookmarkedRequest, r1));
	assertEqual("PBOWLoginPage", r1["page"].AsString());	// login must be enforced

	// -- perform unsuccessful login using existing session id
	Anything c1 = AddSessionInfo(GetConfig()["BadLoginRequest"], r1);
	t_assert(EvalRequest(c1, r2));
	assertEqual("PBOWLoginPage", r2["page"].AsString());

	// verify if we are still using the same session
	assertEqual(r1["sessionId"].AsString(), r2["sessionId"].AsString());
}

void TransitionTests::PBOWBookmarkSequence()
//: simulate a privileged bookmark with a subsequent successful login
//  privileged request -> PBOWLoginPage ...
//  successful login request -> PBOWCustomerOnlyPage2 (now in privileged role) ...
//  logout request -> PBOWLoginPage (back to unprivileged role)
{
	Anything r1, r2, r3;
	// the session is back in unprivileged mode (or has even been removed)...
	// re-play a privileged request:

	t_assert(EvalRequest(fBookmarkedRequest, r1));
	assertEqual("PBOWLoginPage", r1["page"].AsString());	// login must be enforced

	// -- perform successful login using existing session id...
	//    original request should be satisfied...
	Anything c1 = AddSessionInfo(GetConfig()["GoodLoginRequest"], r1);
	t_assert(EvalRequest(c1, r2));
	assertEqual("PBOWCustomerOnlyPage2", r2["page"].AsString());

	// verify if we are still using the same session
	assertEqual(r1["sessionId"].AsString(), r2["sessionId"].AsString());

	// -- logout (overwrite r3)
	Anything c3 = AddSessionInfo(GetConfig()["LogoutRequest"], r3);
	t_assert(EvalRequest(c3, r3));
	assertEqual("PBOWLoginPage", r3["page"].AsString());
}

void TransitionTests::RunRequestSequence()
{
	// startup a simple server
	Server *server = Server::FindServer("PBOWTypeServer");
	if ( t_assert(server != 0) ) {
		if ( ! t_assertm(server->Init() == 0, "PBOWTypeServer initialization failed") ) {
			return;
		}

		ServerThread mt(server);
		mt.Start();
		mt.CheckState(Thread::eRunning);
		// --- run various request
		//     sequences
		PBOWLoginSequence1();
		PBOWLoginSequence2();
		PBOWFailedBookmarkSequence();
		PBOWBookmarkSequence();

		server->PrepareShutdown(0);
		mt.Terminate(0);
	}
}

Test *TransitionTests::suite ()
{
	TestSuite *testSuite = new TestSuite;
	ADD_CASE(testSuite, TransitionTests, RunRequestSequence);
	return testSuite;
}
