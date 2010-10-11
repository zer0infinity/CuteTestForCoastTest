/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

//--- interface include --------------------------------------------------------
#include "MapperTest.h"

//--- module under test --------------------------------------------------------
#include "Mapper.h"

//--- test modules used --------------------------------------------------------
#include "TestSuite.h"

//--- standard modules used ----------------------------------------------------
#include "DataAccessImpl.h"
#include "Dbg.h"

//--- c-library modules used ---------------------------------------------------

//---- MapperTest ----------------------------------------------------------------
Test *MapperTest::suite ()
{
	TestSuite *testSuite = new TestSuite;

	ADD_CASE(testSuite, MapperTest, GetTests);
	ADD_CASE(testSuite, MapperTest, StdGetTest);
	ADD_CASE(testSuite, MapperTest, StdGetNoDataTest);
	ADD_CASE(testSuite, MapperTest, StdPutTest);
	ADD_CASE(testSuite, MapperTest, ExtendedPutTest);
	ADD_CASE(testSuite, MapperTest, GetConfigNameTest);
	ADD_CASE(testSuite, MapperTest, DoLoadConfigTest);
	ADD_CASE(testSuite, MapperTest, HardConfiguredGet);
	ADD_CASE(testSuite, MapperTest, MixedConfiguredGet);
	ADD_CASE(testSuite, MapperTest, LookupMapperGetTest);
	ADD_CASE(testSuite, MapperTest, RenameSlotWithConfigPutTest);
	ADD_CASE(testSuite, MapperTest, ScriptedPutTest);
	ADD_CASE(testSuite, MapperTest, RenameSlotWithConfigGetTest);

	return testSuite;
}

MapperTest::MapperTest(TString tname)
	: TestCaseType(tname)
{
}

MapperTest::~MapperTest()
{
}

void MapperTest::HardConfiguredGet()
{
	StartTrace(MapperTest.HardConfiguredGet);

	Anything dummy;
	Context ctx(dummy, dummy, (Server *)0, (Session *)0, (Role *)0);
	EagerParameterMapper httpmapper("HTTPHardCodedMapperTest");
	httpmapper.Initialize("ParameterMapper");

	String input("<");
	OStringStream Ios(&input);

	t_assert(httpmapper.Get("Input", Ios, ctx));
	Ios << std::flush;
	input << ">";
	Trace("Input: " << input);
	assertEqual("<GET /finVAL/XS.ISA?Action=Login HTTP/1.1\ntestcase: foobah\nContent-length: 0\n\n>", input);
}

void MapperTest::MixedConfiguredGet()
{
	StartTrace(MapperTest.MixedConfiguredGet);

	Anything inputData;
	inputData["trxinput"]["data"]["vorname"] = "Peter";
	inputData["trxinput"]["data"]["nachname"] = "Sommerlad";
	inputData["trxinput"]["data"]["server"] = "strozzi";
	inputData["trxinput"]["data"]["geb"] = "xxxx.xx.xx";
	inputData["trxinput"]["data"]["cvks"] = "PATTERNs";
	Context ctx(inputData, inputData, (Server *)0, (Session *)0, (Role *)0);

	EagerParameterMapper httpmapper("HostDAInputMapperTest");
	httpmapper.Initialize("ParameterMapper");

	String input("<");
	String input1("<");
	OStringStream Ios(&input);
	OStringStream ios1(&input1);

	t_assert(httpmapper.Get("Msg", Ios, ctx));
	Ios << std::flush;
	input << ">";
	Trace("Input: " << input);
	assertEqual("<A1B7L007GET-HOMEDIR*****PETER                "
				"SOMMERLAD           :"
				"strozzi             :"
				"xxxx.xx.xx          :"
				"PATTERNs>", input);

	t_assert(httpmapper.Get("FixedSize", ios1, ctx));
	ios1 << std::flush;
	input1 << ">";
	Trace("Input: " << input1);
	assertEqual("<strozzi             >", input1);
}

void MapperTest::GetTests()
{
	StartTrace(MapperTest.GetTests);

	ParameterMapper mapper("MapperTest");
	mapper.Initialize("ParameterMapper");

	Context ctx;
	ctx.GetTmpStore()["AKeyFromContext"] = 9.1;
	ctx.GetTmpStore()["AKeyInConfig"] = 5.5;
	ctx.GetTmpStore()["ATrueFromContext"] = 1;
	ctx.GetTmpStore()["AFalseFromContext"] = 0;

	{
		// if used with anything else than a Long it should not work here
		bool b = false;
		t_assert(mapper.Get("AKeyInConfig", b, ctx));
		assertEqualm(false, b, "bool from config");
		t_assert(mapper.Get("AKeyFromContext", b, ctx));
		assertEqualm(false, b, "bool from context");
		t_assert(!mapper.Get("AKeyThatDoesNotExist", b, ctx));
		assertEqualm(false, b, "bool unchanged");
		b = true;
		t_assert(mapper.Get("AKeyInConfig", b, ctx));
		assertEqualm(true, b, "bool from config");
		t_assert(mapper.Get("AKeyFromContext", b, ctx));
		assertEqualm(true, b, "bool from context");
		t_assert(!mapper.Get("AKeyThatDoesNotExist", b, ctx));
		assertEqualm(true, b, "bool unchanged");

		// now do the correct cases on long
		b = true;
		t_assert(mapper.Get("AFalseInConfig", b, ctx));
		assertEqualm(false, b, "bool from config");
		b = true;
		t_assert(mapper.Get("AFalseFromContext", b, ctx));
		assertEqualm(false, b, "bool from context");
		b = true;
		t_assert(!mapper.Get("AFalseThatDoesNotExist", b, ctx));
		assertEqualm(true, b, "bool unchanged");
		b = false;
		t_assert(mapper.Get("ATrueInConfig", b, ctx));
		assertEqualm(true, b, "bool from config");
		b = false;
		t_assert(mapper.Get("ATrueFromContext", b, ctx));
		assertEqualm(true, b, "bool from context");
		b = false;
		t_assert(!mapper.Get("ATrueThatDoesNotExist", b, ctx));
		assertEqualm(false, b, "bool unchanged");
	}

	{
		float f;
		t_assert(mapper.Get("AKeyInConfig", f, ctx));
		assertDoublesEqualm(1.9, f, 0.0000001, "float from config");
		t_assert(mapper.Get("AKeyFromContext", f, ctx));
		assertDoublesEqualm(9.1, f, 0.000001, "float from context");
		t_assert(!mapper.Get("AKeyThatDoesNotExist", f, ctx));
		assertDoublesEqualm(9.1, f, 0.000001, "float unchanged");
	}

	{
		double d;
		t_assert(mapper.Get("AKeyInConfig", d, ctx));
		assertDoublesEqualm(1.9, d, 0.0000001, "double from config");
		t_assert(mapper.Get("AKeyFromContext", d, ctx));
		assertDoublesEqualm(9.1, d, 0.0000001, "double from context");
		t_assert(!mapper.Get("AKeyThatDoesNotExist", d, ctx));
		assertDoublesEqualm(9.1, d, 0.0000001, "double unchanged");
	}

	{
		int i;
		t_assert(mapper.Get("AKeyInConfig", i, ctx));
		assertEqualm(1, i, "int from config");
		t_assert(mapper.Get("AKeyFromContext", i, ctx));
		assertEqualm(9, i, "int from context");
		t_assert(!mapper.Get("AKeyThatDoesNotExist", i, ctx));
		assertEqualm(9, i, "int unchanged");
	}

	{
		long l;
		t_assert(mapper.Get("AKeyInConfig", l, ctx));
		assertEqualm(1, l, "long from config");
		t_assert(mapper.Get("AKeyFromContext", l, ctx));
		assertEqualm(9, l, "long from context");
		t_assert(!mapper.Get("AKeyThatDoesNotExist", l, ctx));
		assertEqualm(9, l, "long unchanged");
	}

	{
		String s;
		t_assert(mapper.Get("AKeyInConfig", s, ctx));
		assertEqualm("1.9", s, "String from config");
		t_assert(mapper.Get("AKeyFromContext", s, ctx));
		assertEqualm("9.1", s, "String from context");
		t_assert(!mapper.Get("AKeyThatDoesNotExist", s, ctx));
		assertEqualm("9.1", s, "String unchanged");
	}

	{
		String s;
		OStringStream os(s);
		t_assert(mapper.Get("AKeyInConfig", os, ctx));
		os.flush();
		assertEqualm("1.9", s, "Stream from config");
	}

	{
		String s;
		OStringStream os(s);
		t_assert(mapper.Get("AKeyFromContext", os, ctx));
		os.flush();
		assertEqualm("9.1", s, "Stream from context");
		t_assert(!mapper.Get("AKeyThatDoesNotExist", os, ctx));
		os.flush();
		assertEqualm("9.1", s, "Stream unchanged");
	}

	{
		Anything a;
		Anything expected;
		expected["ExpectedKeyFromConfig"] = 1.9;
		expected["ExpectedKeyFromContext"] = 9.1;
		t_assert(mapper.Get("AKeyInConfig", a, ctx));
		assertAnyEqualm(expected["ExpectedKeyFromConfig"], a, "Anything from config");
		t_assert(mapper.Get("AKeyFromContext", a, ctx));
		assertAnyEqualm(expected["ExpectedKeyFromContext"], a, "Anything from context");
		t_assert(!mapper.Get("AKeyThatDoesNotExist", a, ctx));
		assertAnyEqualm(expected["ExpectedKeyFromContext"], a, "Anything unchanged");
	}
}

void MapperTest::StdGetTest()
{
	StartTrace(MapperTest.StdGetTest);
	Anything dummy, anyContext(GetConfig().DeepClone());
	Context ctx(anyContext, dummy, 0, 0, 0, 0);

	ParameterMapper mapper("stdtestgetmapper");
	mapper.Initialize("ParameterMapper");

	// test the overloaded get api
	int iTestVal;
	t_assert(mapper.Get("testInt", iTestVal, ctx));
	assertEqual(10, iTestVal);

	bool bTestVal = true;
	t_assert(mapper.Get("testBool", bTestVal, ctx));
	assertEqual(false, bTestVal);

	long lTestVal;
	t_assert(mapper.Get("testLong", lTestVal, ctx));
	assertEqual(123, lTestVal);

	float fTestVal;
	t_assert(mapper.Get("testFloat", fTestVal, ctx));
	t_assert(1.23F == fTestVal);

	double dTestVal;
	t_assert(mapper.Get("testDouble", dTestVal, ctx));
	t_assert(2.46 == dTestVal);

	String sTestVal;
	t_assert(mapper.Get("testString", sTestVal, ctx));
	assertEqual("foo baz", sTestVal);

	Anything aTestVal;
	t_assert(mapper.Get("testAny", aTestVal, ctx));
	TraceAny(aTestVal, "aTestVal");
	t_assert(aTestVal.IsDefined("foo"));
	assertEqual("baz", aTestVal["foo"].AsCharPtr());

	String strTest;
	StringStream Ios(strTest);
	t_assert(mapper.Get("testString", Ios, ctx));
	Ios << std::flush;
	assertEqual("foo baz", strTest);
}

void MapperTest::StdGetNoDataTest()
{
	Context ctx;

	ParameterMapper mapper("stdtestgetmapper");
	mapper.Initialize("ParameterMapper");

	// test the overloaded get api
	int iTestVal;
	t_assert(!mapper.Get("testInt", iTestVal, ctx));

	bool bTestVal;
	t_assert(!mapper.Get("testBool", bTestVal, ctx));

	long lTestVal;
	t_assert(!mapper.Get("testLong", lTestVal, ctx));

	float fTestVal;
	t_assert(!mapper.Get("testFloat", fTestVal, ctx));

	double dTestVal;
	t_assert(!mapper.Get("testDouble", dTestVal, ctx));

	String sTestVal;
	t_assert(!mapper.Get("testString", sTestVal, ctx));

	Anything aTestVal;
	t_assert(!mapper.Get("testAny", aTestVal, ctx));

	StringStream os;
	t_assert(!mapper.Get("noStdStreams", os, ctx));
}

void MapperTest::StdPutTest()
{
	Anything dummy;
	Context ctx(dummy, dummy, 0, 0, 0, 0);
	Anything tmpStore(ctx.GetTmpStore());

	const char *mappername = "Mapper";
	ResultMapper mapper(mappername);
	mapper.Initialize("ResultMapper");

	// test the overloaded get api
	int iTestVal = 10;
	t_assert(mapper.Put("testInt", iTestVal, ctx));
	assertEqual(10, tmpStore[mappername]["testInt"].AsLong());

	bool bTestVal = true;
	t_assert(mapper.Put("testBool", bTestVal, ctx));
	assertEqual(true, tmpStore[mappername]["testBool"].AsBool());

	long lTestVal = 123;
	t_assert(mapper.Put("testLong", lTestVal, ctx));
	assertEqual(123, tmpStore[mappername]["testLong"].AsLong());

	float fTestVal = 1.23F;
	t_assert(mapper.Put("testFloat", fTestVal, ctx));
	t_assert(1.23F == (float)tmpStore[mappername]["testFloat"].AsDouble());

	double dTestVal = 2.46;
	t_assert(mapper.Put("testDouble", dTestVal, ctx));
	t_assert(2.46 == tmpStore[mappername]["testDouble"].AsDouble());

	String sTestVal = "foo baz";
	t_assert(mapper.Put("testString", sTestVal, ctx));
	assertEqual("foo baz", tmpStore[mappername]["testString"].AsCharPtr());

	Anything aTestVal;
	aTestVal["foo"] = "baz";
	t_assert(mapper.Put("testAny", aTestVal, ctx));
	t_assert(tmpStore[mappername]["testAny"].IsDefined("foo"));
	assertEqual("baz", tmpStore[mappername]["testAny"]["foo"].AsCharPtr());

	String test;
	test << "#--\x0A# Copyright (c) 2005 ifs\x0A# All Rights Reserved\x0A#--";
	IStringStream is(test);
	t_assert(mapper.Put("StdStreams", is, ctx));
	t_assert(tmpStore[mappername].IsDefined("StdStreams"));
	assertEqual("#--\x0A# Copyright (c) 2005 ifs\x0A# All Rights Reserved\x0A#--", tmpStore[mappername]["StdStreams"].AsCharPtr());
}

void MapperTest::ExtendedPutTest()
{
	StartTrace(MapperTest.ExtendedPutTest);
	Anything testRecord(GetConfig()["ArrayTestConfig"].DeepClone());
	long tsrSz = testRecord.GetSize();

	// setup context
	Anything dummy;
	Context ctx(dummy, dummy, 0, 0, 0, 0);
	Anything tmpStore(ctx.GetTmpStore());
	const long arrSz = 10;
	long i, j;

	const char *mappername = "Mapper";
	ResultMapper mapper(mappername);
	mapper.Initialize("ResultMapper");

	for ( i = 0; i < arrSz; i++) {
		for ( j = 0; j < tsrSz; j++) {
			t_assert(mapper.Put(testRecord.SlotName(j), testRecord[j], ctx));
		}
	}
	TraceAny(tmpStore, "TmpStore");
	for (j = 0; j < tsrSz; j++) {
		for (i = 0; i < arrSz; i++) {
			switch (j) {
				case 0:
					assertEqual(10, tmpStore[mappername]["testInt"][i].AsLong());
					break;
				case 1:
					assertEqual(false, tmpStore[mappername]["testBool"][i].AsBool());
					break;
				case 2:
					assertEqual(123, tmpStore[mappername]["testLong"][i].AsLong());
					break;
				case 3:	{
					// direct comparison would fail on WIN32 ....
					float f1 = (float)tmpStore[mappername]["testFloat"][i].AsDouble();
					// it seems that a float conversion needs either some time or a context switch or who knows...
					if ( !t_assert((float)1.23F == f1) ) {
						Trace("returned value [" << f1 << "]");
					}
					break;
				}
				case 4:
					t_assert(2.46 == tmpStore[mappername]["testDouble"][i].AsDouble());
					break;
				case 5:
					assertEqual("foo baz", tmpStore[mappername]["testString"][i].AsCharPtr());
					break;
				default:
					t_assert(false);
			};
		}
	}
}

void MapperTest::GetConfigNameTest()
{
	String mapperName("testmapper");
	ParameterMapper inputMapper(mapperName);
	inputMapper.Initialize(ParameterMapper::gpcCategory);
	assertCharPtrEqual("InputMapperMeta", inputMapper.GetConfigName());
	ResultMapper outputMapper(mapperName);
	outputMapper.Initialize(ResultMapper::gpcCategory);
	assertCharPtrEqual("OutputMapperMeta", outputMapper.GetConfigName());
}

void MapperTest::DoLoadConfigTest()
{
	String mapperName("testmapper");
	String categoryName("ParameterMapper");
	ParameterMapper inputMapper(mapperName);

	t_assert( inputMapper.Initialize(categoryName) );
	assertEqual("foo", inputMapper.Lookup("testitem1").AsCharPtr());
	assertEqual("bah", inputMapper.Lookup("testitem2").AsCharPtr());

	ResultMapper outputMapper(mapperName);
	categoryName = "ResultMapper";

	t_assert( outputMapper.Initialize(categoryName) );
	assertEqual("foo", outputMapper.Lookup("testitem1").AsCharPtr());
	assertEqual("bah", outputMapper.Lookup("testitem2").AsCharPtr());
}

void MapperTest::LookupMapperGetTest()
{
	ParameterMapper m("lookupMapperTest");
	m.Initialize("ParameterMapper");
	ParameterMapper m1("anotherMapper");
	m1.Initialize("ParameterMapper");
	m1.Register("anotherMapper", "ParameterMapper");
	Context ctx;

	String str;
	{
		OStringStream os(&str);
		t_assert(m.Get("Input", os, ctx));
	}
	assertEqual("Foo", (const char *)str);

	m1.Unregister("anotherMapper", "ParameterMapper");
}

void MapperTest::RenameSlotWithConfigPutTest()
{
	/* config for RenameSlotWithConfigPutTest is
	   { /Kaspar { /Peter * }}
	 */
	ResultMapper m("RenameSlotWithConfigPutTest");
	m.Initialize("ResultMapper");
	Context ctx;
	t_assert(m.Put("Kaspar", 42L, ctx));
	assertEqual(42L, ctx.GetTmpStore()["Mapper"]["Peter"].AsLong(0));
}

void MapperTest::ScriptedPutTest()
{
	StartTrace(MapperTest.ScriptedPutTest);
	/* config for ScriptedPutTest is
	    { /Kaspar { /Mapper { /Peter * } { /Mapper { /Mike * }} } }
	 */
	EagerResultMapper m("ScriptedPutTest");
	m.Initialize("ResultMapper");
	Context ctx;
	t_assert(m.Put("Kaspar", 42L, ctx));
	TraceAny(ctx.GetTmpStore(), "tmpstore");
	assertEqual(42L, ctx.GetTmpStore()["Mapper"]["Peter"].AsLong(0));
	assertEqual(42L, ctx.GetTmpStore()["Mapper"]["Mike"].AsLong(1));
}

void MapperTest::RenameSlotWithConfigGetTest()
{
	/* config for RenameSlotWithConfigGetTest
	   { /Kaspar { /Peter * } }
	 */
	ParameterMapper m("RenameSlotWithConfigGetTest");
	m.Initialize("ParameterMapper");
	Context ctx;
	long value1, value2;
	ctx.GetTmpStore()["Kaspar"] = 42L;		// will never be accessed
	ctx.GetTmpStore()["Peter"] = 43L;
	t_assert(m.Get("Kaspar", value1, ctx));	// "redirected" get
	t_assert(m.Get("Peter", value2, ctx));	// fallback case
	assertEqual(43L, value1);
	assertEqual(43L, value2);
}
