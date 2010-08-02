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

//---- ParameterMapperTest ----------------------------------------------------------------
ParameterMapperTest::ParameterMapperTest(TString tstrName) : TestCaseType(tstrName)
{
	StartTrace(ParameterMapperTest.Ctor);
}

ParameterMapperTest::~ParameterMapperTest()
{
	StartTrace(ParameterMapperTest.Dtor);
}

void ParameterMapperTest::DoSelectScriptTest()
{
	StartTrace(ParameterMapperTest.DoSelectScriptTest);

	Anything script, empty;
	script["KeyA"] = "a";
	ParameterMapper pm("");
	Context ctx;
	assertAnyEqual(pm.SelectScript("KeyA", script, ctx), script["KeyA"]);
	assertAnyEqual(pm.SelectScript("KeyNonex", script, ctx), empty);
}

void ParameterMapperTest::DoLoadConfigTest()
{
	StartTrace(ParameterMapperTest.DoLoadConfigTest);

	ParameterMapper pm("ParameterMapperTest");
	t_assertm(pm.Initialize("ParameterMapper"), "Couldn't load config");

	ParameterMapper fm("ParameterMapperFail");
	if ( t_assertm(fm.Initialize("NonExistingMapper"), "initialization should succeed") ) {
		t_assertm(fm.GetConfig().IsNull(), "Found a config, but shouldn't!");
	}
}

void ParameterMapperTest::DoGetConfigNameTest()
{
	StartTrace(ParameterMapperTest.DoGetConfigNameTest);
	ParameterMapper pm("GetConfigNameTest");
	pm.Initialize(ParameterMapper::gpcCategory);
	assertCharPtrEqual("InputMapperMeta", pm.GetConfigName());
}

void ParameterMapperTest::PlaceIntoAnyOrAppendIfNotEmptyTest()
{
	StartTrace(ParameterMapperTest.PlaceIntoAnyOrAppendIfNotEmptyTest);

	Anything val = "aValue", val2 = "bValue", empty;
	t_assert(empty.GetSize() == 0);

	ParameterMapper::PlaceIntoAnyOrAppendIfNotEmpty(empty, val);
	t_assert(empty.GetSize() == 1);
	t_assert(empty.Contains("aValue"));

	ParameterMapper::PlaceIntoAnyOrAppendIfNotEmpty(empty, val2);
	t_assert(empty.GetSize() == 2);
	t_assert(empty.Contains("bValue"));
}

void ParameterMapperTest::DoFinalGetAnyTest()
{
	StartTrace(ParameterMapperTest.DoFinalGetAnyTest);

	ParameterMapper pm("");
	pm.Initialize(ParameterMapper::gpcCategory);
	Anything res, store;
	store["aKey"] = "a";
	Context ctx;
	Context::PushPopEntry<Anything> aEntry(ctx, "test", store);

	t_assert(pm.Get("aKey", res, ctx));
	t_assert(res.AsString().IsEqual("a"));
	t_assert(!pm.Get("unknownKey", res, ctx));
}

void ParameterMapperTest::DoFinalGetStreamTest()
{
	StartTrace(ParameterMapperTest.DoFinalGetStreamTest);

	ParameterMapper pm("");
	pm.Initialize(ParameterMapper::gpcCategory);
	Anything store, res;
	store["aKey"] = "a";
	Context ctx;
	Context::PushPopEntry<Anything> aEntry(ctx, "test", store);
	OStringStream os;

	// Default implementation gets value of key from context
	t_assert(pm.Get("aKey", os, ctx));
	t_assert(os.str().IsEqual("a"));
	t_assert(!pm.Get("unknownKey", os, ctx));
}

void ParameterMapperTest::DoGetAnyTest()
{
	StartTrace(ParameterMapperTest.DoGetAnyTest);
	Anything store;
	store["aKey"] = "a";
	Context ctx;
	Context::PushPopEntry<Anything> aEntry(ctx, "test", store);
	// --- 1. Mapper without script (i.e. script == empty)
	{
		ParameterMapper pm("MyPlainMapperWithoutConfig");
		pm.Initialize(ParameterMapper::gpcCategory);
		Anything res;
		// 1.1. Known key in context
		t_assertm(pm.Get("aKey", res, ctx), "key should be found in context");
		t_assert(res.GetSize() == 1);
		t_assert(res.Contains("a"));

		res = Anything();
		t_assertm(!pm.Get("unknownKey", res, ctx), "key should not be found");
		t_assert(res.GetSize() == 0);
		t_assert(res.IsNull());
	}
	// --- 2. Mapper with simple valued (key matters)
	{
		ParameterMapper pm("SingleScript1");
		pm.Initialize(ParameterMapper::gpcCategory);
		Anything res;
		t_assertm(!pm.Get("-", res, ctx), "should fail because key is not found in mapper config");
		t_assert(res.GetSize() == 0);
	}
	// 2.2. Value is not empty before call -> collect (append)
	{
		ParameterMapper pm("SingleScript2");
		pm.Initialize(ParameterMapper::gpcCategory);
		Anything res;
		t_assertm(pm.Get("-", res, ctx), "key should be found within mapper config");
		assertCompare(1L, equal_to, res.GetSize());
		t_assert(res.Contains("dong"));
	}
	// --- 3. Mapper with a full script (interpreatation)
	// 3.1 Collector Script
	{
		ParameterMapper pm("CollectorScript");
		pm.Initialize(ParameterMapper::gpcCategory);
		Anything res, exp;
		t_assertm(pm.Get("mapkey", res, ctx), "key should be found in mapper config");
		String strIn("{ a ValueToGet x y z }");
		IStringStream is(strIn);
		is >> exp;
		assertAnyEqualm(exp, res, "content should equal because stream-Get is appending content when not a mapper");
	}
	// 3.2 Delegation Script
	{
		ParameterMapper pm("DelegationScript");
		pm.Initialize(ParameterMapper::gpcCategory);
		Anything res;
		OStringStream os;
		t_assertm(pm.Get("mapkey", os, ctx), "key should be found in mapper config");
		assertEqualm("AB", os.str(), "Result not as expected. Was RendererMapper called?");
	}
	// 3.3 Dysfunctional Script (stop at first slot)
	{
		ParameterMapper pm("DysfunctionalScript");
		pm.Initialize(ParameterMapper::gpcCategory);
		Anything res;
		t_assertm(!pm.Get("mapkey", res, ctx), "should fail because not everything in mapper config is a mapper and therefor leads to an error in mapping");
		t_assert(res.IsNull());
	}
}

void ParameterMapperTest::DoGetStreamTest()
{
	StartTrace(ParameterMapperTest.DoFinalGetStreamTest);
	Anything store, res;
	store["aKey"] = "a";
	Context ctx;
	Context::PushPopEntry<Anything> aEntry(ctx, "test", store);
	OStringStream os;
	ParameterMapper pm("CollectorScript");
	pm.Initialize(ParameterMapper::gpcCategory);
	pm.Get("mapkey", os, ctx);
	assertEqual(os.str(), "aValueToGetxyz");
}

void ParameterMapperTest::GetTest()
{
	StartTrace(ParameterMapperTest.GetTest);

	Anything store;
	store["sKey"] = "ChristmasIsComing";
	store["bKey"] = true;
	store["lKey"] = 1234L;
	store["dKey"] = 3.1415;
	store["aKey"]["foo"] = "bar";
	store["aKey"]["hello"] = "world";
	Context ctx;
	Context::PushPopEntry<Anything> aEntry(ctx, "test", store);
	ParameterMapper pm("ParameterMapperTest");
	pm.Initialize("ParameterMapper");
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

void ParameterMapperTest::EagerDoSelectScriptTest()
{
	StartTrace(ParameterMapperTest.EagerDoSelectScriptTest);

	Anything script;
	script["KeyA"] = "a";
	EagerParameterMapper epm("");
	Context ctx;
	assertAnyEqual(epm.SelectScript("KeyA", script, ctx), script["KeyA"]);
	assertAnyEqual(epm.SelectScript("KeyNonex", script, ctx), script);
}

void ParameterMapperTest::EagerGetTest()
{
	StartTrace(ParameterMapperTest.EagerGetTest);

	EagerParameterMapper epm("EagerParameterMapperTest");
	epm.Initialize("ParameterMapper");
	Anything store;
	store["someKey"] = "!";
	store["anotherKey"] = "?";
	Context ctx;
	Context::PushPopEntry<Anything> aEntry(ctx, "test", store);
	OStringStream os;
	String str;

	// script is also loaded and executed, if key is not found in it (i.e. in any case)
	epm.Get("nonexKey", os, ctx);
	assertEqual("curiositykillsthecat!really?", os.str());

	// however, if key exists, sub-script will be executed
	epm.Get("PlainValueOnly", str, ctx);
	assertEqual("really", str);
}

void ParameterMapperTest::DoSetSourceSlotDynamicallyTest()
{
	StartTrace(ParameterMapperTest.DoSetSourceSlotDynamicallyTest);

	Context ctx;
	ParameterMapper pm("DynamicFetcher");

	// without config (no path returned)
	assertEqual("", pm.GetSourceSlot(ctx));

	// set in tmp-store
	ctx.GetTmpStore()["DynamicFetcher"]["SourceSlot"] = "Mapper.a.b.c";
	assertEqual("Mapper.a.b.c", pm.GetSourceSlot(ctx));

	// with config (overrides dest in tmp store)
	pm.Initialize("ParameterMapper");
	assertEqual("Mapper.foo.bar", pm.GetSourceSlot(ctx));
}

void ParameterMapperTest::DoGetSourceSlotWithPathTest()
{
	StartTrace(ParameterMapperTest.DoGetSourceSlotWithPathTest);

	Context ctx;
	ParameterMapper pm("PMTest");

	// regular get
	ctx.GetTmpStore()["msg"] = "foo";
	Anything res;
	pm.Get("msg", res, ctx);
	assertEqual("foo", res.AsString());

	// get dynamically
	ctx.GetTmpStore()["PMTest"]["SourceSlot"] = "nested";
	ctx.GetTmpStore()["nested"]["msg"] = "bar";
	pm.Get("msg", res, ctx);
	assertEqual("bar", res.AsString());
}

// builds up a suite of testcases, add a line for each testmethod
Test *ParameterMapperTest::suite ()
{
	StartTrace(ParameterMapperTest.suite);
	TestSuite *testSuite = new TestSuite;

	ADD_CASE(testSuite, ParameterMapperTest, DoSelectScriptTest);
	ADD_CASE(testSuite, ParameterMapperTest, DoLoadConfigTest);
	ADD_CASE(testSuite, ParameterMapperTest, DoGetConfigNameTest);
	ADD_CASE(testSuite, ParameterMapperTest, PlaceIntoAnyOrAppendIfNotEmptyTest);
	ADD_CASE(testSuite, ParameterMapperTest, DoFinalGetAnyTest);
	ADD_CASE(testSuite, ParameterMapperTest, DoFinalGetStreamTest);
	ADD_CASE(testSuite, ParameterMapperTest, DoGetAnyTest);
	ADD_CASE(testSuite, ParameterMapperTest, DoGetStreamTest);
	ADD_CASE(testSuite, ParameterMapperTest, GetTest);
	ADD_CASE(testSuite, ParameterMapperTest, DoSetSourceSlotDynamicallyTest);
	ADD_CASE(testSuite, ParameterMapperTest, DoGetSourceSlotWithPathTest);

	ADD_CASE(testSuite, ParameterMapperTest, EagerDoSelectScriptTest);
	ADD_CASE(testSuite, ParameterMapperTest, EagerGetTest);

	return testSuite;
}
