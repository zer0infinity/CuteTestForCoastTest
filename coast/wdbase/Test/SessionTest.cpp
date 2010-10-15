/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

//--- interface include --------------------------------------------------------
#include "SessionTest.h"

//--- module under test --------------------------------------------------------
#include "Session.h"

//--- test modules used --------------------------------------------------------
#include "TestSuite.h"

//--- standard modules used ----------------------------------------------------
#include "Role.h"
#include "Registry.h"
#include "PoolAllocator.h"
#include "AnyIterators.h"

//---- SessionTest ----------------------------------------------------------------
SessionTest::SessionTest(TString tname)
	: TestCaseType(tname)
{
	StartTrace(SessionTest.SessionTest);
}

SessionTest::~SessionTest()
{
	StartTrace(SessionTest.Dtor);
}

void SessionTest::setUp ()
{
	StartTrace(SessionTest.setUp);
	t_assert(GetConfig().IsDefined("Roles"));
	ROAnything dummy;
	t_assert(GetConfig().LookupPath(dummy, "Roles.Role.RTGuest"));
	assertEqual("RTCustomer", dummy[0L].AsCharPtr());
	t_assert(GetConfig().IsDefined("Modules"));
}

static Role *GetDefaultRole(Context &ctx)
{
	return Role::FindRoleWithDefault(Role::GetDefaultRoleName(ctx), ctx);
}

void SessionTest::SetGetRole ()
{
	StartTrace(SessionTest.SetGetRole);
	Role *cust = Role::FindRole("RTCustomer"); // from RoleTest

	Context theCtx;
	Session s("dummysession");
	{
		Context ctx;
		// does now also update the slot RoleName
		t_assert(0 != s.GetRole(ctx));
		Role *defr = GetDefaultRole(ctx);
		t_assert(defr != 0);
		assertCompare(defr, equal_to, s.GetRole(ctx));
		assertCompare(GetDefaultRole(ctx), equal_to, s.GetRole(ctx));
		assertEqual("X", ROAnything(s.fStore)["RoleName"].AsString("X"));
		ctx.GetTmpStore()["DefaultRole"] = "RTCustomer";
		assertCompare(cust, equal_to, GetDefaultRole(ctx));
		assertCompare(cust, equal_to, s.GetRole(ctx));
		assertCharPtrEqual("X", ROAnything(s.fStore)["RoleName"].AsString("X"));

		s.SetRole(cust, ctx);
		// DoGetDefaultRole and GetRole are now returning RTCustomer Role
		assertEqual("RTCustomer", ROAnything(s.fStore)["RoleName"].AsString("X"));
		assertCompare(cust, equal_to, s.GetRole(ctx));

		t_assert(defr != s.GetRole(ctx));
		t_assert(cust == s.GetRole(ctx));

		Role *defr2 = GetDefaultRole(ctx);
		t_assert(defr2 != 0);
		t_assert(defr2 == cust);

		s.SetRole(defr, ctx);
		assertEqual("Role", ROAnything(s.fStore)["RoleName"].AsString("X"));

		s.SetRole(0, ctx);
		assertEqual("none", ROAnything(s.fStore)["RoleName"].AsString("X"));
		t_assert(!s.GetRole(ctx));
	}
}

void MakeDummyArg(Anything &arg)
{
	arg["env"]["REMOTE_ADDR"] = "127.0.0.1";
	arg["env"]["HTTP_USER_AGENT"] = "Testframework";
	arg["query"]["adr"] = "127.0.0.2";
	arg["query"]["port"] = 2412;
}

void SessionTest::TestInit ()
{
	StartTrace(SessionTest.TestInit);
	Anything arg;
	MakeDummyArg(arg);
	Context theCtx;

	Session s("dummysession");
	{
		Context ctx(arg);;
		s.Init("sessionid", ctx);
		t_assert(0 != s.GetRole(ctx));
		assertEqual("sessionid", s.GetId());
		t_assert(s.Verify(ctx));
		assertEqual("Testframework", s.fBrowser);
		assertEqual("127.0.0.1", s.fRemoteAddr);
		assertEqual("127.0.0.2", s.fAddress);
		assertEqual(2412, s.fPort);
	}
}

void SessionTest::DoFindNextPageLogin ()
{
	StartTrace(SessionTest.DoFindNextPageLogin);

	Anything arg;
	MakeDummyArg(arg);
	Context theCtx;
	Session s("dummysession");
	{
		Context ctx(arg);

		s.Init("sessionid", ctx);

		Context c;
		Anything args;
		Anything env;
		String p("BannerPage");
		// fake global config in env
		args["env"]["RoleChanges"]["LoginOK"] = "RTCustomer";

		args["query"]["page"] = p;
		args["query"]["role"] = "RTCustomer"; // trigger login
		args["query"]["action"] = "GoHome";
		args["query"]["Language"] = "F";
		c.PushRequest(args);
		c.Push(&s);
		String t;
		s.SetupContext(c, t, p);
		s.DoFindNextPage(c, t, p);
		assertEqual("Login", t); // depends on configuration
		assertEqual("Home", p);
		t_assert(s.fStore.IsDefined("delayed"));
		t_assert(s.fStore["delayed"].GetSize() == 1);
		assertEqual("GoHome", s.fStore["delayed"][0L]["action"].AsString("X"));
		assertEqual("RTCustomer", s.fStore["delayed"][0L]["role"].AsString("X"));
		assertEqual("BannerPage", s.fStore["delayed"][0L]["page"].AsString("X"));
		assertEqual("F", c.Language());
		assertEqual("F", s.fStore["delayed"][0L]["Language"].AsString("X"));
		assertEqual("F", c.GetTmpStore()["Language"].AsString("X"));
		TraceAny(c.GetTmpStore(), "Tempstore");

		t = "LoginOK";
		args["query"] = Anything(Anything::ArrayMarker());
		args["query"]["action"] = t;
		args["query"]["page"] = p;
		args["query"]["delayedIndex"] = "0";
		args["query"]["Language"] = "E";
		Context c2;
		c2.PushRequest(args);
		c2.Push(&s);
		s.SetupContext(c2, t, p);
		s.DoFindNextPage(c2, t, p);
		String rname;
		s.GetRole(c2)->GetName(rname);
		assertEqual("RTCustomer", rname);
		assertEqual("CustomerHome", p);
		assertEqual("PreprocessAction", t); // for dealing with legacy page classes

		// The new value for Language goes into TmpStore because of state full processing
		// but the query gets the old value from the delayed query
		assertEqual("E", c2.Language());
		assertEqual("F", c2.GetQuery()["Language"].AsString("X"));
		assertEqual("E", c2.GetTmpStore()["Language"].AsString("X"));
		TraceAny(c2.GetTmpStore(), "Tempstore2");
	}
}

void SessionTest::RetrieveFromDelayed ()
{
	StartTrace(SessionTest.RetrieveFromDelayed);
	Anything arg;
	MakeDummyArg(arg);
	Context theCtx;
	Session s("dummysession");

	{
		Context ctx(arg);
		s.Init("sessionid", ctx);

		Anything query;
		Anything env;
		String p("Shouldbedelayed");
		query["page"] = p;
		{
			Context c(env, query, 0, &s, 0, 0);
			String t;
			s.SetupContext(c, t, p);

			s.SaveToDelayed(c, t, p);
			assertEqual(1, s.fStore["delayed"].GetSize());
			Anything q = c.GetQuery();
			t_assert(q.IsDefined("delayedIndex"));
			assertEqual(0L, q["delayedIndex"].AsLong(-1));
			s.RetrieveFromDelayed(c, t, p);
			Anything qafter = c.GetQuery();
			assertEqual(query["page"].AsCharPtr("before"), qafter["page"].AsCharPtr("after"));
		}
	}
}

void SessionTest::CheckInstalled ()
{
	StartTrace(SessionTest.CheckInstalled);
	Registry *reg = Registry::GetRegistry("Role");
	t_assert(reg != 0);
	RegistryIterator ri(reg);

	while ( ri.HasMore() ) {
		String roleName("null");
		Role *r = (Role *)ri.Next(roleName);
		Trace("role found <" << roleName << ">");
		if (r) {
			String sname("null");
			t_assert(r->GetName(sname));
			Trace("r says: <" << sname << ">");
			t_assert(r->IsInitialized()); // ensure config is available
			if (r->GetSuper()) {
				String supername("null");
				r->GetSuper()->GetName(supername);
				Trace("super <" << supername << ">");
			}
			ROAnything map;
			t_assert(r->Lookup("Map", map)); // ensure Map is configured
		}
	}
}

void SessionTest::UseSessionStoreTest()
{
	StartTrace(SessionTest.UseSessionStoreTest);
	PoolAllocator pa(1, 8 * 1024, 21);
	TestStorageHooks tsh(&pa);

	Context theCtx;
	Session s("TestSession");
	{
		Context ctx;

		Anything local;
		Anything global(Storage::Global());
		ctx.Push(&s);

		//--- manipulate session store trough context
		global = ctx.GetSessionStore();
		assertAnyEqualm(global, ctx.GetSessionStore(), "should be still the same since they are aliased");
		// do some manipulations
		global["Test"] = "try";

		//--- should be still the same since they are aliased
		assertAnyEqualm(global, ctx.GetSessionStore(), "should be the same since they are aliased");
		assertEqualm("try", ctx.Lookup("Test", "not found"), "context should find session store items");

		//--- adding items to session directly
		s.PutInStore("Test1", "next try");

		//--- should be still the same since they are aliased
		assertAnyEqualm(global, ctx.GetSessionStore(), "should be the same since they are aliased");
		assertEqualm("next try", ctx.Lookup("Test1", "not found"), "context should find session store items");

		//--- WE CANNOT manipulate session store trough context if anything uses local allocator
		local = ctx.GetSessionStore();
		// do some manipulations
		local["Test2"] = "try";

		//--- should not be found since we have a local copy
		assertEqualm("not found", ctx.Lookup("Test2", "not found"), "context should not find copied session store items");

		//--- adding items to session directly
		s.PutInStore("Test3", "next try");
		local["Test3"] = "failed try";

		//--- should be still the same since they are aliased
		assertEqualm("next try", ctx.Lookup("Test3", "not found"), "context should find session store items");
	}
}

void SessionTest::VerifyTest()
{
	StartTrace(SessionTest.VerifyTest);

	Context theCtx;
	ROAnything cConfig;
	AnyExtensions::Iterator<ROAnything> aEntryIterator(GetTestCaseConfig());
	while ( aEntryIterator.Next(cConfig) ) {
		Session s("TestSession");
		{
			Anything args;
			args["env"] = cConfig["Session"].DeepClone();
			Context ctx(args);
			s.Init("TestSession", ctx);
			ctx.GetEnvStore() = cConfig["Env"].DeepClone();
			ctx.Push(&s);
			t_assertm(cConfig["ExpectedResult"].AsBool() == s.Verify(ctx), "expected verification to fail");
		}
	}
}

void SessionTest::InfoTest()
{
	StartTrace(SessionTest.InfoTest);
	{
		// simple case no environment no session settings
		Context theCtx;
		Session s("TestSession");
		{
			Context ctx;
			ctx.Push(&s);
			Anything resultedInfo;
			Anything expectedInfo;

			expectedInfo["Role"] = Role::GetDefaultRoleName(ctx);
			expectedInfo["Accessed"] = s.GetAccessCounter();
			expectedInfo["Last"] = s.GetAccessTime();
			expectedInfo["Timeout"] = s.GetTimeout(ctx);
			expectedInfo["Referenced"] = s.GetRefCount();
			expectedInfo["SessionStore"]["RoleStore"] = Anything(Anything::ArrayMarker());
			TraceAny(expectedInfo, "expected:");
			t_assertm(s.Info(resultedInfo, ctx), "expected info to succeed");
			assertAnyEqual(expectedInfo, resultedInfo);
			TraceAny(resultedInfo, "Result:");
		}
	}
}

void SessionTest::IsDeletableTest()
{
	StartTrace(SessionTest.IsDeletableTestTest);
	{
		// simple case no environment no session settings
		Context theCtx;
		long notTimeout = time(0) + 1;
		long isTimeout = time(0) + 200000;
		Session s("TestSession");
		s.ResetAccessTime(); // set time to "now"
		{
			// This one fails because the session is still referenced by the context
			Context ctx;
			ctx.Push(&s);
			// This one fails because the session is referenced from the context object
			assertEqual(false, s.IsDeletable(isTimeout, ctx, false));
			t_assert(ctx.GetSession()->GetRefCount() == 1);
			// This one fails too because the session is referenced from the context object
			// although the timeout doesn't matter here (DisabledSessions)
			assertEqual(false, s.IsDeletable(isTimeout, ctx, true));
			t_assert(ctx.GetSession()->GetRefCount() == 1);
		}

		// Now the context unreferenced the session in its destructor

		t_assert(s.GetRefCount() == 0);

		// The session is not removable because it is not timeout (but unreferenced)
		assertEqual(false, s.IsDeletable(notTimeout, theCtx, false));
		// The session is  removable because it its timeout is not relevant (DisabledSessions)
		// and it is unreferenced
		assertEqual(true, s.IsDeletable(notTimeout, theCtx, true));
		// The session is now removable because it is  timeout (and unreferenced)
		assertEqual(true,  s.IsDeletable(isTimeout, theCtx, false));

	}
}

void SessionTest::SetupContextTest()
{
	StartTrace(SessionTest.SetupContextTest);

	Anything arg;
	MakeDummyArg(arg);
	Context theContext;
	Session s("dummysession");
	{
		Context ctx(arg);

		s.Init("sessionid", ctx);

		Context c;
		Anything args;
		String p("BannerPage");
		// fake global config in env

		args["query"]["page"] = p;
		args["query"]["action"] = "";
		c.PushRequest(args);
		c.Push(&s);
		String t;
		s.SetupContext(c, t, p);
		s.DoFindNextPage(c, t, p);
		assertEqual("PreprocessAction", t);
		assertEqual("GuestPage", p);

		// Check with Error Page -
		c.GetTmpStore()["DefaultAction"] = "misconfigured";
		s.SetupContext(c, t, p);
		s.DoFindNextPage(c, t, p);
		assertEqual("misconfigured", t);
		assertEqual("ErrorPage", p);
	}
}
class STTestSession : public Session
{
public:
	STTestSession(const char *nm) : Session(nm) {}
	Role *PublicCheckRoleExchange(const char *t, Context &ctx) {
		return CheckRoleExchange(t, ctx);
	}
};

void SessionTest::IntCheckRoleExchange(const char *source_role, const char *target_role, const char *transition, STTestSession &s, Context &theCtx, bool should_succeed = true)
{
	Role *rs = Role::FindRole(source_role);
	s.SetRole(rs, theCtx);

	Role *rt = s.PublicCheckRoleExchange(transition, theCtx);

	if (should_succeed) {
		if (t_assertm(rt != 0, TString("srcR [") << source_role << "] transi [" << transition << "] trgR [" << target_role << "]")) {
			String rn;
			rt->GetName(rn);
			assertCharPtrEqualm(target_role, rn, TString("srcR [") << source_role << "] transi [" << transition << "] trgR [" << target_role << "]");
		}
	} else {
		t_assertm(!rt, TString("srcR [") << source_role << "] transi [" << transition << "] trgR [" << target_role << "]");
	}
}

void SessionTest::CheckRoleExchangeTest()
{
	StartTrace(SessionTest.CheckRoleExchangeTest);

	Context theCtx;
	Context::PushPopEntry<ROAnything> aEntry(theCtx, "fConfig", GetConfig());

	STTestSession s("dummysession");

	IntCheckRoleExchange("Role", "RTCustomer", "LoginOk", s, theCtx);
	IntCheckRoleExchange("RTCustomer", "RTGuest", "Logoff", s, theCtx);
	IntCheckRoleExchange("RTGuest", "Role", "Logoff", s, theCtx);

	IntCheckRoleExchange("RTGuest", "Role", "Blah", s, theCtx, false);
	IntCheckRoleExchange("Role", "RTCustomer", "Logoff", s, theCtx, false);
}

Test *SessionTest::suite ()
{
	TestSuite *testSuite = new TestSuite;
	ADD_CASE(testSuite, SessionTest, CheckInstalled);
	ADD_CASE(testSuite, SessionTest, SetGetRole);
	ADD_CASE(testSuite, SessionTest, TestInit);
	ADD_CASE(testSuite, SessionTest, RetrieveFromDelayed);
	ADD_CASE(testSuite, SessionTest, DoFindNextPageLogin);
	ADD_CASE(testSuite, SessionTest, UseSessionStoreTest);
	ADD_CASE(testSuite, SessionTest, VerifyTest);
	ADD_CASE(testSuite, SessionTest, InfoTest);
	ADD_CASE(testSuite, SessionTest, IsDeletableTest);
	ADD_CASE(testSuite, SessionTest, SetupContextTest);
	ADD_CASE(testSuite, SessionTest, CheckRoleExchangeTest);
	return testSuite;
}
