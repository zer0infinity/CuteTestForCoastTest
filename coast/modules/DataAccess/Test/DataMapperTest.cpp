/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#include "DataMapperTest.h"
#include "DataMapper.h"
#include "TestSuite.h"
#include "StringStream.h"

//---- DataMapperTest ----------------------------------------------------------------
Test *DataMapperTest::suite() {
	TestSuite *testSuite = new TestSuite;

	ADD_CASE(testSuite, DataMapperTest, StdGetTest);
	ADD_CASE(testSuite, DataMapperTest, NegativGetTest);
	ADD_CASE(testSuite, DataMapperTest, FixedSizeTest);
	ADD_CASE(testSuite, DataMapperTest, UppercaseTest);

	return testSuite;
}

DataMapperTest::DataMapperTest(TString tname) :
	TestCaseType(tname) {
}

DataMapperTest::~DataMapperTest() {
}

void DataMapperTest::StdGetTest() {
	Anything dummy, anyContext(GetConfig().DeepClone());
	Context ctx(anyContext, dummy, 0, 0, 0, 0);

	ParameterMapper mapper("stdtestgetmapper");
	mapper.Initialize("ParameterMapper");

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

void DataMapperTest::NegativGetTest() {
	Anything dummy, anyContext(GetConfig().DeepClone());
	Context ctx(anyContext, dummy, 0, 0, 0, 0);

	ParameterMapper mapper("stdtestgetmapper");
	mapper.Initialize("ParameterMapper");

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
	t_assert(!mapper.Get("noStdStreams", os, ctx));
}

void DataMapperTest::FixedSizeTest() {
	Anything lookupInput;
	lookupInput["Find"]["Key"]["Here"] = "Result";

	Context ctx(lookupInput, lookupInput, 0, 0, 0, 0);

	String result("<");
	OStringStream os(&result);
	t_assert(FixedSizeMapper("noConfig").Get("Find.Key.Here", os, ctx));
	os << ">" << std::flush;

	assertEqual("<Result              >", result);
}

void DataMapperTest::UppercaseTest() {
	Anything lookupInput;
	lookupInput["Find"]["Key"]["Here"] = "Result";

	Context ctx(lookupInput, lookupInput, 0, 0, 0, 0);

	String result("<");
	OStringStream os(&result);
	t_assert(UpperCaseMapper("noConfig").Get("Find.Key.Here", os, ctx));
	os << ">" << std::flush;

	assertEqual("<RESULT>", result);
}
