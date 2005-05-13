/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

//--- c-library modules used ---------------------------------------------------
#include <stdlib.h>

//--- standard modules used ----------------------------------------------------
#include "Anything.h"
#include "StringStream.h"
#include "System.h"
#include "Context.h"

//--- test modules used --------------------------------------------------------
#include "TestSuite.h"

//--- interface include --------------------------------------------------------
#include "DataMapperTest.h"

//--- standard modules used ----------------------------------------------------
#include "DataMapper.h"

//---- DataMapperTest ----------------------------------------------------------------
Test *DataMapperTest::suite ()
// collect all test cases for the RegistryStream
{
	TestSuite *testSuite = new TestSuite;

	testSuite->addTest (NEW_CASE(DataMapperTest, StdPutTest));
	testSuite->addTest (NEW_CASE(DataMapperTest, StdGetTest));
	testSuite->addTest (NEW_CASE(DataMapperTest, ExtendedPutTest));
	testSuite->addTest (NEW_CASE(DataMapperTest, NegativGetTest));
	testSuite->addTest (NEW_CASE(DataMapperTest, FixedSizeTest));
	testSuite->addTest (NEW_CASE(DataMapperTest, UppercaseTest));

	return testSuite;

} // suite

DataMapperTest::DataMapperTest(TString tname) : TestCase(tname)
{
}

DataMapperTest::~DataMapperTest()
{
}

void DataMapperTest::setUp ()
// setup connector for this TestCase
{
	iostream *Ios = System::OpenStream("StdContext", "any");
	if ( Ios ) {
		fStdContextAny.Import((*Ios));
		delete Ios;
	}

} // setUp

void DataMapperTest::StdGetTest()
{
	Anything dummy;
	Context ctx(fStdContextAny, dummy, 0, 0, 0, 0);

	ParameterMapper mapper("stdtestgetmapper");

	// test the overloaded get api
	int iTestVal;
	t_assert(mapper.Get("testInt", iTestVal, ctx));
	assertEqual(10, iTestVal);

	bool bTestVal;
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
	t_assert(aTestVal.IsDefined("foo"));
	assertEqual("baz", aTestVal["foo"].AsCharPtr());

}

void DataMapperTest::NegativGetTest()
{
	Anything dummy;
	Context ctx(fStdContextAny, dummy, 0, 0, 0, 0);

	ParameterMapper mapper("stdtestgetmapper");

	// Test the overloaded get api
	int iTestVal;
	t_assert(!mapper.Get("TestInt", iTestVal, ctx));

	bool bTestVal;
	t_assert(!mapper.Get("TestBool", bTestVal, ctx));

	long lTestVal;
	t_assert(!mapper.Get("TestLong", lTestVal, ctx));

	float fTestVal;
	t_assert(!mapper.Get("TestFloat", fTestVal, ctx));

	double dTestVal;
	t_assert(!mapper.Get("TestDouble", dTestVal, ctx));

	String sTestVal;
	t_assert(!mapper.Get("TestString", sTestVal, ctx));

	Anything aTestVal;
	t_assert(!mapper.Get("TestAny", aTestVal, ctx));
	t_assert(!aTestVal.IsDefined("foo"));

	OStringStream os;
	t_assert(!mapper.DoGetStream("noStdStreams", os, ctx, Anything()));
}

void DataMapperTest::StdPutTest()
{
	Anything dummy;
	Context ctx(dummy, dummy, 0, 0, 0, 0);
	Anything tmpStore(ctx.GetTmpStore());

	const char *mappername = "stdtestputmapper";
	ListDataMapper mapper(mappername);

	// test the overloaded get api
	int iTestVal = 10;
	t_assert(mapper.Put("testInt", iTestVal, ctx));
	assertEqual(10, tmpStore[mappername][0L]["testInt"].AsLong());

	bool bTestVal = true;
	t_assert(mapper.Put("testBool", bTestVal, ctx));
	assertEqual(true, tmpStore[mappername][0L]["testBool"].AsBool());

	long lTestVal = 123;
	t_assert(mapper.Put("testLong", lTestVal, ctx));
	assertEqual(123, tmpStore[mappername][0L]["testLong"].AsLong());

	float fTestVal = 1.23F;
	t_assert(mapper.Put("testFloat", fTestVal, ctx));
	t_assert(1.23F == (float)tmpStore[mappername][0L]["testFloat"].AsDouble());

	double dTestVal = 2.46;
	t_assert(mapper.Put("testDouble", dTestVal, ctx));
	t_assert(2.46 == tmpStore[mappername][0L]["testDouble"].AsDouble());

	String sTestVal = "foo baz";
	t_assert(mapper.Put("testString", sTestVal, ctx));
	assertEqual("foo baz", tmpStore[mappername][0L]["testString"].AsCharPtr());

	Anything aTestVal;
	aTestVal["foo"] = "baz";
	t_assert(mapper.Put("testAny", aTestVal, ctx));
	t_assert(tmpStore[mappername][0L]["testAny"].IsDefined("foo"));
	assertEqual("baz", tmpStore[mappername][0L]["testAny"]["foo"].AsCharPtr());

	String test;
	IStringStream is(&test);
	t_assert(mapper.DoPutStream("StdStreams", is, ctx, Anything()));
}

void DataMapperTest::ExtendedPutTest()
{
	Anything testRecord(fStdContextAny["ArrayTestConfig"]);
	long tsrSz = testRecord.GetSize();

	// setup context
	Anything dummy;
	Context ctx(dummy, dummy, 0, 0, 0, 0);
	Anything tmpStore(ctx.GetTmpStore());
	const long arrSz = 10;
	long i, j;

	const char *mappername = "extendedputtestmapper";
	ListDataMapper mapper(mappername);

	for ( i = 0; i < arrSz; i++) {

		for ( j = 0; j < tsrSz; j++) {
			if ( j == 0 ) {
				tmpStore["IsNewRow"] = true;
			}
			t_assert(mapper.Put(testRecord.SlotName(j), testRecord[j], ctx));
			if ( j == 0 ) {
				tmpStore["IsNewRow"] = false;
			}
		}
	}

	for (i = 0; i < arrSz; i++) {
		assertEqual(10, tmpStore[mappername][i]["testInt"].AsLong());
		assertEqual(false, tmpStore[mappername][i]["testBool"].AsBool());
		assertEqual(123, tmpStore[mappername][i]["testLong"].AsLong());
		// direct comparison would fail on WIN32 ...
		float f1 = (float)tmpStore[mappername][i]["testFloat"].AsDouble();
		// it seems that a float conversion needs either some time or a context switch or who knows...
		String jummy;
		t_assert(1.23F == f1);
		t_assert(2.46 == tmpStore[mappername][i]["testDouble"].AsDouble());
		assertEqual("foo baz", tmpStore[mappername][i]["testString"].AsCharPtr());

	}
}

void DataMapperTest::FixedSizeTest()
{
	Anything lookupInput;
	lookupInput["Find"]["Key"]["Here"] = "Result";

	Context ctx(lookupInput, lookupInput, 0, 0, 0, 0);

	String result("<");
	OStringStream os(&result);
	t_assert(FixedSizeMapper("noConfig").Get("Find.Key.Here", os, ctx));
	os << ">" << flush;

	assertEqual("<Result              >", result);
}

void DataMapperTest::UppercaseTest()
{
	Anything lookupInput;
	lookupInput["Find"]["Key"]["Here"] = "Result";

	Context ctx(lookupInput, lookupInput, 0, 0, 0, 0);

	String result("<");
	OStringStream os(&result);
	t_assert(UpperCaseMapper("noConfig").Get("Find.Key.Here", os, ctx));
	os << ">" << flush;

	assertEqual("<RESULT>", result);
}
