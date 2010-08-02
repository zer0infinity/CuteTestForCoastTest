/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

//--- interface include --------------------------------------------------------
#include "ResultMapperTest.h"

//--- test modules used --------------------------------------------------------
#include "TestSuite.h"

//--- module under test --------------------------------------------------------
#include "Mapper.h"

//--- standard modules used ----------------------------------------------------
#include "Dbg.h"
#include "StringStream.h"
#include "CacheHandler.h"

//---- ResultMapperTest ----------------------------------------------------------------
ResultMapperTest::ResultMapperTest(TString tstrName) : TestCaseType(tstrName)
{
	StartTrace(ResultMapperTest.Ctor);
}

ResultMapperTest::~ResultMapperTest()
{
	StartTrace(ResultMapperTest.Dtor);
}

void ResultMapperTest::DoSelectScriptTest()
{
	StartTrace(ResultMapperTest.DoSelectScriptTest);

	Anything script, empty;
	script["Sub"] = "a";
	ResultMapper rm("");
	Context ctx;
	assertAnyEqual(rm.SelectScript("Sub", script, ctx), script["Sub"]);
	assertAnyEqual(rm.SelectScript("Nonex", script, ctx), empty);
}

void ResultMapperTest::DoLoadConfigTest()
{
	StartTrace(ResultMapperTest.DoLoadConfigTest);

	ResultMapper rm("ResultMapperTest");
	t_assertm(rm.Initialize("ResultMapper"), "Couldn't load config");

	ResultMapper fm("ResultMapperFail");
	if ( t_assertm(fm.Initialize("NonExistingMapper"), "initialization should succeed") ) {
		t_assertm(fm.GetConfig().IsNull(), "Found a config, but shouldn't!");
	}
}

void ResultMapperTest::DoGetConfigNameTest()
{
	StartTrace(ResultMapperTest.DoGetConfigNameTest);
	ResultMapper rm("GetConfigNameTest");
	rm.Initialize(ResultMapper::gpcCategory);
	assertCharPtrEqual("OutputMapperMeta", rm.GetConfigName());
}

void ResultMapperTest::DoFinalPutAnyTest()
{
	StartTrace(ResultMapperTest.DoFinalPutAnyTest);

	ResultMapper rm("");
	rm.Initialize(ResultMapper::gpcCategory);
	Context ctx;
	Anything a = "first", b = "second", res;

	// 1. Empty key not allowed
	t_assert(!rm.Put("", a, ctx));
	ctx.GetTmpStore().LookupPath(res, "Mapper");
	t_assert(res.IsNull());

	// 2. Use key
	rm.Put("rmt", a, ctx);
	ctx.GetTmpStore().LookupPath(res, "Mapper.rmt");
	assertAnyEqual(a, res);

	rm.Put("rmt", b, ctx);
	ctx.GetTmpStore().LookupPath(res, "Mapper.rmt");
	t_assert(res.GetSize() == 2);
	t_assert(res.Contains(a.AsString()));
	t_assert(res.Contains(b.AsString()));
}

void ResultMapperTest::DoFinalPutStreamTest()
{
	// uses DoFinalPutAny in the end, so just do a sanity check
	String msg = "MessageToStore";
	IStringStream is(msg);
	ResultMapper rm("");
	rm.Initialize(ResultMapper::gpcCategory);
	Context ctx;
	Anything res;

	rm.Put("rmt", is, ctx);
	ctx.GetTmpStore().LookupPath(res, "Mapper.rmt");
	assertEqual(msg, res.AsString());
}

void ResultMapperTest::DoPutAnyTest()
{
	StartTrace(ResultMapperTest.DoPutAnyTest);
	// --- 1. Mapper without script (i.e. script == empty)
	{
		Context ctx;
		Anything any = "value";
		ResultMapper rm("MyPlainMapperWithoutConfig");
		rm.Initialize(ResultMapper::gpcCategory);
		t_assert(rm.Put("rmt", any, ctx));
		assertAnyEqual(any, ctx.GetTmpStore()["Mapper"]["rmt"]);
	}
	// --- 2. Mapper with simple valued script (key/context do not matter)
	{
		Context ctx;
		Anything any = "value";
		ResultMapper rm("SingleScript1");
		rm.Initialize(ResultMapper::gpcCategory);
		rm.Put("ding", any, ctx);
		assertAnyEqual(any, ctx.GetTmpStore()["Mapper"]["ding"]);
	}
	// --- 3. Mapper with a full script (interpreatation, init key doesn't matter)
	// 3.1 DistributorScript
	{
		Context ctx;
		Anything any = "value";
		ResultMapper rm("DistributorScript");
		rm.Initialize(ResultMapper::gpcCategory);
		rm.Put("out", any, ctx);
		OStringStream os;
		os << ctx.Lookup("Mapper") << std::flush;
		assertEqual("{\n  /a "_QUOTE_("value")"\n  /b "_QUOTE_("value")"\n  /c "_QUOTE_("value")"\n  /d "_QUOTE_("value")"\n}", os.str());
		TraceAny(ctx.GetTmpStore(), "tmp");
	}
	// 3.2 DelegationScript
	{
		Context ctx;
		Anything any = "value";
		ResultMapper rm("AnyPlacerScript");
		rm.Initialize(ResultMapper::gpcCategory);
		rm.Put("Inside", any, ctx);
		assertAnyEqualm(any, ctx.GetTmpStore()["SomeSlot"]["Inside"], "Was NameUsingOutputMapper called?");
		assertAnyEqualm(any, ctx.GetTmpStore()["SomeOtherSlot"]["Deep"]["Inside"], "Was NameUsingOutputMapper called?");
		TraceAny(ctx.GetTmpStore(), "tmp");
	}
}

void ResultMapperTest::DoPutStreamTest()
{
	StartTrace(ResultMapperTest.DoFinalPutStreamTest);
	// --- 1. Mapper without script (i.e. script == empty)
	{
		Context ctx;
		ResultMapper rm("MyPlainMapperWithoutConfig");
		rm.Initialize(ResultMapper::gpcCategory);
		String msg = "myMsgStream";
		IStringStream is(msg);
		rm.Put("rmt", is, ctx);
		assertEqual(msg, ctx.GetTmpStore()["Mapper"]["rmt"].AsString());
	}
	// --- 2. Mapper with simple valued script (key/context do not matter)
	{
		Context ctx;
		ResultMapper rm("SingleScript1");
		rm.Initialize(ResultMapper::gpcCategory);
		String msg = "myMsgStream";
		IStringStream is2(msg);
		rm.Put("ding", is2, ctx);
		assertEqual(msg, ctx.GetTmpStore()["Mapper"]["ding"].AsString());
	}
	// --- 3. Mapper with a full script (interpreatation)
	{
		Context ctx;
		ResultMapper rm("StreamPlacerScript");
		rm.Initialize(ResultMapper::gpcCategory);
		String msg = "myMsgStream";
		IStringStream is3(msg);
		Anything res;
		rm.Put("StreamSlot", is3, ctx);
		assertEqualm(msg, ctx.GetTmpStore()["SomeSlot"]["StreamSlot"].AsString(), "Was NameUsingOutputMapper called?");
		t_assert(ctx.GetTmpStore().LookupPath(res, "EmptyStuff.StreamSlot"));
		assertEqualm("", res.AsString(), "Should be empty, because stream was consumed!");
		TraceAny(ctx.GetTmpStore(), "tmp");
	}
}

void ResultMapperTest::PutTest()
{
	StartTrace(ResultMapperTest.PutTest);

	ResultMapper rm("ResultMapperTest");
	rm.Initialize("ResultMapper");
	Context ctx;
	String s("hello"), msg("message");
	long l = 1234L;
	double d = 3.1415;
	bool b = true;
	Anything any;
	any["a"] = "foo";
	any["b"] = "bar";
	IStringStream is(msg);
	Anything tmp = ctx.GetTmpStore();

	// put simple values
	rm.Put("string", s, ctx);
	assertEqual(s, tmp["Mapper"]["sKey"].AsString());

	rm.Put("bool", b, ctx);
	assertEqual(b, tmp["Mapper"]["bKey"].AsBool());

	rm.Put("long", l, ctx);
	assertEqual(l, tmp["Mapper"]["lKey"].AsLong());

	rm.Put("double", d, ctx);
	assertDoublesEqual(d, tmp["Mapper"]["dKey"].AsDouble(), 1);

	// put any
	rm.Put("any", any, ctx);
	assertAnyEqual(any, tmp["Mapper"]["aKey"]);

	// put stream
	rm.Put("stream", is, ctx);
	assertEqual(msg, tmp["Mapper"]["strmKey"].AsString());

	// unknown key in script is put under Mapper.key
	rm.Put("unknownKey", msg, ctx);
	assertEqual(msg, tmp["Mapper"]["unknownKey"].AsString());

	// another put will append the new result
	Anything anyExp = any;
	anyExp.Append(any);
	rm.Put("any", any, ctx);
	assertAnyEqual(anyExp, tmp["Mapper"]["aKey"]);

	// test with special always appending mapper
	ResultMapper arm("AppendingResultMapper");
	arm.Initialize("ResultMapper");

	// put any
	anyExp = Anything();
	anyExp.Append(any);
	arm.Put("any", any, ctx);
	assertAnyEqual(anyExp, tmp["AppendAnyMapper"]["aKey"]);
	// another put will append the new result
	anyExp.Append(any);
	arm.Put("any", any, ctx);
	assertAnyEqual(anyExp, tmp["AppendAnyMapper"]["aKey"]);

	anyExp = Anything();
	anyExp.Append(l);
	arm.Put("long", l, ctx);
	assertAnyEqual(anyExp, tmp["AppendAnyMapper"]["lKey"]);
	anyExp.Append(l);
	arm.Put("long", l, ctx);
	assertAnyEqual(anyExp, tmp["AppendAnyMapper"]["lKey"]);
}

void ResultMapperTest::EagerDoSelectScriptTest()
{
	StartTrace(ParameterMapperTest.EagerDoSelectScriptTest);

	Anything script;
	script["Sub"] = "a";
	EagerResultMapper erm("");
	Context ctx;
	assertAnyEqual(erm.SelectScript("Sub", script, ctx), script["Sub"]);
	assertAnyEqual(erm.SelectScript("Nonex", script, ctx), script);
}

void ResultMapperTest::EagerPutTest()
{
	StartTrace(ResultMapperTest.EagerPutTest);

	// eager mapper gets a script in any case, even if key is not found
	// which leads to a distribution of the information
	EagerResultMapper erm("ResultMapperTest");
	erm.Initialize("ResultMapper");
	Context ctx;
	String msg("foo");

	erm.Put("unknownKey", msg, ctx);

	OStringStream os;
	os << ctx.GetTmpStore()["Mapper"];
	String strIn("{ /sKey foo /dKey foo /lKey foo /bKey foo /aKey foo /strmKey foo }");
	IStringStream is(strIn);
	Anything a;
	is >> a;
	OStringStream os2;
	os2 << a;
	assertEqual(os2.str(), os.str());
}

void ResultMapperTest::DoSetDestinationSlotDynamicallyTest()
{
	StartTrace(ResultMapperTest.SetDestinationSlotTest);

	Context ctx;
	ResultMapper rm("DynamicStorer");

	// without config
	assertCharPtrEqual("Mapper", rm.GetDestinationSlot(ctx));

	// set in tmp-store
	ctx.GetTmpStore()["DynamicStorer"]["DestinationSlot"] = "Mapper.a.b.c";
	assertCharPtrEqual("Mapper.a.b.c", rm.GetDestinationSlot(ctx));

	// with config (overrides dest in tmp store)
	rm.Initialize("ResultMapper");
	assertCharPtrEqual("Mapper.foo.bar", rm.GetDestinationSlot(ctx));
}

void ResultMapperTest::DoGetDestinationSlotWithPathTest()
{
	StartTrace(ResultMapperTest.DoGetDestinationSlotWithPathTest);

	Context ctx;
	PathTestMapper ptm("");
	assertCharPtrEqual("Mapper.x.y.z", ptm.GetDestinationSlot(ctx));

	// assign first
	ptm.Put("msg", String("foo"), ctx);
	TraceAny(ctx.GetTmpStore(), "tmp");
	Anything res;
	ctx.GetTmpStore().LookupPath(res, "Mapper.x.y.z.msg");
	assertEqual("foo", res.AsString());

	// assign second
	ptm.Put("msg", String("bar"), ctx);
	ctx.GetTmpStore().LookupPath(res, "Mapper.x.y.z.msg");
	assertEqual(2, res.GetSize());
	t_assert(res.Contains("foo"));
	t_assert(res.Contains("bar"));
}

// builds up a suite of testcases, add a line for each testmethod
Test *ResultMapperTest::suite ()
{
	StartTrace(ResultMapperTest.suite);
	TestSuite *testSuite = new TestSuite;

	ADD_CASE(testSuite, ResultMapperTest, DoLoadConfigTest);
	ADD_CASE(testSuite, ResultMapperTest, DoSelectScriptTest);
	ADD_CASE(testSuite, ResultMapperTest, DoGetConfigNameTest);
	ADD_CASE(testSuite, ResultMapperTest, DoFinalPutAnyTest);
	ADD_CASE(testSuite, ResultMapperTest, DoFinalPutStreamTest);
	ADD_CASE(testSuite, ResultMapperTest, DoPutAnyTest);
	ADD_CASE(testSuite, ResultMapperTest, DoPutStreamTest);
	ADD_CASE(testSuite, ResultMapperTest, PutTest);
	ADD_CASE(testSuite, ResultMapperTest, DoSetDestinationSlotDynamicallyTest);
	ADD_CASE(testSuite, ResultMapperTest, DoGetDestinationSlotWithPathTest);

	ADD_CASE(testSuite, ResultMapperTest, EagerDoSelectScriptTest);
	ADD_CASE(testSuite, ResultMapperTest, EagerPutTest);
	return testSuite;
}
