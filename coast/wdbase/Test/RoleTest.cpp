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
#include "Role.h"

//--- interface include --------------------------------------------------------
#include "RoleTest.h"

//--- standard modules used ----------------------------------------------------
#include "Registry.h"
#include "Context.h"

//--- c-library modules used ---------------------------------------------------
#if !defined(WIN32)
#include <stdlib.h>
#endif

//---- RoleTest ----------------------------------------------------------------
RoleTest::RoleTest(TString tname)
	: TestCaseType(tname)
{
	StartTrace(RoleTest.RoleTest);
}

RoleTest::~RoleTest()
{
	StartTrace(RoleTest.Dtor);
}

void RoleTest::setUp ()
{
	StartTrace(RoleTest.setUp);
	t_assert(Role::FindRole("Role") != NULL);
	t_assert(GetConfig().IsDefined("Roles"));
	t_assert(GetConfig().IsDefined("Modules"));
	t_assert(GetConfig()["Modules"].Contains("RolesModule"));
}

void RoleTest::GetNewPageName ()
{
	StartTrace(RoleTest.GetNewPageName);
	// this test assumes a specific configuration for the
	// roles configured in RoleTest.any
	String pagename;
	Role *r = Role::FindRole("RTGuest");
	t_assertm(r != 0, "Role RTGuest not found");
	Anything dummy;
	Context ctx(dummy, dummy, 0, 0, 0, 0);
	if (r) {
		ctx.SetRole(r);
		pagename = ""; // look for Default Map
		String transition = "Home";
		t_assert(r->GetNewPageName(ctx, transition, pagename));
		assertEqual("GuestPage", pagename);
		assertEqual("PreprocessAction", transition);

		transition = "StayOnSamePage";
		t_assert(r->GetNewPageName(ctx, transition, pagename));
		assertEqual("GuestPage", pagename);
		assertEqual("PreprocessAction", transition);

		transition = "Login";
		t_assert(r->GetNewPageName(ctx, transition, pagename));
		assertEqual("Home", pagename);
		assertEqual("Login", transition);

		pagename = "Login";
		transition = "LoginOK"; // look into role specific map
		t_assert(r->GetNewPageName(ctx, transition, pagename));
		assertEqual("CustomerPage", pagename);
		assertEqual("PreprocessAction", transition);;

		transition = "StayOnSamePage";
		t_assert(r->GetNewPageName(ctx, transition, pagename));
		assertEqual("CustomerPage", pagename);
		assertEqual("PreprocessAction", transition);

		pagename = "AnyPageYouLike";
		transition = "StayOnSamePage";
		t_assert(r->GetNewPageName(ctx, transition, pagename));
		assertEqual("AnyPageYouLike", pagename);;
		assertEqual("PreprocessAction", transition);

		pagename = "GuestPage";
		transition = "LoginOK";
		t_assert(!r->GetNewPageName(ctx, transition, pagename));
		assertEqual("GuestPage", pagename);;
		assertEqual("LoginOK", transition);

		pagename = "GuestPage";
		transition = "AnUndefinedToken";
		t_assert(!r->GetNewPageName(ctx, transition, pagename));
		assertEqual("GuestPage", pagename);;
		assertEqual("AnUndefinedToken", transition);
// transition token found in Map
		pagename = "GuestPage";
		transition = "GoBanner";
		t_assert(r->GetNewPageName(ctx, transition, pagename));
		assertEqual("BannerPage", pagename);;
		assertEqual("PreprocessAction", transition);

// StayOnSamePageTokens
		pagename = "BannerPage";
		transition = "LoginRTCustomer";
		t_assert(r->GetNewPageName(ctx, transition, pagename));
		assertEqual("BannerPage", pagename);;
		assertEqual("PreprocessAction", transition);

		pagename = "AnyPageYouLike";
		transition = "LoginRTCustomer";
		t_assert(r->GetNewPageName(ctx, transition, pagename));
		assertEqual("AnyPageYouLike", pagename);;
		assertEqual("PreprocessAction", transition);
	}
}

void RoleTest::Synchronize ()
{
	// this test is somehow dummy, because Role::Synchronize does not do much
	// it is a hook for Roles to be implemented by subclasses
	Role *r = Role::FindRole("RTCustomer");
	t_assertm(r != 0, "Role RTCustomer not found");
	Anything dummy;
	Context ctx(dummy, dummy, 0, 0, 0, 0);
	if (r) {
		t_assert(r->Synchronize(ctx));

	}
	r =  Role::FindRole("RTGuest");
	t_assertm(r != 0, "Role RTGuest not found");
	if (r) {
		t_assert(r->Synchronize(ctx));
	}
}
void RoleTest::CollectLinkState ()
{
	StartTrace(RoleTest.CollectLinkState);
	// this test assumes a specific configuration for the
	// roles configured in RoleTest.any and RTCustomer.any
	String pagename;
	Role *r = Role::FindRole("RTCustomer");
	t_assertm(r != 0, "Role RTCustomer not found");
	Anything dummy;
	Context ctx(dummy, dummy, 0, 0, 0, 0);
	Anything qfa;
	qfa["namefield"] = "ifs";
	qfa["pwfield"] = "unbekannt";
	ctx.GetQuery()["fields"] = qfa; // should always succeed
	assertEqual("ifs", ctx.GetQuery()["fields"]["namefield"].AsCharPtr(0));
	ctx.GetQuery()["dummyfield"] = "hallo";
	ctx.GetTmpStore()["pwfield"] = "Geheim"; // should stay there
	if (r) {
		ROAnything rodummy;
		t_assert(r->Lookup("StateFull", rodummy));
		r->PrepareTmpStore(ctx);
		Anything state;
		r->CollectLinkState(state, ctx);
		assertEqual("Geheim", state["pwfield"].AsCharPtr(0));
		assertEqual("ifs", state["namefield"].AsCharPtr(0));
		assertEqual("hallo", state["dummyfield"].AsCharPtr(0));
		String sname;
		t_assert(r->GetName(sname));
		t_assert(sname.Length() > 0);
		assertEqual(sname, state["role"].AsCharPtr(0));

		assertEqual(60L, r->GetTimeout());
	}
}

void RoleTest::PrepareTmpStore ()
{
	StartTrace(RoleTest.PrepareTmpStore);
	// this test assumes a specific configuration for the
	// roles configured in RoleTest.any and RTCustomer.any
	String pagename;
	Role *r = Role::FindRole("RTCustomer");
	t_assertm(r != 0, "Role RTCustomer not found");
	Anything dummy;
	Context ctx(dummy, dummy, 0, 0, 0, 0);
	Anything qfa;
	qfa["namefield"] = "ifs";
	qfa["pwfield"] = "unbekannt";
	ctx.GetQuery()["fields"] = qfa; // should always succeed
	assertEqual("ifs", ctx.GetQuery()["fields"]["namefield"].AsCharPtr(0));
	ctx.GetQuery()["dummyfield"] = "hallo";
	ctx.GetTmpStore()["pwfield"] = "Geheim"; // should stay there
	if (r) {
		ROAnything rodummy;
		t_assert(r->Lookup("StateFull", rodummy));
		r->PrepareTmpStore(ctx);
		assertEqual("Geheim", ctx.GetTmpStore()["pwfield"].AsCharPtr(0));
		assertEqual("ifs", ctx.GetTmpStore()["namefield"].AsCharPtr(0));
		assertEqual("hallo", ctx.GetTmpStore()["dummyfield"].AsCharPtr(0));

		assertEqual(60L, r->GetTimeout());
	}
}

void RoleTest::VerifyLevel ()
{
	StartTrace(RoleTest.VerifyLevel);
	// this test assumes a specific configuration for the
	// roles configured in RoleTest.any
	String pagename;
	Role *r = Role::FindRole("RTCustomer");
	t_assertm(r != 0, "Role RTCustomer not found");
	Anything dummy;
	Context ctx(dummy, dummy, 0, 0, 0, 0);
	ctx.GetQuery()["action"] = "Home";
	ctx.GetQuery()["role"] = "RTGuest";
	if (r) {
		String transition = ctx.GetQuery()["action"].AsString();
		String spagename = ctx.GetQuery()["page"].AsString();
		// currently this test does not succeed anymore because the role must be of the same name to be valid
		t_assert(!r->Verify(ctx, transition, spagename));
	}
	r =  Role::FindRole("RTGuest");
	t_assertm(r != 0, "Role RTGuest not found");
	ctx.GetQuery()["action"] = "Login"; // should not succeed
	ctx.GetQuery()["role"] = "RTCustomer";
	if (r) {
		String transition = ctx.GetQuery()["action"].AsString();
		String spagename = ctx.GetQuery()["page"].AsString();
		t_assert(!r->Verify(ctx, transition, spagename));
	}
}

void RoleTest::VerifyLogout ()
{
	StartTrace(RoleTest.VerifyLogout);
	// this test assumes a specific configuration for the
	// roles configured in RoleTest.any
	String pagename;
	Role *r = Role::FindRole("RTGuest");
	t_assert(r != 0);
	Anything dummy;
	Context ctx(dummy, dummy, 0, 0, 0, 0);
	ctx.GetQuery()["action"] = "Logout"; // should always succeed
	if (r) {
		String transition = ctx.GetQuery()["action"].AsString();
		String spagename = ctx.GetQuery()["page"].AsString();
		t_assert(r->Verify(ctx, transition, spagename));
	}

	ctx.GetQuery()["action"] = "GoBanner"; // should not succeed
	ctx.GetQuery()["role"] = "RTCustomer"; // checklevel should fail for Role RTGuest
	if (r) {
		String transition = ctx.GetQuery()["action"].AsString();
		String spagename = ctx.GetQuery()["page"].AsString();
		t_assert(!r->Verify(ctx, transition, spagename));
	}
}

void RoleTest::CheckInstalled ()
{
	StartTrace(RoleTest.CheckInstalled);
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
			Trace(" r says: <" << sname << ">") ;
			t_assert(r->CheckConfig(sname)); // ensure config is available
			if (r->GetSuper()) {
				String supername("null");
				r->GetSuper()->GetName(supername);
				Trace(" super <" << supername << ">");
			}
			ROAnything map;
			t_assert(r->Lookup("Map", map)); // ensure Map is configured
		}
	}
}

void RoleTest::FindRoleWithDefault()
{
	StartTrace(RoleTest.FindRoleWithDefault);

	Anything dummy;
	Context ctx(dummy, dummy, 0, 0, 0, 0);

	t_assert(Role::FindRole("Role") == Role::FindRoleWithDefault("", ctx));
	t_assert(Role::FindRole("RTGuest") == Role::FindRoleWithDefault("RTGuest", ctx));
	t_assert(Role::FindRole("RTGuest") == Role::FindRoleWithDefault("", ctx, "RTGuest"));
	t_assert(Role::FindRole("RTGuest") == Role::FindRoleWithDefault("XCXC", ctx, "RTGuest"));
	t_assert(Role::FindRole("RTCustomer") == Role::FindRoleWithDefault("RTCustomer", ctx, "RTGuest"));
}

void RoleTest::GetDefaultRoleName()
{
	StartTrace(RoleTest.GetDefaultRoleName);

	Anything dummy;
	Context ctx(dummy, dummy, 0, 0, 0, 0);

	assertEqual("Role", Role::GetDefaultRoleName(ctx));
	Anything newrole;
	newrole["DefaultRole"] = "Fantasy";
	Context::PushPopEntry<Anything> aEntry(ctx, "x", newrole);
	assertEqual("Fantasy", Role::GetDefaultRoleName(ctx));
	Context ctx2(GetConfig().DeepClone(), dummy, 0, 0, 0, 0);
	assertEqual("MyRole", Role::GetDefaultRoleName(ctx2));
}

Test *RoleTest::suite ()
{
	TestSuite *testSuite = new TestSuite;
	ADD_CASE(testSuite, RoleTest, CheckInstalled);
	ADD_CASE(testSuite, RoleTest, GetNewPageName);
	ADD_CASE(testSuite, RoleTest, VerifyLogout);
	ADD_CASE(testSuite, RoleTest, VerifyLevel);
	ADD_CASE(testSuite, RoleTest, Synchronize);
	ADD_CASE(testSuite, RoleTest, PrepareTmpStore);
	ADD_CASE(testSuite, RoleTest, CollectLinkState);
	ADD_CASE(testSuite, RoleTest, FindRoleWithDefault);
	ADD_CASE(testSuite, RoleTest, GetDefaultRoleName);
	return testSuite;
}
