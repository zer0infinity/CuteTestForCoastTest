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

//--- c-modules used -----------------------------------------------------------

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
	assertAnyEqual(rm.DoSelectScript("Sub", script, ctx), script["Sub"]);
	assertAnyEqual(rm.DoSelectScript("Nonex", script, ctx), empty);
}

void ResultMapperTest::DoLoadConfigTest()
{
	StartTrace(ResultMapperTest.DoLoadConfigTest);

	ResultMapper rm("ResultMapperTest");

	t_assertm(rm.DoLoadConfig("ResultMapper"), "Couldn't load config");
	t_assertm(!rm.DoLoadConfig("NonExistingMapper"), "Found a config, but shouldn't!");
}

void ResultMapperTest::DoGetConfigNameTest()
{
	StartTrace(ResultMapperTest.DoGetConfigNameTest);

	String configName;
	ResultMapper rm("");

	rm.DoGetConfigName("ResultMapper", "", configName);
	assertEqual("OutputMapperMeta", configName);
	rm.DoGetConfigName("SomeOtherMapper", "", configName);
	assertEqual("SomeOtherMapperMeta", configName);
}

void ResultMapperTest::DoFinalPutAnyTest()
{
	StartTrace(ResultMapperTest.DoFinalPutAnyTest);

	ResultMapper rm("");
	Context ctx;
	Anything a = "first", b = "second", res;

	// 1. Empty key not allowed
	t_assert(!rm.DoFinalPutAny("", a, ctx));
	ctx.GetTmpStore().LookupPath(res, "Mapper");
	t_assert(res.IsNull());

	// 2. Use key
	rm.DoFinalPutAny("rmt", a, ctx);
	ctx.GetTmpStore().LookupPath(res, "Mapper.rmt");
	assertAnyEqual(a, res);

	rm.DoFinalPutAny("rmt", b, ctx);
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
	Context ctx;
	Anything res;

	rm.DoFinalPutStream("rmt", is, ctx);
	ctx.GetTmpStore().LookupPath(res, "Mapper.rmt");
	assertEqual(msg, res.AsString());
}

void ResultMapperTest::DoPutAnyTest()
{
	StartTrace(ResultMapperTest.DoPutAnyTest);

	SimpleAnyLoader sal;
	Anything scripts = sal.Load("MapperTestScripts");
	ROAnything script;
	Anything any = "value";
	Context ctx;
	ResultMapper rm("");

	// --- 1. Mapper without script (i.e. script == empty)

	rm.DoPutAny("rmt", any, ctx, script);
	assertAnyEqual(any, ctx.GetTmpStore()["Mapper"]["rmt"]);

	// --- 2. Mapper with simple valued script (key/context do not matter)

	script = rm.DoSelectScript("SingleScript1", scripts, ctx);
	rm.DoPutAny("-", any, ctx, script);
	assertAnyEqual(any, ctx.GetTmpStore()["Mapper"]["ding"]);

	// --- 3. Mapper with a full script (interpreatation, init key doesn't matter)

	// 3.1 DistributorScript
	Context ctx2;
	script = rm.DoSelectScript("DistributorScript", scripts, ctx);
	rm.DoPutAny("-", any, ctx2, script);
	OStringStream os;
	os << ctx2.GetTmpStore()["Mapper"];
	assertEqual("{\n  /a "_QUOTE_("value")"\n  /b "_QUOTE_("value")"\n  /c "_QUOTE_("value")"\n  /d "_QUOTE_("value")"\n}", os.str());

	// 3.2 DelegationScript
	script = rm.DoSelectScript("AnyPlacerScript", scripts, ctx);
	rm.DoPutAny("Inside", any, ctx, script);
	assertAnyEqualm(any, ctx.GetTmpStore()["SomeSlot"]["Inside"], "Was NameUsingOutputMapper called?");
	assertAnyEqualm(any, ctx.GetTmpStore()["SomeOtherSlot"]["Deep"]["Inside"], "Was NameUsingOutputMapper called?");

	TraceAny(ctx.GetTmpStore(), "tmp");
}

void ResultMapperTest::DoPutStreamTest()
{
	StartTrace(ResultMapperTest.DoFinalPutStreamTest);

	// cannot "distribute" a stream like in example above, since
	// stream will be consumed when accessed - no renewal

	SimpleAnyLoader sal;
	Anything scripts = sal.Load("MapperTestScripts");
	ROAnything script;
	Context ctx;
	ResultMapper rm("");
	String msg = "myMsgStream";

	// --- 1. Mapper without script (i.e. script == empty)

	IStringStream is(msg);
	rm.DoPutStream("rmt", is, ctx, script);
	assertEqual(msg, ctx.GetTmpStore()["Mapper"]["rmt"].AsString());

	// --- 2. Mapper with simple valued script (key/context do not matter)

	IStringStream is2(msg);
	script = rm.DoSelectScript("SingleScript1", scripts, ctx);
	rm.DoPutStream("-", is2, ctx, script);
	assertEqual(msg, ctx.GetTmpStore()["Mapper"]["ding"].AsString());

	// --- 3. Mapper with a full script (interpreatation)

	IStringStream is3(msg);
	Anything res;
	script = rm.DoSelectScript("StreamPlacerScript", scripts, ctx);
	rm.DoPutStream("StreamSlot", is3, ctx, script);
	assertEqualm(msg, ctx.GetTmpStore()["SomeSlot"]["StreamSlot"].AsString(), "Was NameUsingOutputMapper called?");
	t_assert(ctx.GetTmpStore().LookupPath(res, "EmptyStuff.StreamSlot"));
	assertEqualm("", res.AsString(), "Should be empty, because stream was consumed!");
}

void ResultMapperTest::PutTest()
{
	StartTrace(ResultMapperTest.PutTest);

	ResultMapper rm("ResultMapperTest");
	rm.CheckConfig("ResultMapper");
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
	arm.CheckConfig("ResultMapper");

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
	assertAnyEqual(erm.DoSelectScript("Sub", script, ctx), script["Sub"]);
	assertAnyEqual(erm.DoSelectScript("Nonex", script, ctx), script);
}

void ResultMapperTest::EagerPutTest()
{
	StartTrace(ResultMapperTest.EagerPutTest);

	// eager mapper gets a script in any case, even if key is not found
	// which leads to a distribution of the information
	EagerResultMapper erm("ResultMapperTest");
	erm.CheckConfig("ResultMapper");
	Context ctx;
	String msg("foo");

	erm.Put("unknownKey", msg, ctx);

	OStringStream os;
	os << ctx.GetTmpStore()["Mapper"];
	IStringStream is("{ /sKey foo /dKey foo /lKey foo /bKey foo /aKey foo /strmKey foo }");
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
	rm.CheckConfig("ResultMapper");
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
