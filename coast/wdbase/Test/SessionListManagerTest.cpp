/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#include "SessionListManagerTest.h"
#include "TestSuite.h"
#include "FoundationTestTypes.h"
#include "URLFilter.h"
#include "Context.h"
#include "SecurityModule.h"
#include "Server.h"
#include "TestSession.h"
#include "Tracer.h"

void SessionListManagerTest::tearDown() {
	StartTrace(SessionListManagerTest.tearDown);
	Context ctx;
	ctx.GetTmpStore()["Notify"]["expected"] = (long) Session::eRemoved;
	SessionListManager *sessionListManager = SafeCast(WDModule::FindWDModule("SessionListManager"), SessionListManager);
	sessionListManager->ForcedSessionCleanUp(ctx);
}

void SessionListManagerTest::InitFinisTest() {
	StartTrace(SessionListManagerTest.InitFinisTest);
	Anything config;
	Anything moduleConfig;
	moduleConfig["MaxSessionsAllowed"] = 1;
	moduleConfig["CleanerTimeout"] = 5;
	moduleConfig["Log2Cerr"] = 1;
	config["SessionListManager"] = moduleConfig;

	SessionListManager *sessionListManager = SafeCast(WDModule::FindWDModule("SessionListManager"), SessionListManager);
	t_assertm(sessionListManager != 0, "expected SessionListManager module to be there");
	if (sessionListManager) {
		t_assertm(sessionListManager->Init(config), "expected SessionListManager module to initialize correctly");
		assertEqualm(1L, sessionListManager->fMaxSessionsAllowed, "expected fMaxSessionsAllowed to be initialized by config");
		assertEqualm(0L, sessionListManager->GetNumberOfSessions(), "expected fSessionsCount to be initialized");
		assertEqualm(0L, sessionListManager->fNextId, "expected fNextId to be initialized");
		assertEqualm(1L, sessionListManager->fLogToCerr, "expected fLogToCerr to be initialized");
		t_assertm(sessionListManager->fSessionCleaner != 0, "expected fSessionCleaner to be initialized");
		t_assertm(sessionListManager->fSessionFactory != 0, "expected fSessionFactory to be initialized");

		t_assertm(sessionListManager->Finis(), "expected SessionListManager module to terminate successfully");
		assertEqualm(0L, sessionListManager->fMaxSessionsAllowed, "expected fMaxSessionsAllowed to be reset to zero");
		t_assertm(sessionListManager->fSessionCleaner == 0, "expected fSessionCleaner to be deleted");
		t_assertm(sessionListManager->fSessionFactory == 0, "expected fSessionFactory to be deleted");

	}

}

void SessionListManagerTest::ReInitTest() {
	StartTrace(SessionListManagerTest.ReInitTest);
	Context ctx;
	Session *s = 0, *s1 = 0;
	String sessionId;
	Anything config1;
	Anything moduleConfig1;
	moduleConfig1["MaxSessionsAllowed"] = 1;
	moduleConfig1["CleanerTimeout"] = 5;
	moduleConfig1["Log2Cerr"] = 1;
	config1["SessionListManager"] = moduleConfig1;

	Anything config2;
	Anything moduleConfig2;
	moduleConfig2["MaxSessionsAllowed"] = 10;
	moduleConfig2["CleanerTimeout"] = 50;
	moduleConfig2["Log2Cerr"] = 0;
	config2["SessionListManager"] = moduleConfig2;

	SessionListManager *sessionListManager = SafeCast(WDModule::FindWDModule("SessionListManager"), SessionListManager);
	t_assertm(sessionListManager != 0, "expected SessionListManager module to be there");
	if (sessionListManager) {
		t_assertm(sessionListManager->Init(config1), "expected SessionListManager module to initialize correctly");
		assertEqualm(1L, sessionListManager->fMaxSessionsAllowed, "expected fMaxSessionsAllowed to be initialized by config");
		assertEqualm(0L, sessionListManager->GetNumberOfSessions(), "expected fSessionsCount to be initialized");
		assertEqualm(0L, sessionListManager->fNextId, "expected fNextId to be initialized");
		assertEqualm(1L, sessionListManager->fLogToCerr, "expected fLogToCerr to be initialized");
		t_assertm(sessionListManager->fSessionCleaner != 0, "expected fSessionCleaner to be initialized");

		assertEqualm(0L, sessionListManager->GetNumberOfSessions(), "expected sessionListManager to have no sessions");
		s = sessionListManager->CreateSession(sessionId, ctx);
		if (t_assertm(s != 0, "expected sessionListManager to allocate a session")) {
			t_assertm(s->GetId() != 0, "expected session to have an id");
			assertEqual(sessionId, s->GetId());
		}
		t_assertm(sessionListManager->ResetFinis(config1), "expected SessionListManager module to initialize correctly");
		assertEqualm(0L, sessionListManager->fMaxSessionsAllowed, "expected fMaxSessionsAllowed to be reset to zero");
		t_assertm(sessionListManager->fSessionCleaner == 0, "expected fSessionCleaner to be deleted");
		t_assertm(sessionListManager->fSessionFactory == 0, "expected fSessionFactory to be deleted");

		t_assertm(sessionListManager->ResetInit(config2), "expected SessionListManager module to initialize correctly");
		assertEqualm(10L, sessionListManager->fMaxSessionsAllowed, "expected fMaxSessionsAllowed to be initialized by config");
		assertEqualm(1L, sessionListManager->GetNumberOfSessions(), "expected fSessionsCount to be initialized");
		assertEqualm(0L, sessionListManager->fLogToCerr, "expected fLogToCerr to be initialized");
		assertEqualm(1L, sessionListManager->fSessions.GetSize(), "expected sessions to survive reinit");
		t_assertm(sessionListManager->fSessionCleaner != 0, "expected fSessionCleaner to be initialized");
		t_assertm(sessionListManager->fSessionFactory != 0, "expected fSessionFactory to be initialized");
		s1 = sessionListManager->LookupSession(sessionId, ctx);
		if (t_assertm(s1 != 0, "expected sessionListManager to allocate a session")) {
			t_assertm(s == s1, "expected sessions to survive");
			t_assertm(s->GetId() == sessionId && s1->GetId() == sessionId, "expected session to have an id");
			assertEqual(s1->GetId(), s->GetId());
		}
		// reset session in ctx, since sessions
		// are deleted in Finis of SLM
		ctx.Push((Session *) 0);

		t_assertm(sessionListManager->Finis(), "expected SessionListManager module to terminate successfully");
		assertEqualm(0L, sessionListManager->fMaxSessionsAllowed, "expected fMaxSessionsAllowed to be reset to zero");
		t_assertm(sessionListManager->fSessionCleaner == 0, "expected fSessionCleaner to be deleted");
		t_assertm(sessionListManager->fSessionFactory == 0, "expected fSessionFactory to be deleted");
	}

}

void SessionListManagerTest::CreateSessionTest() {
	StartTrace(SessionListManagerTest.CreateSessionTest);

	Session *s;
	SessionListManager *sessionListManager = SafeCast(WDModule::FindWDModule("SessionListManager"), SessionListManager);
	t_assertm(sessionListManager != 0, "expected SessionListManager module to be there");

	if (sessionListManager) {
		Context ctx;
		String sessionId;
		sessionListManager->fMaxSessionsAllowed = 3;

		// standard case create a session and increment counter
		t_assertm(sessionListManager->fMaxSessionsAllowed >= 1, "fMaxSessionsAllowed to allow at least one session");
		t_assertm(sessionListManager->GetNumberOfSessions() == 0, "expected sessionListManager to have no sessions");
		s = sessionListManager->CreateSession(sessionId, ctx);
		t_assertm(s != 0, "expected sessionListManager to allocate a session");
		t_assertm(sessionListManager->GetNumberOfSessions() == 1, "expected sessionListManager to have one sessions");

		Session *s2 = sessionListManager->CreateSession(sessionId, ctx);
		t_assertm(s2 != 0, "expected sessionListManager to allocate a session");
		Session *s3 = sessionListManager->CreateSession(sessionId, ctx);
		t_assertm(s3 != 0, "expected sessionListManager to allocate a session");
		assertEqual(3L, sessionListManager->GetNumberOfSessions());

		// special case all slots filled; no session to delete -> session will not be allocated
		s = sessionListManager->CreateSession(sessionId, ctx);
		t_assertm(!s, "expected sessionListManager to fail allocating a session");
		assertEqual(3L, sessionListManager->GetNumberOfSessions());

		// special case all slots filled; sessions ready to be deleted -> session should be allocated
		s2->MakeInvalid(ctx);
		s3->MakeInvalid(ctx);
		s = sessionListManager->CreateSession(sessionId, ctx);
		t_assertm(s != 0, "expected sessionListManager to allocate a session");
		assertEqual(2L, sessionListManager->GetNumberOfSessions());
	}
}

void SessionListManagerTest::LookupSessionTest() {
	StartTrace(SessionListManagerTest.LookupSessionTest);

	Session *s;
	SessionListManager *sessionListManager = SafeCast(WDModule::FindWDModule("SessionListManager"), SessionListManager);
	t_assertm(sessionListManager != 0, "expected SessionListManager module to be there");

	if (sessionListManager) {
		sessionListManager->fMaxSessionsAllowed = 10;
		Context ctx1;
		String sessionId;

		// standard case create a session and increment counter
		t_assertm(sessionListManager->GetNumberOfSessions() == 0, "expected sessionListManager to have no sessions");
		s = sessionListManager->CreateSession(sessionId, ctx1);
		t_assertm(s != 0, "expected sessionListManager to allocate a session");
		if (s) {
			t_assertm(sessionListManager->GetNumberOfSessions() == 1, "expected sessionListManager to have one sessions");
			t_assertm(s->GetId() != 0, "expected session to have an id");
			t_assertm(s == sessionListManager->fSessions[s->GetId()].AsIFAObject(0), "expected session to be in sessionlist");
		}
	}
	// standard case create a session with a session id defined in query
	if (sessionListManager) {

		Anything query;
		String sessionId = "LookupTest";
		query["sessionId"] = sessionId;
		query["URLFilterSpec"] = Anything("None");
		Anything any;
		any["query"] = query;
		Context ctx(any);

		t_assertm(sessionListManager->GetNumberOfSessions() == 1, "expected sessionListManager to have no sessions");
		s = sessionListManager->CreateSession(sessionId, ctx);
		if (s) {
			t_assertm(s != 0, "expected sessionListManager to allocate a session");
			t_assertm(sessionListManager->GetNumberOfSessions() == 2, "expected sessionListManager to have one sessions");
			t_assertm(s->GetId() != 0, "expected session to have an id");
			t_assertm(s == sessionListManager->fSessions[s->GetId()].AsIFAObject(0), "expected session to be in sessionlist");
			assertEqualm(sessionId, s->GetId(), "expected sessionId to be 'LookupTest'");
			assertEqualm(any["query"]["SessionIsNew"].AsBool(0L), true, "expected flag to be there");

			s = sessionListManager->LookupSession(sessionId, ctx);
			t_assertm(s != 0, "expected sessionListManager to find allocated session");
			if (s) {
				assertEqualm("LookupTest", s->GetId(), "expected sessionId to be 'LookupTest'");

				// simulating sequence when request thread and cleaner thread have a bad overlapping
				s->MakeInvalid(ctx);
				sessionId = "";
				s = sessionListManager->LookupSession(sessionId, ctx);
				sessionListManager->CleanupSessions(ctx);
				assertEqual("", sessionId);
				t_assertm(!s, "expected sessionListManager to ignore terminated session");
			}

			// looking up sessions which are not there
			String notThere = "NotThere";
			query["sessionId"] = notThere;
			s = sessionListManager->LookupSession(sessionId, ctx);
			assertEqual("", sessionId);
			//	ctx.Push((Session*)0);
			t_assertm(s == NULL, "expected session to be NULL");
		}
	}
}

void SessionListManagerTest::DisableSessionTest() {
	StartTrace(SessionListManagerTest.DisableSessionTest);

	// assumes SessionListManager is initialized ok
	Session *s;
	SessionListManager *sessionListManager = SafeCast(WDModule::FindWDModule("SessionListManager"), SessionListManager);
	t_assertm(sessionListManager != 0, "expected SessionListManager module to be there");
	if (sessionListManager) {
		Anything query;
		String sessionId = "DisableTest";
		query["sessionId"] = sessionId;
		query["URLFilterSpec"] = Anything("None");
		Anything any;
		any["query"] = query;
		Context ctx(any);

		assertEqualm(0L, sessionListManager->GetNumberOfSessions(), "expected sessionListManager to have no sessions");
		s = sessionListManager->CreateSession(sessionId, ctx);
		if (t_assertm(s != 0, "expected sessionListManager to allocate a session")) {
			t_assertm(s->GetId() != 0, "expected session to have an id");
			assertEqual(sessionId, s->GetId());
		}

		s = sessionListManager->LookupSession(sessionId, ctx);
		if (t_assertm(s != 0, "expected sessionListManager to find test session")) {
			t_assertm(s != 0, "expected sessionListManager to find test session");
			t_assertm(s->GetId() != 0, "expected session to have an id");
			assertEqual(sessionId, s->GetId());
		}
		assertEqual(1, sessionListManager->fSessions.GetSize());
		sessionListManager->DisableSession(sessionId, ctx);
		assertEqual(0, sessionListManager->fSessions.GetSize());
		assertEqual(0, sessionListManager->GetNumberOfSessions());
		if (t_assertm(s != 0, "expected sessionListManager to find test session for disabling")) {
			t_assertm(s->GetId() != 0, "expected session to have an id");
			assertEqual(sessionId, s->GetId());
		}

		s = sessionListManager->LookupSession(sessionId, ctx);
		t_assertm(s == NULL, "expected sessionListManager to ignore disabled sessions");
		assertEqual(0, sessionListManager->fSessions.GetSize());
		Trace("remaining " << sessionListManager->fSessions.GetSize() << "  Sessions");
	}

}

void SessionListManagerTest::CleanupSessionTest() {
	StartTrace(SessionListManagerTest.DisableSessionTest);

	{
		Session *s;
		SessionListManager *sessionListManager = SafeCast(WDModule::FindWDModule("SessionListManager"), SessionListManager);
		t_assertm(sessionListManager != 0, "expected SessionListManager module to be there");

		if (sessionListManager) {
			Anything query;
			String sessionId = "DisableTest";
			query["sessionId"] = sessionId;
			query["URLFilterSpec"] = Anything("None");
			Anything any;
			any["query"] = query;
			Context ctx(any);

			assertEqualm(0L, sessionListManager->GetNumberOfSessions(), "expected sessionListManager to have no sessions");
			s = sessionListManager->CreateSession(sessionId, ctx);
			if (t_assertm(s != 0, "expected sessionListManager to allocate a session")) {
				t_assertm(s != 0, "expected sessionListManager to allocate a session");
				t_assertm(s->GetId() != 0, "expected session to have an id");
				assertEqual(sessionId, s->GetId());
			}

			s = sessionListManager->LookupSession(sessionId, ctx);
			if (t_assertm(s != 0, "expected sessionListManager to find test session")) {
				t_assertm(s->GetId() != 0, "expected session to have an id");
				assertEqual(sessionId, s->GetId());
			}

			sessionListManager->DisableSession(sessionId, ctx);

			s = sessionListManager->LookupSession(sessionId, ctx);
			t_assertm(s == NULL, "expected sessionListManager to ignore disabled sessions");

			s = sessionListManager->CreateSession(sessionId, ctx);
			if (t_assertm(s != 0, "expected sessionListManager to allocate a session")) {
				t_assertm(s->GetId() != 0, "expected session to have an id");
				assertEqual(sessionId, s->GetId());
				t_assertm(s->MakeInvalid(ctx), "expected session to terminate");
			}

			sessionListManager->CleanupSessions(ctx);
			assertEqualm(0L, sessionListManager->GetNumberOfSessions(), "expected sessionListManager to have no active sessions");
			assertEqualm(0L, sessionListManager->fSessions.GetSize(), "expected sessionListManager to have no active sessions in list");
			assertEqualm(0L, sessionListManager->fDisabledSessions.GetSize(), "expected sessionListManager to have no disabled sessions");

		}
	}
	{
		Session *s;
		SessionListManager *sessionListManager = SafeCast(WDModule::FindWDModule("SessionListManager"), SessionListManager);
		t_assertm(sessionListManager != 0, "expected SessionListManager module to be there");

		if (sessionListManager) {
			Anything query;
			String sessionId = "DisableTest";
			query["sessionId"] = sessionId;
			query["URLFilterSpec"] = Anything("None");
			Anything any;
			any["query"] = query;
			Context ctx(any);

			assertEqualm(0L, sessionListManager->GetNumberOfSessions(), "expected sessionListManager to have no sessions");
			s = sessionListManager->CreateSession(sessionId, ctx);
			if (t_assertm(s != 0, "expected sessionListManager to allocate a session")) {
				t_assertm(s != 0, "expected sessionListManager to allocate a session");
				t_assertm(s->GetId() != 0, "expected session to have an id");
				assertEqual(sessionId, s->GetId());
			}

			s = sessionListManager->LookupSession(sessionId, ctx);
			if (t_assertm(s != 0, "expected sessionListManager to find test session")) {
				t_assertm(s->GetId() != 0, "expected session to have an id");
				assertEqual(sessionId, s->GetId());
			}

			sessionListManager->DisableSession(sessionId, ctx);

			s = sessionListManager->LookupSession(sessionId, ctx);
			t_assertm(s == NULL, "expected sessionListManager to ignore disabled sessions");

			s = sessionListManager->CreateSession(sessionId, ctx);
			if (t_assertm(s != 0, "expected sessionListManager to allocate a session")) {
				t_assertm(s->GetId() != 0, "expected session to have an id");
				assertEqual(sessionId, s->GetId());
			}

			// the session in DisabledSessions will be removed. The recently created session
			// will be kept.
			sessionListManager->CleanupSessions(ctx);
			assertEqualm(1L, sessionListManager->GetNumberOfSessions(), "expected sessionListManager to have one active sessions");
			assertEqualm(1L, sessionListManager->fSessions.GetSize(), "expected sessionListManager to have one active sessions in list");
			assertEqualm(0L, sessionListManager->fDisabledSessions.GetSize(), "expected sessionListManager to have no disabled sessions");

		}
	}
}

void SessionListManagerTest::AddSameSessionNTimesTest() {
	StartTrace(SessionListManagerTest.AddSameSessionNTimesTest);

	Session *s;
	SessionListManager *sessionListManager = SafeCast(WDModule::FindWDModule("SessionListManager"), SessionListManager);
	t_assertm(sessionListManager != 0, "expected SessionListManager module to be there");

	if (sessionListManager) {
		Anything query;
		String sessionId = "UseUTwice";
		sessionListManager->fMaxSessionsAllowed = 10;
		query["sessionId"] = sessionId;
		query["URLFilterSpec"] = Anything("None");
		Anything any;
		any["query"] = query;
		Context ctx(any);

		assertEqualm(0L, sessionListManager->GetNumberOfSessions(), "expected sessionListManager to have no sessions");

		for (int i = 1; i < 11; i++) {
			s = sessionListManager->CreateSession(sessionId, ctx);
			if (t_assertm(s != 0, "expected sessionListManager to allocate a new session")) {
				t_assertm(s != 0, "expected sessionListManager to allocate a new session");
				t_assertm(s->GetId() != 0, "expected session to have an id");
				assertEqual(sessionId, s->GetId());
				assertEqualm(1L, sessionListManager->GetNumberOfSessions(), "expected sessionListManager to have ONE active sessions");
				assertEqualm(1L, sessionListManager->fSessions.GetSize(), "expected sessionListManager to have ONE active session in list");
				assertEqualm(i - 1, sessionListManager->fDisabledSessions.GetSize(), "expected sessionListManager to have  disabled sessions");
				s = sessionListManager->LookupSession(sessionId, ctx);
				if (t_assertm(s != 0, "expected sessionListManager to find test session")) {
					t_assertm(s->GetId() != 0, "expected session to have an id");
					assertEqual(sessionId, s->GetId());
				}
				// now mark session ready for removing if not modulo 10
				if ((i % 10) != 0) {
					t_assertm(s->MakeInvalid(ctx), "expected session be able to mark invalid");
				}
			}
		}

		sessionListManager->CleanupSessions(ctx);
		// Because the last session ((i %10) != 0) was not marked invalid, we have ONE active session and zero inactive ones
		// after CleanupSessions() is done
		assertEqualm(1L, sessionListManager->GetNumberOfSessions(), "expected sessionListManager to have NO active sessions");
		assertEqualm(1L, sessionListManager->fSessions.GetSize(), "expected sessionListManager to have no active sessions in list");
		assertEqualm(0L, sessionListManager->fDisabledSessions.GetSize(), "expected sessionListManager to have no disabled sessions");
	}
}

void SessionListManagerTest::StdFilterQueryTest() {
	StartTrace(SessionListManagerTest.StdFilterQueryTest);

	SessionListManager *sessionListManager = SafeCast(WDModule::FindWDModule("SessionListManager"), SessionListManager);
	t_assertm(sessionListManager != 0, "expected SessionListManager module to be there");

	Anything query;
	Anything env;
	Anything filterTags;
	URLFilter urlFilter("Test");
	long querySz;

	// first setup a state and scramble it
	Anything state;
	state["first"] = "test";
	state["second"] = "another test";
	String scrambledState;

	// scramble the state
	SecurityModule::ScrambleState(scrambledState, state);

	query["X"] = scrambledState;

	query["first"] = "base";
	query["third"] = "derived";
	query["sessionId"] = "s4";
	query["page"] = "s5";
	;
	query["action"] = "rc1";
	query["role"] = "s4";

	querySz = query.GetSize();
	Anything args;
	args["query"] = query;
	args["env"] = env;
	Context ctx(args);
	ctx.SetServer(Server::FindServer("Server"));
	t_assert(sessionListManager->FilterQuery(ctx));
	t_assertm(query.IsDefined("first"), "expected first to be there");
	t_assertm(query.IsDefined("second"), "expected second to be there");
	t_assertm(query.IsDefined("third"), "expected third to be there");
	t_assertm(!query.IsDefined("sessionId"), "expected sessionId not to be there");
	t_assertm(!query.IsDefined("page"), "expected page not to be there");
	t_assertm(!query.IsDefined("role"), "expected role not to be there");
	t_assertm(!query.IsDefined("action"), "expected action not to be there");
	assertEqual("base", query["first"].AsCharPtr(""));
	assertEqual("another test", query["second"].AsCharPtr(""));
	assertEqual("derived", query["third"].AsCharPtr(""));
	t_assert(query.GetSize() == querySz - 4);
}

void SessionListManagerTest::SessionListInfoTest() {
	StartTrace(SessionListManagerTest.SessionListInfoTest);

	Context ctx;
	String sessionId;
	Session *s;
	SessionListManager *sessionListManager = SafeCast(WDModule::FindWDModule("SessionListManager"), SessionListManager);
	Anything expectedSessionListInfo;
	Anything resultedSessionListInfo = Anything(Anything::ArrayMarker());
	if (t_assertm(sessionListManager != 0, "expected SessionListManager module to be there")) {
		s = sessionListManager->CreateSession(sessionId, ctx);
		expectedSessionListInfo["List"][sessionId]["id"] = sessionId;
		s->Info(expectedSessionListInfo["List"][sessionId], ctx);
		s = sessionListManager->CreateSession(sessionId, ctx);
		expectedSessionListInfo["List"][sessionId]["id"] = sessionId;
		s->Info(expectedSessionListInfo["List"][sessionId], ctx);
		s = sessionListManager->CreateSession(sessionId, ctx);
		expectedSessionListInfo["List"][sessionId]["id"] = sessionId;
		s->Info(expectedSessionListInfo["List"][sessionId], ctx);
		expectedSessionListInfo["Size"] = 3;
		expectedSessionListInfo["Pages"] = 1;
		expectedSessionListInfo["PageSize"] = 3;
		Anything config;
		t_assertm(sessionListManager->SessionListInfo(resultedSessionListInfo, ctx, config), "expected SessionListInfo to work");

		TraceAny(expectedSessionListInfo, "Expected:");
		TraceAny(resultedSessionListInfo, "Result:");

		assertAnyEqualm(expectedSessionListInfo, resultedSessionListInfo, "expected session list information to match");
	}
}

void SessionListManagerTest::GetASessionsInfoTest() {
	StartTrace(SessionListManagerTest.GetASessionsInfoTest);

	Context ctx;
	String sessionId1;
	String sessionId2;
	Session *s1;
	SessionListManager *sessionListManager = SafeCast(WDModule::FindWDModule("SessionListManager"), SessionListManager);
	sessionListManager->fMaxSessionsAllowed = 3;
	Anything expected;
	Anything resultedSessionListInfo = Anything(Anything::ArrayMarker());
	if (t_assertm(sessionListManager != 0, "expected SessionListManager module to be there")) {
		s1 = sessionListManager->CreateSession(sessionId1, ctx);
		t_assertm(s1->Info(expected[sessionId1], ctx), "Expected session info to be available");
		Anything config;
		t_assertm(sessionListManager->GetASessionsInfo(resultedSessionListInfo, sessionId1, ctx, config), "Expected GetASessionsInfo to succeed");
		TraceAny(resultedSessionListInfo, "Session Info for: " << sessionId1);
		TraceAny(expected, "Expected:");
		assertAnyEqualm(expected, resultedSessionListInfo, "expected session list information to match");
		t_assertm(!sessionListManager->GetASessionsInfo(resultedSessionListInfo, "dosnotexist", ctx, config), "Expected GetASessionsInfo to fail");

		sessionListManager->CreateSession(sessionId2, ctx);
		t_assertm(s1->Info(expected[sessionId1], ctx), "Expected session info to be available");
		t_assertm(sessionListManager->GetASessionsInfo(resultedSessionListInfo, sessionId1, ctx, config), "Expected GetASessionsInfo to succeed");
		TraceAny(resultedSessionListInfo, "Session Info for: " << sessionId1);
		TraceAny(expected, "Expected:");
		// Original session's refcount is incremented in GetASessionsInfo (session in ctx is not equal session for which
		// GetASessinInfo looks up the session info.
		expected[sessionId1]["Referenced"] = expected[sessionId1]["Referenced"].AsLong(0L) + 1L;
		assertAnyEqualm(expected, resultedSessionListInfo, "expected session list information to match");
		t_assertm(!sessionListManager->GetASessionsInfo(resultedSessionListInfo, "dosnotexist", ctx, config), "Expected GetASessionsInfo to fail");
	}
}

void SessionListManagerTest::SessionFactoryTest() {
	StartTrace(SessionListManagerTest.SessionFactoryTest);
	Anything args;
	args["env"]["header"]["USER-AGENT"] = "test agent";
	args["env"]["header"]["REMOTE_ADDR"] = "127.0.0.1";
	Context ctx(args);
	Session *s, *s1;
	String sessionId("SessionFactoryTest");
	Anything config1;
	Anything moduleConfig1;
	moduleConfig1["MaxSessionsAllowed"] = 1;
	moduleConfig1["CleanerTimeout"] = 5;
	config1["SessionListManager"] = moduleConfig1;

	Anything config2;
	Anything moduleConfig2;
	moduleConfig2["MaxSessionsAllowed"] = 10;
	moduleConfig2["CleanerTimeout"] = 50;
	moduleConfig2["SessionFactory"] = "TestSessionFactory";
	config2["SessionListManager"] = moduleConfig2;

	TestSessionFactory *tsm = SafeCast(SessionFactory::FindSessionFactory("TestSessionFactory"), TestSessionFactory);
	t_assertm(tsm != 0, "expected TestSessionFactory module to be there");

	SessionListManager *sessionListManager = SafeCast(WDModule::FindWDModule("SessionListManager"), SessionListManager);
	t_assertm(sessionListManager != 0, "expected SessionListManager module to be there");
	if (tsm && sessionListManager) {
		tsm->SetTest(this);
		t_assertm(sessionListManager->Init(config1), "expected SessionListManager module to initialize correctly");
		assertEqualm(1L, sessionListManager->fMaxSessionsAllowed, "expected fMaxSessionsAllowed to be initialized by config");
		assertEqualm(0L, sessionListManager->GetNumberOfSessions(), "expected fSessionsCount to be initialized");
		assertEqualm(0L, sessionListManager->fNextId, "expected fNextId to be initialized");
		t_assertm(sessionListManager->fSessionCleaner != 0, "expected fSessionCleaner to be initialized");

		t_assertm(sessionListManager->ResetFinis(config1), "expected SessionListManager module to initialize correctly");
		assertEqualm(0L, sessionListManager->fMaxSessionsAllowed, "expected fMaxSessionsAllowed to be reset to zero");
		t_assertm(sessionListManager->fSessionCleaner == 0, "expected fSessionCleaner to be deleted");
		t_assertm(sessionListManager->fSessionFactory == 0, "expected fSessionFactory to be deleted");

		t_assertm(sessionListManager->ResetInit(config2), "expected SessionListManager module to initialize correctly");
		ctx.GetTmpStore()["Notify"]["expected"] = (long) Session::eAdded;
		s = sessionListManager->CreateSession(sessionId, ctx);
		t_assertm(s != NULL, "expected SessionListManager to create a session");
		bool isBusy = false;
		ctx.GetTmpStore()["Session"]["expected"] = s;

		s1 = sessionListManager->PrepareSession(s, isBusy, ctx);
		t_assert(!isBusy);
		t_assertm(s == s1, "expected PrepareSession to return the same session");
		t_assertm(sessionListManager->ResetFinis(config2), "expected SessionListManager module to initialize correctly");
		assertEqualm(0L, sessionListManager->fMaxSessionsAllowed, "expected fMaxSessionsAllowed to be reset to zero");
		t_assertm(sessionListManager->fSessionCleaner == 0, "expected fSessionCleaner to be deleted");
		t_assertm(sessionListManager->fSessionFactory == 0, "expected fSessionFactory to be deleted");

		t_assertm(sessionListManager->ResetInit(config1), "expected SessionListManager module to initialize correctly");
	}
}

void SessionListManagerTest::UniqueInstanceIdTest() {
	StartTrace(SessionListManagerTest.CheckUniqueIdIsSet);

	SessionListManager *sessionListManager = SafeCast(WDModule::FindWDModule("SessionListManager"), SessionListManager);
	t_assertm(sessionListManager != 0, "expected SessionListManager module to be there");
	if (sessionListManager) {
		Anything config;
		sessionListManager->Init(config);

		String uniqueId;
		uniqueId = sessionListManager->GetUniqueInstanceId();
		// expect a "!" in the middle of the result string, not at beginning
		t_assert(uniqueId.StrChr('!', 0) > 0);
		t_assert(uniqueId.StrChr('!', 0) == uniqueId.StrRChr('!', -1));
		t_assert(uniqueId.StrChr('-', 0) == -1);
		t_assert(uniqueId.Length() > 2);

		String uniqueId2;
		uniqueId2 = SessionListManager::SLM()->GetUniqueInstanceId();
		t_assert(uniqueId == uniqueId2);
	}
}

void SessionListManagerTest::NotifyCalled(Session::ESessionEvt evt, Context &ctx) {
	StartTrace(SessionListManagerTest.NotifyCalled);
	assertEqual(ctx.Lookup("Notify.expected").AsLong(-1), evt);
}

void SessionListManagerTest::DoPrepareSessionCalled(Context &ctx, Session *session) {
	StartTrace(SessionListManagerTest.DoPrepareSessionCalled);
	assertEqual(ctx.Lookup("Session.expected").AsLong(-1), (long)session);

}

void SessionListManagerTest::SessionAccountingTest() {
	StartTrace(SessionListManagerTest.SessionAccountingTest);

	Session *s;
	SessionListManager *sessionListManager = SafeCast(WDModule::FindWDModule("SessionListManager"), SessionListManager);
	t_assertm(sessionListManager != 0, "expected SessionListManager module to be there");

	if (sessionListManager) {

		Anything config2;
		Anything moduleConfig2;
		moduleConfig2["MaxSessionsAllowed"] = 3;
		moduleConfig2["CleanerTimeout"] = 50;
		config2["SessionListManager"] = moduleConfig2;
		t_assertm(sessionListManager->Init(config2), "expected SessionListManager module to initialize correctly");

		Anything query;
		String sessionId = "SessionAccountingTest";
		query["sessionId"] = sessionId;
		query["URLFilterSpec"] = Anything("None");
		Anything any;
		any["query"] = query;
		Context ctx(any);

		assertEqualm(0L, sessionListManager->GetNumberOfSessions(), "expected sessionListManager to have no sessions");
		s = sessionListManager->CreateSession(sessionId, ctx);
		s->MakeInvalid(ctx); // force timeout of session
		assertEqualm(1L, sessionListManager->GetNumberOfSessions(), "expected sessionListManager to have one sessions");
		s = sessionListManager->CreateSession(sessionId, ctx);
		s->MakeInvalid(ctx); // force timeout of session
		assertEqualm(1L, sessionListManager->GetNumberOfSessions(), "expected sessionListManager to have one sessions");
		assertEqualm(1L, sessionListManager->fSessions.GetSize(), "expected SessionsList to be one");
		assertEqualm("SessionAccountingTest", sessionId, "expected Session to be replaced by new one");
		sessionListManager->CleanupSessions(ctx);
		assertEqualm(0L, sessionListManager->GetNumberOfSessions(), "expected sessionListManager to have one sessions");
		assertEqualm(0L, sessionListManager->fSessions.GetSize(), "expected SessionsList to be one");
	}
}

// builds up a suite of testcases, add a line for each testmethod
Test *SessionListManagerTest::suite() {
	StartTrace(SessionListManagerTest.suite);
	TestSuite *testSuite = new TestSuite;

	ADD_CASE(testSuite, SessionListManagerTest, InitFinisTest);
	ADD_CASE(testSuite, SessionListManagerTest, ReInitTest);
	ADD_CASE(testSuite, SessionListManagerTest, CreateSessionTest);
	ADD_CASE(testSuite, SessionListManagerTest, LookupSessionTest);
	ADD_CASE(testSuite, SessionListManagerTest, DisableSessionTest);
	ADD_CASE(testSuite, SessionListManagerTest, CleanupSessionTest);
	ADD_CASE(testSuite, SessionListManagerTest, StdFilterQueryTest);
	ADD_CASE(testSuite, SessionListManagerTest, SessionListInfoTest);
	ADD_CASE(testSuite, SessionListManagerTest, GetASessionsInfoTest);
	ADD_CASE(testSuite, SessionListManagerTest, SessionFactoryTest);
	ADD_CASE(testSuite, SessionListManagerTest, SessionAccountingTest);
	ADD_CASE(testSuite, SessionListManagerTest, AddSameSessionNTimesTest);
	ADD_CASE(testSuite, SessionListManagerTest, UniqueInstanceIdTest);

	return testSuite;

}
