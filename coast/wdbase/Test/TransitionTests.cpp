/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#include "TestSuite.h"
#include "Session.h"
#include "TransitionTests.h"
#include "Socket.h"
#include "Server.h"
#include "Role.h"
#include "Renderer.h"
#include "SecurityModule.h"
#include "StringStream.h"

class SessionInfoRenderer : public Renderer
{
public:
	SessionInfoRenderer(const char *name);

	void RenderAll(std::ostream &reply, Context &c, const ROAnything &data);
};

RegisterRenderer(SessionInfoRenderer);

SessionInfoRenderer::SessionInfoRenderer(const char *name) : Renderer(name)
{
}

void SessionInfoRenderer::RenderAll(std::ostream &reply, Context &c, const ROAnything &config)
{
	String roleName;
	c.GetRole()->GetName(roleName);
	reply << "/sessionId \"" << c.GetSession()->GetId() << "\"" << std::endl;
	reply << "/role " << roleName << std::endl;
	reply << "/delayed " << ROAnything(c.GetQuery())["delayedIndex"].AsLong(-1) << std::endl;
}

//:simple login action
class LoginAction : public Action
{
public:
	LoginAction(const char *name);

	virtual bool DoAction(String &action, Context &);
	/*! @copydoc IFAObject::Clone(Allocator *) */
	IFAObject *Clone(Allocator *a) const {
		return new (a) LoginAction(fName);
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
}

bool TransitionTests::EvalRequest(ROAnything request, Anything &returned)
{
	StartTrace(TransitionTests.EvalRequest);
	// connect to the server
	StringStream reply;
	Connector connector("127.0.0.1", GetConfig()["testport"].AsLong());
	std::iostream *Ios = connector.Use()->GetStream();
	t_assert(Ios != 0);

	if (Ios) {
		String resultRole, resultPage, resultPage2, sessionId, delayed;
		TraceAny(request, "evalrequest");
		// post request
		(*Ios) << request << std::flush;

		// read reply
		char c;
		while ((c = Ios->get()) != EOF) {
			reply << c;
		}
		reply << std::flush;
		Trace("native reply [" << reply.str() << "]");
		// extract infos about the received page
		if ( getline(reply, resultPage) ) {
			// trim carriage return
			resultPage.Trim(resultPage.Length() - 1);	// since page is written as the response header
			Trace("resultpage [" << resultPage << "]");
			t_assertm(returned.Import(reply, "stream"),"failed to import anything from server stream");
			TraceAny(returned, "result");
			assertEqual(resultPage, returned["page"].AsString());
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
		ServerThread mt(server);
		if ( t_assert(mt.Start()) && t_assert(mt.CheckState(Thread::eRunning, 5)) ) {
			if ( t_assertm(mt.serverIsInitialized(), "expected initialization to succeed") ) {
				mt.SetWorking();
				if (t_assertm(mt.IsReady(true, 5), "expected server to become ready within 5 seconds")) {
					// --- run various request sequences
					PBOWLoginSequence1();
					PBOWLoginSequence2();
					PBOWFailedBookmarkSequence();
					PBOWBookmarkSequence();
					mt.PrepareShutdown(0);
				}
			}
			if (t_assertm(mt.IsReady(false, 5), "expected server to become terminated within 5 seconds")) {
				mt.Terminate(10);
			}
		}
		t_assertm(mt.CheckState(Thread::eTerminated, 10), "expected server thread to become terminated");
	}
}

Test *TransitionTests::suite ()
{
	TestSuite *testSuite = new TestSuite;
	ADD_CASE(testSuite, TransitionTests, RunRequestSequence);
	return testSuite;
}
