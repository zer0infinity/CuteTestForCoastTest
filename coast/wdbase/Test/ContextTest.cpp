/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#include "ContextTest.h"
#include "Context.h"
#include "TestSuite.h"
#include "StringStreamSocket.h"
#include "Page.h"
#include "Session.h"
#include "Role.h"
#include "Server.h"
#include "Dbg.h"
#include "Renderer.h"

//---- ContextTest ----------------------------------------------------------------
ContextTest::ContextTest(TString tname)
	: TestCaseType(tname)
{
	StartTrace(ContextTest.ContextTest);
}

ContextTest::~ContextTest()
{
}

void ContextTest::setUp ()
{
	t_assert(GetConfig().IsDefined("Modules"));
	Server *s;
	if (t_assert((s = Server::FindServer("Server")) != NULL)) {
		ROAnything result;
		t_assert(s->Lookup("TCP5010", result));
	}
}

void ContextTest::RequestConstructorTest ()
{
	Anything args;
	args["env"]["foo"] = "foo";
	args["env"]["header"] = "test";
	args["query"]["Button"] = "DoIt";
	args["query"]["Field"] = "None";

	Context ctx(args["env"], args["query"], 0, 0, 0);
	Context ctx1(args);

	assertEqual(ctx.Lookup("foo", "wrong"), ctx1.Lookup("foo", "wrong"));
	assertEqual(ctx.Lookup("header", "wrong"), ctx1.Lookup("header", "wrong"));
	assertEqual(ctx.Lookup("Button", "wrong"), ctx1.Lookup("Button", "wrong"));
	assertEqual(ctx.Lookup("Field", "wrong"), ctx1.Lookup("Field", "wrong"));
} // RequestConstructorTest

void ContextTest::EmptyConstructorTest ()
{
	// accessor tests
	Anything empty;
	Anything tmpStore = Anything(Anything::ArrayMarker());
	Anything result;
	String popStoreKey("noStore");

	Context ctx;

	// request is not empty since Init calls LocalizationUtils::FindLanguageKey which useses env
//	assertAnyEqual(empty, ctx.GetRequest());

	// some lookup tests
	assertEqual("right", ctx.Lookup("foo", "right"));
	assertEqual(1L, ctx.Lookup("foo", 1L));
	assertEqual("yes", ctx.Lookup("foo").AsCharPtr("yes"));

	assertEqual((long)0, (long)ctx.GetSession());
	assertEqual((long)0, (long)ctx.GetSessionId());
	assertEqual((long)0, (long)ctx.GetSocket());
	assertEqual((long)0, (long)ctx.GetStream());
	t_assert(ctx.GetStore("noKey", result) == false);
	assertAnyEqual(empty, result);
	t_assert(ctx.GetStore("tmp", result));
	assertAnyEqualm(tmpStore, result, "tmpStore should always be there");
	String strKey;
	t_assert(ctx.Pop(strKey) == false);
	t_assert(ctx.PopStore(popStoreKey) == false);
	assertEqual("noStore", popStoreKey);
	t_assert(ctx.GetStore("tmp", result));
	assertAnyEqualm(tmpStore, result, "tmpStore should still be there");
}

void ContextTest::SocketCtorTests()
{
	StartTrace(ContextTest.SocketCtorTests);
	Anything empty;
	Anything clientinfo;
	clientinfo["REMOTE_ADDR"] = "localhost";
	clientinfo["HTTPS"] = false;

	//--- normal case ------------------
	String str("Test");
	String control;
	{
		StringStreamSocket sss(str);
		Context ctx(&sss);

		Socket *s = ctx.GetSocket();
		t_assert(s != 0);
		if (s) {
			*(s->GetStream()) >> control;
		}
		assertEqualm("Test", control, "expected String from Socket");
		assertAnyEqualm(clientinfo, ctx.Lookup("ClientInfo"), "expected ClientInfo from Socket");
	}

	//--- no socket case ---------
	{
		Context ctx;

		Socket *s = ctx.GetSocket();
		t_assert(!s);
		assertAnyEqualm(empty, ctx.Lookup("ClientInfo"), "expected ClientInfo to be empty");
	}

	//--- null socket pointer case ---------
	{
		Context ctx((Socket *)0);

		Socket *s = ctx.GetSocket();
		t_assert(!s);
		assertAnyEqualm(empty, ctx.Lookup("ClientInfo"), "expected ClientInfo to be empty");
	}
}

void ContextTest::VerySimplePush()
{
	Context ctx;
	Role *r = Role::FindRole("Role");
	t_assert(r);
	ctx.SetRole(r);
	assertEqual((long)r, (long)ctx.GetRole());
}

void ContextTest::SimplePushNoPop()
{
	Context ctx;

	//empty context always returns default information
	assertEqual("right", ctx.Lookup("foo", "right"));
	assertEqual(1L, ctx.Lookup("foo", 1L));
	assertEqual("yes", ctx.Lookup("foo").AsCharPtr("yes"));

	//push a lookup interface
	ctx.Push("Role", Role::FindRole("Role"));

	//lookups find everything in the first object
	assertEqual("Role", ctx.Lookup("ContextItem", "not found"));
	assertEqual("found", ctx.Lookup("ContextTestRole", "not found"));
	assertEqual("not there", ctx.Lookup("ContextTestPage").AsCharPtr("not there"));

	//push a second interface
	ctx.Push("Page", Page::FindPage("Page"));

	//lookups find things first in the second object than in the first object
	assertEqual("Page", ctx.Lookup("ContextItem", "not found"));
	assertEqual("found", ctx.Lookup("ContextTestRole", "not found"));
	assertEqual("found", ctx.Lookup("ContextTestPage").AsCharPtr("not found"));

}

void ContextTest::SimpleNamedPushPop()
{
	Context ctx;

	//empty context always returns default information
	assertEqual("right", ctx.Lookup("foo", "right"));
	assertEqual(1L, ctx.Lookup("foo", 1L));
	assertEqual("yes", ctx.Lookup("foo").AsCharPtr("yes"));

	//push a lookup interface
	ctx.Push("Role", Role::FindRole("Role"));

	//lookups find everything in the first object
	assertEqual("Role", ctx.Lookup("ContextItem", "not found"));
	assertEqual("found", ctx.Lookup("ContextTestRole", "not found"));
	assertEqual("not there", ctx.Lookup("ContextTestPage").AsCharPtr("not there"));

	//push a second interface
	ctx.Push("Page", Page::FindPage("Page"));

	//lookups find things first in the second object than in the first object
	assertEqual("Page", ctx.Lookup("ContextItem", "not found"));
	assertEqual("found", ctx.Lookup("ContextTestRole", "not found"));
	assertEqual("found", ctx.Lookup("ContextTestPage").AsCharPtr("not found"));

	//pop the interface
	String strKey;
	t_assert(ctx.Pop(strKey));

	//lookups find everything in the first object
	assertEqual("Role", ctx.Lookup("ContextItem", "not found"));
	assertEqual("found", ctx.Lookup("ContextTestRole", "not found"));
	assertEqual("not there", ctx.Lookup("ContextTestPage").AsCharPtr("not there"));

	//pop the interface
	t_assert(ctx.Pop(strKey));

	//empty context always returns default information
	assertEqual("right", ctx.Lookup("foo", "right"));
	assertEqual(1L, ctx.Lookup("foo", 1L));
	assertEqual("yes", ctx.Lookup("foo").AsCharPtr("yes"));

	ctx.Push("Page", 0);
	//pop the interface should fail
	t_assert(ctx.Pop(strKey) == false);
	assertEqual(0, ctx.fStackSz);

}

void ContextTest::FindReplace()
{
	Context ctx;

	//empty context always returns default information
	assertEqual("right", ctx.Lookup("foo", "right"));
	assertEqual(1L, ctx.Lookup("foo", 1L));
	assertEqual("yes", ctx.Lookup("foo").AsCharPtr("yes"));

	//push a lookup interface
	Role *r = Role::FindRole("Role");
	ctx.Push("Role", r );

	//lookups find everything in the first object
	t_assert(r == ctx.GetRole());
	assertEqual("Role", ctx.Lookup("ContextItem", "not found"));
	assertEqual("found", ctx.Lookup("ContextTestRole", "not found"));
	assertEqual("not there", ctx.Lookup("ContextTestPage").AsCharPtr("not there"));
	assertEqual("not found", ctx.Lookup("InGuestOnly", "not found"));

	//push a second interface
	Page *p = Page::FindPage("Page");
	ctx.Push("Page", p);

	t_assert(r == ctx.GetRole());
	t_assert(p == ctx.GetPage());
	//lookups find things first in the second object than in the first object
	assertEqual("Page", ctx.Lookup("ContextItem", "not found"));
	assertEqual("found", ctx.Lookup("ContextTestRole", "not found"));
	assertEqual("found", ctx.Lookup("ContextTestPage").AsCharPtr("not found"));
	assertEqual(2, ctx.fStackSz);

	Role *rGuest = Role::FindRole("Guest");
	ctx.Replace("Role", rGuest);
	t_assert(p == ctx.GetPage());
	t_assert(rGuest == ctx.GetRole());
	assertEqual("Page", ctx.Lookup("ContextItem", "not found"));
	assertEqual("found", ctx.Lookup("ContextTestPage").AsCharPtr("not found"));
	assertEqual("found in Guest", ctx.Lookup("ContextTestRole", "not found"));
	assertEqual("SecretKey", ctx.Lookup("InGuestOnly", "not found"));
	assertEqual(2, ctx.fStackSz);

	ctx.Replace("Role", Role::FindRole("Role"));
	assertEqual("not found anymore", ctx.Lookup("InGuestOnly", "not found anymore"));
	t_assert(p == ctx.GetPage());
	t_assert(r == ctx.GetRole());
	assertEqual("Page", ctx.Lookup("ContextItem", "not found"));
	assertEqual("found", ctx.Lookup("ContextTestPage").AsCharPtr("not found"));
	assertEqual("found", ctx.Lookup("ContextTestRole", "not found"));
	assertEqual(2, ctx.fStackSz);

	// no object with name AnotherRole is there - it gets pushed
	ctx.Replace("AnotherRole", Role::FindRole("Guest"));
	t_assert(p == ctx.GetPage());
	t_assert(r == ctx.GetRole());
	assertEqual("Role", ctx.Lookup("ContextItem", "not found"));
	assertEqual("found", ctx.Lookup("ContextTestPage").AsCharPtr("not found"));
	assertEqual("found in Guest", ctx.Lookup("ContextTestRole", "not found"));
	assertEqual("SecretKey", ctx.Lookup("InGuestOnly", "not found"));
	assertEqual(3, ctx.fStackSz);

	String strKey;
	t_assert(ctx.Pop(strKey));
	assertEqual("not found anymore", ctx.Lookup("InGuestOnly", "not found anymore"));
	t_assert(p == ctx.GetPage());
	t_assert(r == ctx.GetRole());
	assertEqual("Page", ctx.Lookup("ContextItem", "not found"));
	assertEqual("found", ctx.Lookup("ContextTestPage").AsCharPtr("not found"));
	assertEqual("found", ctx.Lookup("ContextTestRole", "not found"));

	ctx.Replace("Null", 0);
	assertEqual(2, ctx.fStackSz);
	assertEqual("not found anymore", ctx.Lookup("InGuestOnly", "not found anymore"));
	t_assert(p == ctx.GetPage());
	t_assert(r == ctx.GetRole());
	assertEqual("Page", ctx.Lookup("ContextItem", "not found"));
	assertEqual("found", ctx.Lookup("ContextTestPage").AsCharPtr("not found"));
	assertEqual("found", ctx.Lookup("ContextTestRole", "not found"));

	// attack the role
	ctx.Replace("Role", 0);
	assertEqual(2, ctx.fStackSz);
	assertEqual("not found anymore", ctx.Lookup("InGuestOnly", "not found anymore"));
	t_assert(p == ctx.GetPage());
	t_assert(r == ctx.GetRole());
	assertEqual("Page", ctx.Lookup("ContextItem", "not found"));
	assertEqual("found", ctx.Lookup("ContextTestPage").AsCharPtr("not found"));
	assertEqual("found", ctx.Lookup("ContextTestRole", "not found"));

	ctx.Replace(0, 0);
	assertEqual(2, ctx.fStackSz);
	assertEqual("not found anymore", ctx.Lookup("InGuestOnly", "not found anymore"));
	t_assert(p == ctx.GetPage());
	t_assert(r == ctx.GetRole());
	assertEqual("Page", ctx.Lookup("ContextItem", "not found"));
	assertEqual("found", ctx.Lookup("ContextTestPage").AsCharPtr("not found"));
	assertEqual("found", ctx.Lookup("ContextTestRole", "not found"));

}

void ContextTest::RemoveTest()
{
	StartTrace(ContextTest.RemoveTest);
	Context ctx;
	assertEqual(0, ctx.fStackSz);

	//empty context always returns default information
	assertEqual("right", ctx.Lookup("foo", "right"));
	assertEqual(1L, ctx.Lookup("foo", 1L));
	assertEqual("yes", ctx.Lookup("foo").AsCharPtr("yes"));

	//push a lookup interface
	ctx.Push("Role", Role::FindRole("Role"));
	assertEqual(1, ctx.fStackSz);

	//lookups find everything in the first object
	assertEqual("Role", ctx.Lookup("ContextItem", "not found"));
	assertEqual("found", ctx.Lookup("ContextTestRole", "not found"));
	assertEqual("not there", ctx.Lookup("ContextTestPage").AsCharPtr("not there"));
	assertEqual("not found", ctx.Lookup("InGuestOnly", "not found"));

	//push a second interface
	ctx.Push("Page", Page::FindPage("Page"));
	assertEqual(2, ctx.fStackSz);

	//lookups find everything in the first object
	assertEqual("Page", ctx.Lookup("ContextItem", "not found"));
	assertEqual("found", ctx.Lookup("ContextTestRole", "not found"));
	assertEqual("found", ctx.Lookup("ContextTestPage").AsCharPtr("not there"));
	assertEqual("not found", ctx.Lookup("InGuestOnly", "not found"));

	ctx.Remove("Role");
	assertEqual(1, ctx.fStackSz);

	//empty context always returns default information
	assertEqual("right", ctx.Lookup("foo", "right"));
	assertEqual(1L, ctx.Lookup("foo", 1L));
	assertEqual("yes", ctx.Lookup("foo").AsCharPtr("yes"));

	//lookups find things first in the second object than in the first object
	assertEqual("Page", ctx.Lookup("ContextItem", "not found"));
	assertEqual("not found", ctx.Lookup("ContextTestRole", "not found"));
	assertEqual("found", ctx.Lookup("ContextTestPage").AsCharPtr("not found"));

	ctx.Remove("NotThere");
	assertEqual(1, ctx.fStackSz);

	ctx.Remove("Page");
	assertEqual(0, ctx.fStackSz);

	//lookups find everything in the first object
	assertEqual("not found", ctx.Lookup("ContextItem", "not found"));
	assertEqual("not found", ctx.Lookup("ContextTestRole", "not found"));
	assertEqual("not there", ctx.Lookup("ContextTestPage").AsCharPtr("not there"));
	assertEqual("not found", ctx.Lookup("InGuestOnly", "not found"));
}

void ContextTest::LookupTests ()
{
	SimplePushNoPop();
	SimpleNamedPushPop();
} // LookupTests

void ContextTest::SimplePushPop()
{
	// create an empty context
	Context ctx;
	String popStoreKey;

	//lookup some item; which are not there
	//empty context always returns default information
	assertEqual("right", ctx.Lookup("foo", "right"));
	assertEqual(1L, ctx.Lookup("foo", 1L));
	assertEqual("yes", ctx.Lookup("foo").AsCharPtr("yes"));

	//define an initial content of a store
	Anything request = Anything(Anything::ArrayMarker());
	request["header"]["HOST"] = "sentosa";
	request["header"]["URI"] = "/";
	request["body"] = "test&it&now";

	ctx.Push("StoreTest.request", request);

	//now the information of the store should be found
	assertEqual("sentosa", ctx.Lookup("header.HOST", "right"));
	assertEqual("/", ctx.Lookup("header.URI", "no"));
	assertEqual("test&it&now", ctx.Lookup("body").AsCharPtr("yes"));

	t_assert(ctx.PopStore(popStoreKey));
	assertEqual("StoreTest.request", popStoreKey);

	//now the information of the store should no longer be there
	assertEqual("right", ctx.Lookup("header.HOST", "right"));
	assertEqual("no", ctx.Lookup("header.URI", "no"));
	assertEqual("yes", ctx.Lookup("body").AsCharPtr("yes"));
}

void ContextTest::SimplePushWithEmptyStore()
{
	// create an empty context
	Context ctx;

	//lookup some item; which are not there
	//empty context always returns default information
	assertEqual("right", ctx.Lookup("foo", "right"));
	assertEqual(1L, ctx.Lookup("foo", 1L));
	assertEqual("yes", ctx.Lookup("foo").AsCharPtr("yes"));

	//define an initial content of a store which is empty but already an array
	Anything request = Anything(Anything::ArrayMarker());
	String popStoreKey;
	ctx.Push("StoreTest.request", request);

	//now the information of the store still empty
	assertEqual("right", ctx.Lookup("header.HOST", "right"));
	assertEqual("no", ctx.Lookup("header.URI", "no"));
	assertEqual("yes", ctx.Lookup("body").AsCharPtr("yes"));

	//now fill in some information
	t_assertm(!ctx.GetStore("Store.request", request), "expected GetStore() to fail because of non existent stack entry! Check changed stack behavior");
	Anything anyPushedRequest;
	anyPushedRequest["header"]["HOST"] = "sentosa";
	anyPushedRequest["header"]["URI"] = "/";
	anyPushedRequest["body"] = "test&it&now";

	ctx.Push("Store.request", anyPushedRequest);
	//now the information of the store should be found
	assertEqual("sentosa", ctx.Lookup("header.HOST", "right"));
	assertEqual("/", ctx.Lookup("header.URI", "no"));
	assertEqual("test&it&now", ctx.Lookup("body").AsCharPtr("yes"));

	t_assert(ctx.PopStore(popStoreKey));
	assertEqual("Store.request", popStoreKey);

	t_assert(ctx.PopStore(popStoreKey));
	assertEqual("StoreTest.request", popStoreKey);
	//now the information of the store should no longer be there
	assertEqual("right", ctx.Lookup("header.HOST", "right"));
	assertEqual("no", ctx.Lookup("header.URI", "no"));
	assertEqual("yes", ctx.Lookup("body").AsCharPtr("yes"));
}

void ContextTest::MoreThanOnePush()
{
	// create an empty context
	Context ctx;

	//define an initial content of a store which is empty but already an array
	Anything request = Anything(Anything::ArrayMarker());
	Anything env = Anything(Anything::ArrayMarker());
	String popStoreKey;
	ctx.Push("Store.request", request);

	//now the information of the store still empty
	assertEqual("right", ctx.Lookup("header.HOST", "right"));
	assertEqual("no", ctx.Lookup("header.URI", "no"));
	assertEqual("yes", ctx.Lookup("body").AsCharPtr("yes"));

	//now fill in some information
	ctx.GetStore("Store.request", request);
	request["header"]["HOST"] = "sentosa";
	request["header"]["URI"] = "/";
	request["body"] = "test&it&now";

	//now the information of the store should be found
	assertEqual("sentosa", ctx.Lookup("header.HOST", "right"));
	assertEqual("/", ctx.Lookup("header.URI", "no"));
	assertEqual("test&it&now", ctx.Lookup("body").AsCharPtr("yes"));

	ctx.Push("Store.env", env);
	//now the information of the store should be found (still the same)
	assertEqual("sentosa", ctx.Lookup("header.HOST", "right"));
	assertEqual("/", ctx.Lookup("header.URI", "no"));
	assertEqual("test&it&now", ctx.Lookup("body").AsCharPtr("yes"));

	env["header"]["HOST"] = "aarau";
	env["header"]["URI"] = "/finval/";
	env["body"] = "new&test&now";
	//now the information of the store should be found (but with new values)
	assertEqual("aarau", ctx.Lookup("header.HOST", "right"));
	assertEqual("/finval/", ctx.Lookup("header.URI", "no"));
	assertEqual("new&test&now", ctx.Lookup("body").AsCharPtr("yes"));

	t_assert(ctx.PopStore(popStoreKey));
	assertEqual("Store.env", popStoreKey);

	//now the information of the store should be found with old values
	assertEqual("sentosa", ctx.Lookup("header.HOST", "right"));
	assertEqual("/", ctx.Lookup("header.URI", "no"));
	assertEqual("test&it&now", ctx.Lookup("body").AsCharPtr("yes"));

	t_assert(ctx.PopStore(popStoreKey));
	assertEqual("Store.request", popStoreKey);

	//now the information of the store should no longer be there
	assertEqual("right", ctx.Lookup("header.HOST", "right"));
	assertEqual("no", ctx.Lookup("header.URI", "no"));
	assertEqual("yes", ctx.Lookup("body").AsCharPtr("yes"));
}

void ContextTest::MoreThanOnePushWithSameKey()
{
	// create an empty context
	Context ctx;

	//define an initial content of a store which is empty but already an array
	Anything request = Anything(Anything::ArrayMarker());
	Anything env;
	String popStoreKey;
	ctx.Push("Store.request", request);

	//now the information of the store still empty
	assertEqual("right", ctx.Lookup("header.HOST", "right"));
	assertEqual("no", ctx.Lookup("header.URI", "no"));
	assertEqual("yes", ctx.Lookup("body").AsCharPtr("yes"));

	//now fill in some information
	request["header"]["HOST"] = "sentosa";
	request["header"]["URI"] = "/";
	request["body"] = "test&it&now";
	request["bottomslot"] = "yuppieh";

	//now the information of the store should be found
	assertEqual("sentosa", ctx.Lookup("header.HOST", "right"));
	assertEqual("/", ctx.Lookup("header.URI", "no"));
	assertEqual("test&it&now", ctx.Lookup("body").AsCharPtr("yes"));
	assertEqual("yuppieh", ctx.Lookup("bottomslot").AsCharPtr("yes"));

	env["header"]["HOST"] = "aarau";
	env["header"]["URI"] = "/finval/";
	env["body"] = "new&test&now";
	env["topslot"] = "ole";
	ctx.Push("Store.request", env);

	//now the information of the store should be found (but with new values)
	assertEqual("aarau", ctx.Lookup("header.HOST", "right"));
	assertEqual("/finval/", ctx.Lookup("header.URI", "no"));
	assertEqual("new&test&now", ctx.Lookup("body").AsCharPtr("yes"));
	assertEqual("ole", ctx.Lookup("topslot").AsCharPtr("yes"));
	assertEqual("yuppieh", ctx.Lookup("bottomslot").AsCharPtr("yes"));

	t_assert(ctx.PopStore(popStoreKey));
	assertEqual("Store.request", popStoreKey);

	//now the information of the store should be found with old values
	assertEqual("sentosa", ctx.Lookup("header.HOST", "right"));
	assertEqual("/", ctx.Lookup("header.URI", "no"));
	assertEqual("test&it&now", ctx.Lookup("body").AsCharPtr("yes"));
	assertEqual("yuppieh", ctx.Lookup("bottomslot").AsCharPtr("yes"));
	assertEqual("yes", ctx.Lookup("topslot").AsCharPtr("yes"));

	t_assert(ctx.PopStore(popStoreKey));
	assertEqual("Store.request", popStoreKey);

	//now the information of the store should no longer be there
	assertEqual("right", ctx.Lookup("header.HOST", "right"));
	assertEqual("no", ctx.Lookup("header.URI", "no"));
	assertEqual("yes", ctx.Lookup("body").AsCharPtr("yes"));
	assertEqual("yes", ctx.Lookup("topslot").AsCharPtr("yes"));
	assertEqual("yes", ctx.Lookup("bottomslot").AsCharPtr("yes"));

	Anything newStore = "foo";
	ctx.Push("Store.request", newStore);
	assertEqual("right", ctx.Lookup("header.HOST", "right"));
	assertEqual("foo", ctx.Lookup(":0").AsCharPtr("yes"));

	// due to reference semantics, we can freely add other values
	newStore["header"]["HOST"] = "aarau";
	newStore["header"]["URI"] = "/finval/";
	newStore["body"] = "new&test&now";

	//now the information of the store should be found (but with new values)
	assertEqual("aarau", ctx.Lookup("header.HOST", "right"));
	assertEqual("/finval/", ctx.Lookup("header.URI", "no"));
	assertEqual("new&test&now", ctx.Lookup("body").AsCharPtr("yes"));
	assertEqual("foo", ctx.Lookup(":0").AsCharPtr("yes"));
}

void ContextTest::MoreThanOnePushWithSameKeyPrefix()
{
	// create an empty context
	Context ctx;

	//define an initial content of a store which is empty but already an array
	Anything request0 = Anything(Anything::ArrayMarker());
	Anything request1 = Anything(Anything::ArrayMarker());
	Anything env;
	String popStoreKey;
	ctx.Push("Store", request0);

	request0["request"] =  "bah";
	request0["bottomslot"] =  "yuppieh";
	assertEqual("bah", ctx.Lookup("request", "not found"));
	assertEqual("yuppieh", ctx.Lookup("bottomslot", "not found"));

	// pay attention, the key has no LookupPath semantics!
	ctx.Push("Store.request", request1);
	request1["request"] = "foo";
	request1["middleslot"] = "ole";
	assertEqual("foo", ctx.Lookup("request", "not found"));
	assertEqual("yuppieh", ctx.Lookup("bottomslot", "not found"));
	assertEqual("ole", ctx.Lookup("middleslot", "not found"));

	env["request"] = "bar";
	env["topslot"] = "gugus";
	ctx.Push("Store", env);
	assertEqual("bar", ctx.Lookup("request", "not found"));
	assertEqual("gugus", ctx.Lookup("topslot", "not found"));
	assertEqual("yuppieh", ctx.Lookup("bottomslot", "not found"));
	assertEqual("ole", ctx.Lookup("middleslot", "not found"));

	// show behavior of pop-store
	String strKey;
	ctx.PopStore(strKey);
	assertCharPtrEqual("Store", strKey);
	assertEqual("foo", ctx.Lookup("request", "not found"));
	assertEqual("yuppieh", ctx.Lookup("bottomslot", "not found"));
	assertEqual("ole", ctx.Lookup("middleslot", "not found"));
	assertEqual("not found", ctx.Lookup("topslot", "not found"));
}

void ContextTest::StoreTests()
{
	SimplePushPop();
	SimplePushWithEmptyStore();
	MoreThanOnePush();
	MoreThanOnePushWithSameKey();
	MoreThanOnePushWithSameKeyPrefix();
}

void ContextTest::RequestSettingTest()
{
	Anything request;
	request["header"]["HOST"] = "sentosa";
	request["header"]["URI"] = "/";
	request["body"] = "test&it&now";

	Context ctx;

	//lookups find things first in the second object than in the first object
	assertEqual("not found", ctx.Lookup("header.HOST", "not found"));
	assertEqual("not found", ctx.Lookup("header.URI").AsCharPtr("not found"));
	assertEqual("not found", ctx.Lookup("body", "not found"));

	ctx.PushRequest(request);

	//lookups find things first in the second object than in the first object
	assertEqual("sentosa", ctx.Lookup("header.HOST", "not found"));
	assertEqual("/", ctx.Lookup("header.URI").AsCharPtr("not found"));
	assertEqual("test&it&now", ctx.Lookup("body", "not found"));

	assertAnyEqual(request, ctx.GetRequest());
}

void ContextTest::SessionPushTest()
{
	// The brackets are used to achieve the following:
	// On destruction of the context, the context decrements the refcount of
	// the session it has on its "stack". Therefore, the context must go out of
	// scope before the session does. With this setup there are no complaints on
	// session destruction that the reference count doesn't equal 0.
	{
		Context sessionsCtx;
		Session s(0);
		{
			Anything sessionStore;
			Anything emptyStore = Anything(Anything::ArrayMarker());
			Context ctx;

			// session store is always there even without session
			// but it will not survive the ctx
			t_assert(ctx.GetStore("Session", sessionStore));
			assertAnyEqualm(emptyStore, sessionStore, "expected an empty metathing");

			assertEqual("none", ctx.Lookup("testKey", "none"));
			assertEqual("not there", ctx.Lookup("test.one", "not there"));

			// fill in some values
			sessionStore["testKey"] = "one";
			sessionStore["test"]["one"] = "there";

			// values are now visible but in non session based store
			assertEqual("one", ctx.Lookup("testKey", "none"));
			assertEqual("there", ctx.Lookup("test.one", "not there"));

			// push session -> store will stack down existing session store
			ctx.Push(&s);

			t_assert(ctx.GetStore("Session", sessionStore));
			assertAnyEqualm(s.GetStoreGlobal(), sessionStore, "expected an empty metathing again");

			// store is now empty -> values are no longer visible
			assertEqual("none", ctx.Lookup("testKey", "none"));
			assertEqual("not there", ctx.Lookup("test.one", "not there"));

			// fill in some values
			sessionStore["testKey"] = "two";
			sessionStore["test"]["one"] = "there two";

			// values are now visible and in session based store
			assertEqual("two", ctx.Lookup("testKey", "none"));
			assertEqual("there two", ctx.Lookup("test.one", "not there"));
		}
	}
	{
		Context sessionsCtx;
		Session s(0);
		{
			// We push the session and manipulate the session afterwards.
			// This should not be reflected in the pushed session, because
			// in this test the push of the session is done with the makeCopy
			// flag set.

			Anything sessionStore;
			Anything emptyStore = Anything(Anything::ArrayMarker());
			Context ctx;
			Anything env;
			env["Context"]["CopySessionStore"] = true;
			ctx.Push("test", env);

			// session store is always there even without session
			// but it will not survive the ctx
			t_assert(ctx.GetStore("Session", sessionStore));
			assertAnyEqualm(emptyStore, sessionStore, "expected an empty metathing");

			assertEqual("none", ctx.Lookup("testKey", "none"));
			assertEqual("not there", ctx.Lookup("test.two", "not there"));

			// fill in some values
			sessionStore["testKey"] = "two";
			sessionStore["test"]["two"] = "there";

			// values are now visible but in non session based store
			assertEqual("two", ctx.Lookup("testKey", "none"));
			assertEqual("there", ctx.Lookup("test.two", "not there"));

			// push session -> store will stack down existing session store

			// fill in some values
			s.fStore["testKey"] = "shouldwork";
			s.fStore["test"]["two"] = "here either";
			ctx.Push(&s); // make a copy

			assertEqual("shouldwork", ctx.Lookup("testKey", "none"));
			assertEqual("here either", ctx.Lookup("test.two", "not there"));

			// fill in other values
			s.fStore["testKey"] = "shouldnotwork";
			s.fStore["test"]["two"] = "here neither";

			assertEqual("shouldwork", ctx.Lookup("testKey", "none"));
			assertEqual("here either", ctx.Lookup("test.two", "not there"));
		}
	}
	{
		Context sessionsCtx;
		Session s(0);
		{
			// We push the session and set the unlock session flag.
			// Session should be unlocked from then on.

			Anything sessionStore;
			Anything emptyStore = Anything(Anything::ArrayMarker());
			Context ctx;
			Anything env;
			env["Context"]["CopySessionStore"] = true;
			ctx.Push("test", env);

			t_assert((s.GetRefCount()) == 0);
			ctx.Push(&s); // Push it, should be unlocked then
			t_assert((s.GetRefCount()) == 1);

			Session *mySession = ctx.GetSession();
			t_assert(&s == mySession);
			// Verify if unlocked
			t_assert( mySession->fMutex.GetCount() == 0);
		}
	}
	{
		Context sessionsCtx;
		Session s(0);
		{
			// We push the session and set the lock (default) session flag.
			// Session should be locked from then on.

			Anything sessionStore;
			Anything emptyStore = Anything(Anything::ArrayMarker());
			Context ctx;
			Anything env;

			ctx.Push("test", env);

			t_assert((s.GetRefCount()) == 0);
			ctx.Push(&s); // Push it, should be locked then
			t_assert((s.GetRefCount()) == 1);

			Session *mySession = ctx.GetSession();
			t_assert(&s == mySession);
			// Verify if locked
			assertEqual(1, mySession->fMutex.GetCount());
		}
	}
}

void ContextTest::RefCountTest()
{
	// The brackets are used to achieve the following:
	// On destruction of the context, the context decrements the refcount of
	// the session it has on its "stack". Therefore, the context must go out of
	// scope before the session does. With this setup there are no complaints on
	// session destruction that the reference count doesn't equal 0.
	Context ctx;
	Session s1("hank1");
	Session s2("hank2");
	s1.Init("1", ctx);
	s2.Init("2", ctx);
	{
		Context ctx1;
		Context ctx2;
		Context ctx3;

		// push session -> store will stack down existing session store
		// session count should be 0
		t_assert((s1.GetRefCount()) == 0);
		ctx1.Push(&s1); // make a copy

		// session count should be 1
		Session *mySession = ctx1.GetSession();
		t_assert((mySession->GetRefCount()) == 1);
		t_assert((s1.GetRefCount()) == 1);

		// Push our session onto a new context, now RefCount should be 2
		ctx2.Push(&s1); // make a copy

		// session count should be 1
		mySession = ctx2.GetSession();
		t_assert((mySession->GetRefCount()) == 2);

		// Push our session onto a new context, now RefCount should be 3
		ctx3.Push(&s1); // make a copy

		// session count should be 1
		mySession = ctx3.GetSession();
		t_assert((mySession->GetRefCount()) == 3);

		// Now we push a new session onto our context, should not alter old
		// sessions refcount

		// session count should be 0
		t_assert((s2.GetRefCount()) == 0);
		ctx1.Push(&s2); // make a copy

		// session count should be 1
		mySession = ctx1.GetSession();
		t_assert((mySession->GetRefCount()) == 1);
		t_assert((s2.GetRefCount()) == 1);

		ctx2.Push(&s2); // make a copy
		// session count should be 1
		mySession = ctx2.GetSession();
		t_assert((mySession->GetRefCount()) == 2);
		t_assert((s2.GetRefCount()) == 2);

		ctx3.Push(&s2);
		mySession = ctx3.GetSession();
		t_assert((mySession->GetRefCount()) == 3);

		// Pushing the same session onto the same context
		// has no effect

		ctx2.Push(&s2); // make a copy
		// session count should be 1
		mySession = ctx2.GetSession();
		t_assert((mySession->GetRefCount()) == 3);
		t_assert((s2.GetRefCount()) == 3);
	}
}

void ContextTest::RoleStoreTest()
{
	Context ctx1;
	Session s(0);
	{
		Context ctx;
		Anything roleStore;

		ctx.Push(&s);

		t_assert(ctx.GetStore("Role", roleStore));
		assertAnyEqualm(s.GetRoleStoreGlobal(), roleStore, "expected an empty metathing again");

		// store is now empty -> values are no longer visible
		assertEqual("none", ctx.Lookup("testKey", "none"));
		assertEqual("not there", ctx.Lookup("test.one", "not there"));

		// fill in some values
		roleStore["testKey"] = "two";
		roleStore["test"]["one"] = "there two";

		// values are now visible and in session based store
		assertEqual("two", ctx.Lookup("testKey", "none"));
		assertEqual("there two", ctx.Lookup("test.one", "not there"));
		assertAnyEqualm(s.GetRoleStoreGlobal(), ctx.GetRoleStore(), "expected beeing the same anything");
	}
	{
		Context ctx;
		Anything roleStore;

		ctx.Push(&s);

		t_assert(ctx.GetStore("Role", roleStore));
		assertAnyEqualm(s.GetRoleStoreGlobal(), roleStore, "expected an empty metathing again");

		// values are still there
		assertEqual("two", ctx.Lookup("testKey", "none"));
		assertEqual("there two", ctx.Lookup("test.one", "not there"));

		// change some values
		roleStore["testKey"] = "three";
		roleStore["test"]["one"] = "there tree";

		// values are now visible and in session based store
		assertEqual("three", ctx.Lookup("testKey", "none"));
		assertEqual("there tree", ctx.Lookup("test.one", "not there"));
		assertAnyEqualm(s.GetRoleStoreGlobal(), ctx.GetRoleStore(), "expected beeing the same anything");
	}
}

void ContextTest::SessionStoreTest()
{
	Context ctx1;
	Session s(0);
	{
		Context ctx;
		Anything sessionStore;

		ctx.Push(&s);

		t_assert(ctx.GetStore("Session", sessionStore));
		assertAnyEqualm(s.GetStoreGlobal(), sessionStore, "expected an empty metathing again");

		// store is now empty -> values are no longer visible
		assertEqual("none", ctx.Lookup("testKey", "none"));
		assertEqual("not there", ctx.Lookup("test.one", "not there"));

		// fill in some values
		sessionStore["testKey"] = "two";
		sessionStore["test"]["one"] = "there two";

		// values are now visible and in session based store
		assertEqual("two", ctx.Lookup("testKey", "none"));
		assertEqual("there two", ctx.Lookup("test.one", "not there"));
	}
	{
		Context ctx;
		Anything sessionStore;

		ctx.Push(&s);

		t_assert(ctx.GetStore("Session", sessionStore));
		assertAnyEqualm(s.GetStoreGlobal(), sessionStore, "expected an empty metathing again");

		// values are still there
		assertEqual("two", ctx.Lookup("testKey", "none"));
		assertEqual("there two", ctx.Lookup("test.one", "not there"));

		// change some values
		sessionStore["testKey"] = "three";
		sessionStore["test"]["one"] = "there tree";

		// values are now visible and in session based store
		assertEqual("three", ctx.Lookup("testKey", "none"));
		assertEqual("there tree", ctx.Lookup("test.one", "not there"));
	}
}

void ContextTest::ObjectAccessTests()
{
	Context ctx;
	assertEqual(0, ctx.fStackSz);

	//push a lookup interface
	ctx.Push("Server", Server::FindServer("Server"));
	assertEqual(1, ctx.fStackSz);

	//push a lookup interface
	ctx.Push("Role", Role::FindRole("Role"));
	assertEqual(2, ctx.fStackSz);

	//push a second interface
	ctx.Push("Page", Page::FindPage("Page"));
	assertEqual(3, ctx.fStackSz);

	// object not in context
	LookupInterface *nothere = ctx.Find("NoObject");
	t_assert(!nothere);

	// object not in context
	LookupInterface *noName = ctx.Find(0);
	t_assert(!noName);

	// use with dynamic_cast to be on the safe side
	Role *r = SafeCast(ctx.Find("Role"), Role);
	t_assert(r != 0);
	if ( r ) {
		String transition("Home");
		String page("BannerPage");
		r->GetNewPageName(ctx, transition, page);
		assertEqual("GuestPage", page);
	}

	// use with dynamic_cast to be on the safe side
	Page *p = SafeCast(ctx.Find("Page"), Page);
	t_assert(p != 0);
	if (p) {
		String transition("Home");
		String page("BannerPage");
		// implicit push
		t_assert(p->Finish(transition, ctx));
	}

	assertEqual(3, ctx.fStackSz);
	String strKey;
	t_assertm(ctx.Pop(strKey), "Pops Page object");
	assertEqual(2, ctx.fStackSz);
	t_assertm(ctx.Pop(strKey), "Pops Role object");
	assertEqual(1, ctx.fStackSz);
	t_assertm(ctx.Pop(strKey), "Pops Sorver object");
	assertEqual(0, ctx.fStackSz);
	t_assertm(ctx.Pop(strKey) == false, "Empty stack");
}

void ContextTest::SetNGetPage()
{
	StartTrace(ContextTest.SetNGetPage);

	Context ctx;
	assertEqual(0, ctx.fStackSz);

	t_assert(!ctx.GetPage());

	Page *p = Page::FindPage("Page");

	ctx.SetPage(p);
	assertEqual(1, ctx.fStackSz);
	assertEqual("Page", ctx.Lookup("ContextItem", "not found"));

	assertEqual((long)p, (long)ctx.GetPage());

	p = Page::FindPage("SecondPage");

	ctx.SetPage(p);
	assertEqual(1, ctx.fStackSz);
	assertEqual("SecondPage", ctx.Lookup("ContextItem", "not found"));

	assertEqual((long)p, (long)ctx.GetPage());
}

void ContextTest::SetNGetRole()
{
	StartTrace(ContextTest.SetNGetRole);

	Context ctx;
	assertEqual(0, ctx.fStackSz);

	t_assert(!ctx.GetRole());

	Role *p = Role::FindRole("Role");

	ctx.SetRole(p);
	assertEqual(1, ctx.fStackSz);
	assertEqual("Role", ctx.Lookup("ContextItem", "not found"));

	assertEqual((long)p, (long)ctx.GetRole());

	p = Role::FindRole("Guest");

	ctx.SetRole(p);
	assertEqual(1, ctx.fStackSz);
	assertEqual("SecretKey", ctx.Lookup("InGuestOnly", "not found"));

	assertEqual((long)p, (long)ctx.GetRole());
}

class TestSessionLockRenderer : public Renderer
{
public:
	TestSessionLockRenderer(const char *name): Renderer(name) {}
	//!destructor does nothing
	~TestSessionLockRenderer() {}

	//!rendering hook; overwrite this method in subclasses
	//!generates output on reply driven by config using the context given
	//! \param reply stream to generate output on
	//! \param c Context to be used for output generation
	//! \param config configuration which drives the output generation
	virtual void RenderAll(std::ostream &reply, Context &c, const ROAnything &config) {
		if (c.GetSession() && c.GetSession()->IsLockedByMe()) {
			reply << "Session is LOCKED";
		} else {
			reply << "Session unlocked";
		}
	}
};

RegisterRenderer(TestSessionLockRenderer);

void ContextTest::SessionUnlockingTest()
{
	StartTrace(ContextTest.SessionUnlockingTest);
	Context initsessioncontext;
	Session s("SessionUnlockingTest");
	t_assert(!s.IsLockedByMe());
	{
		Context c;
		c.GetTmpStore()["Context"]["CopySessionStore"] = true;
		t_assert(!s.IsLockedByMe());
		s.fMutex.Lock();
		t_assert(s.IsLockedByMe());
		Page p("SessionUnlockingTest");
		c.GetTmpStore()["PageLayout"]["TestSessionLockRenderer"] = Anything();
		OStringStream reply;
		p.Render(reply, c);
		assertEqual("Content-type: text/html\r\n\r\nSession unlocked", reply.str());
		t_assert(s.IsLockedByMe());
		s.fMutex.Unlock();
		t_assert(!s.IsLockedByMe());
		OStringStream reply2;
		p.Render(reply2, c);
		assertEqual("Content-type: text/html\r\n\r\nSession unlocked", reply2.str());
		t_assert(!s.IsLockedByMe());
		{
			Context c2;
			c2.GetTmpStore()["Context"]["CopySessionStore"] = false;
			c2.GetTmpStore()["PageLayout"]["TestSessionLockRenderer"] = Anything();
			c2.Push(&s);
			t_assert(s.IsLockedByMe());
			OStringStream reply3;
			p.Render(reply3, c2);
			assertEqual("Content-type: text/html\r\n\r\nSession is LOCKED", reply3.str());
			t_assert(s.IsLockedByMe());
		}
		t_assert(!s.IsLockedByMe());

	}
}

Test *ContextTest::suite ()
{
	TestSuite *testSuite = new TestSuite;
	ADD_CASE(testSuite, ContextTest, VerySimplePush);
	ADD_CASE(testSuite, ContextTest, RequestConstructorTest);
	ADD_CASE(testSuite, ContextTest, EmptyConstructorTest);
	ADD_CASE(testSuite, ContextTest, SocketCtorTests);
	ADD_CASE(testSuite, ContextTest, LookupTests);
	ADD_CASE(testSuite, ContextTest, ObjectAccessTests);
	ADD_CASE(testSuite, ContextTest, RequestSettingTest);
	ADD_CASE(testSuite, ContextTest, StoreTests);
	ADD_CASE(testSuite, ContextTest, SessionPushTest);
	ADD_CASE(testSuite, ContextTest, SessionStoreTest);
	ADD_CASE(testSuite, ContextTest, RoleStoreTest);
	ADD_CASE(testSuite, ContextTest, FindReplace);
	ADD_CASE(testSuite, ContextTest, RemoveTest);
	ADD_CASE(testSuite, ContextTest, SetNGetPage);
	ADD_CASE(testSuite, ContextTest, SetNGetRole);
	ADD_CASE(testSuite, ContextTest, RefCountTest);
	ADD_CASE(testSuite, ContextTest, SessionUnlockingTest);
	return testSuite;
}
