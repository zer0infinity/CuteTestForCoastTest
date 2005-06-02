/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

//--- interface include --------------------------------------------------------
#include "ParameterMapperTest.h"

//--- test modules used --------------------------------------------------------
#include "TestSuite.h"

//--- module under test --------------------------------------------------------
#include "Mapper.h"

//--- standard modules used ----------------------------------------------------
#include "Dbg.h"
#include "StringStream.h"
#include "CacheHandler.h"

//--- c-modules used -----------------------------------------------------------

//---- ParameterMapperTest ----------------------------------------------------------------
ParameterMapperTest::ParameterMapperTest(TString tstrName) : TestCase(tstrName)
{
	StartTrace(ParameterMapperTest.Ctor);
}

ParameterMapperTest::~ParameterMapperTest()
{
	StartTrace(ParameterMapperTest.Dtor);
}

// setup for this TestCase
void ParameterMapperTest::setUp ()
{
	StartTrace(ParameterMapperTest.setUp);
}

void ParameterMapperTest::tearDown ()
{
	StartTrace(ParameterMapperTest.tearDown);
}

void ParameterMapperTest::testDoSelectScript()
{
	StartTrace(ParameterMapperTest.testDoSelectScript);

	Anything script, empty;
	script["KeyA"] = "a";
	ParameterMapper pm("");
	Context ctx;
	assertAnyEqual(pm.DoSelectScript("KeyA", script, ctx), script["KeyA"]);
	assertAnyEqual(pm.DoSelectScript("KeyNonex", script, ctx), empty);
}

void ParameterMapperTest::testDoLoadConfig()
{
	StartTrace(ParameterMapperTest.testDoLoadConfig);

	ParameterMapper pm("ParameterMapperTest");

	t_assertm(pm.DoLoadConfig("ParameterMapper"), "Couldn't load config");
	t_assertm(!pm.DoLoadConfig("NonExistingMapper"), "Found a config, but shouldn't!");
}

void ParameterMapperTest::testDoGetConfigName()
{
	StartTrace(ParameterMapperTest.testDoGetConfigName);

	String configName;
	ParameterMapper pm("");

	pm.DoGetConfigName("ParameterMapper", "", configName);
	assertEqual("InputMapperMeta", configName);
	pm.DoGetConfigName("SomeOtherMapper", "", configName);
	assertEqual("SomeOtherMapperMeta", configName);
}

void ParameterMapperTest::testPlaceIntoAnyOrAppendIfNotEmpty()
{
	StartTrace(ParameterMapperTest.testPlaceIntoAnyOrAppendIfNotEmpty);

	Anything val = "aValue", val2 = "bValue", empty;
	t_assert(empty.GetSize() == 0);

	ParameterMapper::PlaceIntoAnyOrAppendIfNotEmpty(empty, val);
	t_assert(empty.GetSize() == 1);
	t_assert(empty.Contains("aValue"));

	ParameterMapper::PlaceIntoAnyOrAppendIfNotEmpty(empty, val2);
	t_assert(empty.GetSize() == 2);
	t_assert(empty.Contains("bValue"));
}

void ParameterMapperTest::testDoFinalGetAny()
{
	StartTrace(ParameterMapperTest.testDoFinalGetAny);

	ParameterMapper pm("");
	Anything res, store;
	store["aKey"] = "a";
	Context ctx;
	Context::PushPopEntry aEntry(ctx, "test", store);

	t_assert(pm.DoFinalGetAny("aKey", res, ctx));
	t_assert(res.AsString().IsEqual("a"));
	t_assert(!pm.DoFinalGetAny("unknownKey", res, ctx));
}

void ParameterMapperTest::testDoFinalGetStream()
{
	StartTrace(ParameterMapperTest.testDoFinalGetStream);

	ParameterMapper pm("");
	Anything store, res;
	store["aKey"] = "a";
	Context ctx;
	Context::PushPopEntry aEntry(ctx, "test", store);
	OStringStream os;

	// Default implementation gets value of key from context
	t_assert(pm.DoFinalGetStream("aKey", os, ctx));
	t_assert(os.str().IsEqual("a"));
	t_assert(!pm.DoFinalGetStream("unknownKey", os, ctx));
}

void ParameterMapperTest::testDoGetAny()
{
	StartTrace(ParameterMapperTest.testDoGetAny);

	SimpleAnyLoader sal;
	Anything scripts = sal.Load("MapperTestScripts");
	ROAnything script;
	Anything store, res, exp;
	store["aKey"] = "a";
	Context ctx;
	Context::PushPopEntry aEntry(ctx, "test", store);
	ParameterMapper pm("");

	// --- 1. Mapper without script (i.e. script == empty)

	// 1.1. Known key in context
	res = Anything();
	pm.DoGetAny("aKey", res, ctx, script);
	t_assert(res.GetSize() == 1);
	t_assert(res.Contains("a"));

	// 1.2. Unknown key in context
	res = Anything();
	pm.DoGetAny("unknownKey", res, ctx, script);
	t_assert(res.GetSize() == 0);
	t_assert(res.IsNull());

	// --- 2. Mapper with simple valued script (key/context do not matter)

	// 2.1. Value is empty before call -> insert
	res = Anything();
	script = pm.DoSelectScript("SingleScript1", scripts, ctx);
	pm.DoGetAny("-", res, ctx, script);
	t_assert(res.GetSize() == 1);
	t_assert(res.Contains("ding"));

	// 2.2. Value is not empty before call -> collect (append)
	script = pm.DoSelectScript("SingleScript2", scripts, ctx);
	pm.DoGetAny("-", res, ctx, script);
	t_assert(res.GetSize() == 2);
	t_assert(res.Contains("dong"));

	// --- 3. Mapper with a full script (interpreatation)

	// 3.1 Collector Script
	res = Anything();
	script = pm.DoSelectScript("CollectorScript", scripts, ctx);
	pm.DoGetAny("-", res, ctx, script);
	IStringStream is("{ a ValueToGet x y z }");
	is >> exp;
	assertAnyEqual(exp, res);

	// 3.2 Delegation Script
	OStringStream os;
	script = pm.DoSelectScript("DelegationScript", scripts, ctx);
	pm.DoGetStream("-", os, ctx, script);
	assertEqualm("AB", os.str(), "Result not as expected. Was RendererMapper called?");

	// 3.3 Dysfunctional Script (stop at first slot)
	res = Anything();
	script = pm.DoSelectScript("DysfunctionalScript", scripts, ctx);
	pm.DoGetAny("-", res, ctx, script);
	t_assert(res.IsNull());
}

void ParameterMapperTest::testDoGetStream()
{
	StartTrace(ParameterMapperTest.testDoFinalGetStream);

	// Implementation forwards to DoGetAny(), so we just do a sanity-check

	SimpleAnyLoader sal;
	Anything scripts = sal.Load("MapperTestScripts");
	Anything store, res;
	store["aKey"] = "a";
	Context ctx;
	Context::PushPopEntry aEntry(ctx, "test", store);
	ParameterMapper pm("");
	OStringStream os;

	ROAnything script = pm.DoSelectScript("CollectorScript", scripts, ctx);
	pm.DoGetStream("-", os, ctx, script);
	assertEqual(os.str(), "aValueToGetxyz");
}

void ParameterMapperTest::testGet()
{
	StartTrace(ParameterMapperTest.testGet);

	Anything store;
	store["sKey"] = "ChristmasIsComing";
	store["bKey"] = true;
	store["lKey"] = 1234L;
	store["dKey"] = 3.1415;
	store["aKey"]["foo"] = "bar";
	store["aKey"]["hello"] = "world";
	Context ctx;
	Context::PushPopEntry aEntry(ctx, "test", store);
	ParameterMapper pm("ParameterMapperTest");
	pm.CheckConfig("ParameterMapper");
	String s;
	bool b;
	long l;
	double d;
	Anything a, empty;
	OStringStream os;

	// get simple values
	pm.Get("string", s, ctx);
	assertEqual(s, store["sKey"].AsString());

	pm.Get("bool", b, ctx);
	assertEqual(b, store["bKey"].AsBool());

	pm.Get("long", l, ctx);
	assertEqual(l, store["lKey"].AsLong());

	pm.Get("double", d, ctx);
	assertDoublesEqual(d, store["dKey"].AsDouble(), 1);

	// get any
	pm.Get("any", a, ctx);
	assertAnyEqual(a, store["aKey"]);

	// get stream
	pm.Get("any", os, ctx);
	assertEqual("{\n  /foo "_QUOTE_("bar")"\n  /hello "_QUOTE_("world")"\n}", os.str());

	// test with non-existing key (unknown in scripts)
	pm.Get("nonexKey", empty, ctx);
	t_assert(empty.IsNull());
}

void ParameterMapperTest::testEagerDoSelectScript()
{
	StartTrace(ParameterMapperTest.testEagerDoSelectScript);

	Anything script;
	script["KeyA"] = "a";
	EagerParameterMapper epm("");
	Context ctx;
	assertAnyEqual(epm.DoSelectScript("KeyA", script, ctx), script["KeyA"]);
	assertAnyEqual(epm.DoSelectScript("KeyNonex", script, ctx), script);
}

void ParameterMapperTest::testEagerGet()
{
	StartTrace(ParameterMapperTest.testEagerGet);

	EagerParameterMapper epm("EagerParameterMapperTest");
	epm.CheckConfig("ParameterMapper");
	Anything store;
	store["someKey"] = "!";
	store["anotherKey"] = "?";
	Context ctx;
	Context::PushPopEntry aEntry(ctx, "test", store);
	OStringStream os;
	String str;

	// script is also loaded and executed, if key is not found in it (i.e. in any case)
	epm.Get("nonexKey", os, ctx);
	assertEqual("curiositykillsthecat!really?", os.str());

	// however, if key exists, sub-script will be executed
	epm.Get("PlainValueOnly", str, ctx);
	assertEqual("really", str);
}

void ParameterMapperTest::testDoSetSourceSlotDynamically()
{
	StartTrace(ParameterMapperTest.testDoSetSourceSlotDynamically);

	Context ctx;
	ParameterMapper pm("DynamicFetcher");

	// without config (no path returned)
	assertEquals("", pm.DoGetSourceSlot(ctx));

	// set in tmp-store
	ctx.GetTmpStore()["DynamicFetcher"]["SourceSlot"] = "Mapper.a.b.c";
	assertEquals("Mapper.a.b.c", pm.DoGetSourceSlot(ctx));

	// with config (overrides dest in tmp store)
	pm.CheckConfig("ParameterMapper");
	assertEquals("Mapper.foo.bar", pm.DoGetSourceSlot(ctx));
}

void ParameterMapperTest::testDoGetSourceSlotWithPath()
{
	StartTrace(ParameterMapperTest.testDoGetSourceSlotWithPath);

	Context ctx;
	ParameterMapper pm("PMTest");

	// regular get
	ctx.GetTmpStore()["msg"] = "foo";
	Anything res;
	pm.Get("msg", res, ctx);
	assertEquals("foo", res.AsString());

	// get dynamically
	ctx.GetTmpStore()["PMTest"]["SourceSlot"] = "nested";
	ctx.GetTmpStore()["nested"]["msg"] = "bar";
	pm.Get("msg", res, ctx);
	assertEquals("bar", res.AsString());
}

// builds up a suite of testcases, add a line for each testmethod
Test *ParameterMapperTest::suite ()
{
	StartTrace(ParameterMapperTest.suite);
	TestSuite *testSuite = new TestSuite;

	ADD_CASE(testSuite, ParameterMapperTest, testDoSelectScript);
	ADD_CASE(testSuite, ParameterMapperTest, testDoLoadConfig);
	ADD_CASE(testSuite, ParameterMapperTest, testDoGetConfigName);
	ADD_CASE(testSuite, ParameterMapperTest, testPlaceIntoAnyOrAppendIfNotEmpty);
	ADD_CASE(testSuite, ParameterMapperTest, testDoFinalGetAny);
	ADD_CASE(testSuite, ParameterMapperTest, testDoFinalGetStream);
	ADD_CASE(testSuite, ParameterMapperTest, testDoGetAny);
	ADD_CASE(testSuite, ParameterMapperTest, testDoGetStream);
	ADD_CASE(testSuite, ParameterMapperTest, testGet);
	ADD_CASE(testSuite, ParameterMapperTest, testDoSetSourceSlotDynamically);
	ADD_CASE(testSuite, ParameterMapperTest, testDoGetSourceSlotWithPath);

	ADD_CASE(testSuite, ParameterMapperTest, testEagerDoSelectScript);
	ADD_CASE(testSuite, ParameterMapperTest, testEagerGet);

	return testSuite;
}
