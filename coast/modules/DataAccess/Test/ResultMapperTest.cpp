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
ResultMapperTest::ResultMapperTest(TString name) : TestCase(name)
{
	StartTrace(ResultMapperTest.Ctor);
}

ResultMapperTest::~ResultMapperTest()
{
	StartTrace(ResultMapperTest.Dtor);
}

// setup for this TestCase
void ResultMapperTest::setUp ()
{
	StartTrace(ResultMapperTest.setUp);
}

void ResultMapperTest::tearDown ()
{
	StartTrace(ResultMapperTest.tearDown);
}

void ResultMapperTest::testDoSelectScript()
{
	StartTrace(ResultMapperTest.testDoSelectScript);

	Anything script, empty;
	script["Sub"] = "a";
	ResultMapper rm("");

	assertAnyEqual(rm.DoSelectScript("Sub", script), script["Sub"]);
	assertAnyEqual(rm.DoSelectScript("Nonex", script), empty);
}

void ResultMapperTest::testDoLoadConfig()
{
	StartTrace(ResultMapperTest.testDoLoadConfig);

	ResultMapper rm("ResultMapperTest");

	t_assertm(rm.DoLoadConfig("ResultMapper"), "Couldn't load config");
	t_assertm(!rm.DoLoadConfig("NonExistingMapper"), "Found a config, but shouldn't!");
}

void ResultMapperTest::testDoGetConfigName()
{
	StartTrace(ResultMapperTest.testDoGetConfigName);

	String configName;
	ResultMapper rm("");

	rm.DoGetConfigName("ResultMapper", "", configName);
	assertEqual("OutputMapperMeta", configName);
	rm.DoGetConfigName("SomeOtherMapper", "", configName);
	assertEqual("SomeOtherMapperMeta", configName);
}

void ResultMapperTest::testDoFinalPutAny()
{
	StartTrace(ResultMapperTest.testDoFinalPutAny);

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

void ResultMapperTest::testDoFinalPutStream()
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

void ResultMapperTest::testDoPutAny()
{
	StartTrace(ResultMapperTest.testDoPutAny);

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

	script = rm.DoSelectScript("SingleScript1", scripts);
	rm.DoPutAny("-", any, ctx, script);
	assertAnyEqual(any, ctx.GetTmpStore()["Mapper"]["ding"]);

	// --- 3. Mapper with a full script (interpreatation, init key doesn't matter)

	// 3.1 DistributorScript
	Context ctx2;
	script = rm.DoSelectScript("DistributorScript", scripts);
	rm.DoPutAny("-", any, ctx2, script);
	OStringStream os;
	os << ctx2.GetTmpStore()["Mapper"];
	assertEqual("{\n  /a "_QUOTE_("value")"\n  /b "_QUOTE_("value")"\n  /c "_QUOTE_("value")"\n  /d "_QUOTE_("value")"\n}", os.str());

	// 3.2 DelegationScript
	script = rm.DoSelectScript("AnyPlacerScript", scripts);
	rm.DoPutAny("Inside", any, ctx, script);
	assertAnyEqualm(any, ctx.GetTmpStore()["SomeSlot"]["Inside"], "Was NameUsingOutputMapper called?");
	assertAnyEqualm(any, ctx.GetTmpStore()["SomeOtherSlot"]["Deep"]["Inside"], "Was NameUsingOutputMapper called?");

	TraceAny(ctx.GetTmpStore(), "tmp");
}

void ResultMapperTest::testDoPutStream()
{
	StartTrace(ResultMapperTest.testDoFinalPutStream);

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
	script = rm.DoSelectScript("SingleScript1", scripts);
	rm.DoPutStream("-", is2, ctx, script);
	assertEqual(msg, ctx.GetTmpStore()["Mapper"]["ding"].AsString());

	// --- 3. Mapper with a full script (interpreatation)

	IStringStream is3(msg);
	Anything res;
	script = rm.DoSelectScript("StreamPlacerScript", scripts);
	rm.DoPutStream("StreamSlot", is3, ctx, script);
	assertEqualm(msg, ctx.GetTmpStore()["SomeSlot"]["StreamSlot"].AsString(), "Was NameUsingOutputMapper called?");
	t_assert(ctx.GetTmpStore().LookupPath(res, "EmptyStuff.StreamSlot"));
	assertEqualm("", res.AsString(), "Should be empty, because stream was consumed!");
}

void ResultMapperTest::testPut()
{
	StartTrace(ResultMapperTest.testPut);

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

void ResultMapperTest::testEagerDoSelectScript()
{
	StartTrace(ParameterMapperTest.testEagerDoSelectScript);

	Anything script;
	script["Sub"] = "a";
	EagerResultMapper erm("");

	assertAnyEqual(erm.DoSelectScript("Sub", script), script["Sub"]);
	assertAnyEqual(erm.DoSelectScript("Nonex", script), script);
}

void ResultMapperTest::testEagerPut()
{
	StartTrace(ResultMapperTest.testEagerPut);

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

void ResultMapperTest::testDoSetDestinationSlotDynamically()
{
	StartTrace(ResultMapperTest.testSetDestinationSlot);

	Context ctx;
	ResultMapper rm("DynamicStorer");

	// without config
	assertEquals("Mapper", rm.DoGetDestinationSlot(ctx));

	// set in tmp-store
	ctx.GetTmpStore()["DynamicStorer"]["DestinationSlot"] = "Mapper.a.b.c";
	assertEquals("Mapper.a.b.c", rm.DoGetDestinationSlot(ctx));

	// with config (overrides dest in tmp store)
	rm.CheckConfig("ResultMapper");
	assertEquals("Mapper.foo.bar", rm.DoGetDestinationSlot(ctx));
}

void ResultMapperTest::testDoGetDestinationSlotWithPath()
{
	StartTrace(ResultMapperTest.testDoGetDestinationSlotWithPath);

	Context ctx;
	PathTestMapper ptm("");
	assertEquals("Mapper.x.y.z", ptm.DoGetDestinationSlot(ctx));

	// assign first
	ptm.Put("msg", String("foo"), ctx);
	TraceAny(ctx.GetTmpStore(), "tmp");
	Anything res;
	ctx.GetTmpStore().LookupPath(res, "Mapper.x.y.z.msg");
	assertEquals("foo", res.AsString());

	// assign second
	ptm.Put("msg", String("bar"), ctx);
	ctx.GetTmpStore().LookupPath(res, "Mapper.x.y.z.msg");
	assertEquals(2, res.GetSize());
	t_assert(res.Contains("foo"));
	t_assert(res.Contains("bar"));
}

// builds up a suite of testcases, add a line for each testmethod
Test *ResultMapperTest::suite ()
{
	StartTrace(ResultMapperTest.suite);
	TestSuite *testSuite = new TestSuite;

	ADD_CASE(testSuite, ResultMapperTest, testDoLoadConfig);
	ADD_CASE(testSuite, ResultMapperTest, testDoSelectScript);
	ADD_CASE(testSuite, ResultMapperTest, testDoGetConfigName);
	ADD_CASE(testSuite, ResultMapperTest, testDoFinalPutAny);
	ADD_CASE(testSuite, ResultMapperTest, testDoFinalPutStream);
	ADD_CASE(testSuite, ResultMapperTest, testDoPutAny);
	ADD_CASE(testSuite, ResultMapperTest, testDoPutStream);
	ADD_CASE(testSuite, ResultMapperTest, testPut);
	ADD_CASE(testSuite, ResultMapperTest, testDoSetDestinationSlotDynamically);
	ADD_CASE(testSuite, ResultMapperTest, testDoGetDestinationSlotWithPath);

	ADD_CASE(testSuite, ResultMapperTest, testEagerDoSelectScript);
	ADD_CASE(testSuite, ResultMapperTest, testEagerPut);
	return testSuite;
}
