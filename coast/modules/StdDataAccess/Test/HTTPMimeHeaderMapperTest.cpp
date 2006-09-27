/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

//--- interface include --------------------------------------------------------
#include "HTTPMimeHeaderMapperTest.h"

//--- module under test --------------------------------------------------------
#include "HTTPMimeHeaderMapper.h"
//--- test modules used --------------------------------------------------------
#include "TestSuite.h"

//--- standard modules used ----------------------------------------------------
#include "Dbg.h"
#include "StringStream.h"

//--- c-modules used -----------------------------------------------------------

//---- HTTPMimeHeaderMapperTest ----------------------------------------------------------------
HTTPMimeHeaderMapperTest::HTTPMimeHeaderMapperTest(TString tstrName)
	: TestCaseType(tstrName)
{
	StartTrace(HTTPMimeHeaderMapperTest.HTTPMimeHeaderMapperTest);
}

TString HTTPMimeHeaderMapperTest::getConfigFileName()
{
	return "HTTPMimeHeaderMapperTestConfig";
}

HTTPMimeHeaderMapperTest::~HTTPMimeHeaderMapperTest()
{
	StartTrace(HTTPMimeHeaderMapperTest.Dtor);
}

void HTTPMimeHeaderMapperTest::SimpleHeader()
{
	StartTrace(HTTPMimeHeaderMapperTest.SimpleHeader);
	IStringStream is(GetConfig()["simpleHeader"].AsCharPtr());
	HTTPMimeHeaderMapper m("HTTPMimeHeaderMapper");
	m.Initialize("ResultMapper");
	Context ctx;
	t_assert(((ResultMapper &)m).Put("", is, ctx)); // key not necessary
	assertAnyEqual(GetConfig()["simpleHeaderResult"], ctx.GetTmpStore()["Mapper"]["HTTPHeader"]);
}

void HTTPMimeHeaderMapperTest::FieldsOccursMoreThanOnceHeaderTest()
{
	StartTrace(HTTPMimeHeaderMapperTest.FieldsOccursMoreThanOnceHeaderTest);
	IStringStream is(GetConfig()["fieldsOccursMoreThanOnceHeader"].AsCharPtr());
	HTTPMimeHeaderMapper m("HTTPMimeHeaderMapper");
	m.Initialize("ResultMapper");
	Context ctx;
	t_assert(((ResultMapper &)m).Put("", is, ctx)); // key not necessary
	assertAnyEqual(GetConfig()["fieldsOccursMoreThanOnceHeaderResult"], ctx.GetTmpStore()["Mapper"]["HTTPHeader"]);
}

void HTTPMimeHeaderMapperTest::FieldsOccursMoreThanOnceHeaderTestDoSplit()
{
	StartTrace(HTTPMimeHeaderMapperTest.FieldsOccursMoreThanOnceHeaderTest);
	IStringStream is(GetConfig()["fieldsOccursMoreThanOnceHeaderDoSplit"].AsCharPtr());
	HTTPMimeHeaderMapper m("HTTPMimeHeaderMapperDoSplitHeaderFields");
	m.Initialize("ResultMapper");
	Context ctx;
	t_assert(((ResultMapper &)m).Put("", is, ctx)); // key not necessary
	assertAnyEqual(GetConfig()["fieldsOccursMoreThanOnceHeaderDoSplitResult"], ctx.GetTmpStore()["Mapper"]["HTTPHeader"]);
}

void HTTPMimeHeaderMapperTest::CorrectedDateHeadersTest()
{
	StartTrace(HTTPMimeHeaderMapperTest.CorrectedDateHeadersTest);
	Anything dateformats2correct(GetConfig()["DateHeaderFullyParsed"].DeepClone());
	HTTPMimeHeaderMapper::CorrectDateFormats(dateformats2correct);
	assertAnyEqual(GetConfig()["DateHeaderOk"], dateformats2correct);
}
void HTTPMimeHeaderMapperTest::SuppressedHeadersTest()
{
	StartTrace(HTTPMimeHeaderMapperTest.SuppressedHeadersTest);
	IStringStream is(GetConfig()["simpleHeader"].AsCharPtr());
	HTTPMimeHeaderMapper m("HTTPMimeHeaderWithSuppress");
	t_assert(m.Initialize("ResultMapper"));
	Context ctx;
	t_assert(((ResultMapper &)m).Put("", is, ctx)); // key not necessary
	assertAnyEqual(GetConfig()["suppressedHeaderResult"], ctx.GetTmpStore()["Mapper"]["HTTPHeader"]);
}
void HTTPMimeHeaderMapperTest::LiteralSuppressedHeadersTest()
{
	StartTrace(HTTPMimeHeaderMapperTest.LiteralSuppressedHeadersTest);
	IStringStream is(GetConfig()["simpleHeader"].AsCharPtr());
	HTTPMimeHeaderMapper m("HTTPMimeHeader");
	t_assert(m.Initialize("ResultMapper"));
	ROAnything config(GetConfig()["literalSuppressedHeader"]);
	Context ctx;
	t_assert(((ResultMapper &)m).DoPutStream("", is, ctx, config)); // key not necessary
	assertAnyEqual(GetConfig()["suppressedHeaderResult"], ctx.GetTmpStore()["Mapper"]["HTTPHeader"]);
}
void HTTPMimeHeaderMapperTest::AddHeadersTest()
{
	StartTrace(HTTPMimeHeaderMapperTest.AddHeadersTest);
	IStringStream is(GetConfig()["simpleHeader"].AsCharPtr());
	HTTPMimeHeaderMapper m("HTTPMimeHeaderWithAdd");
	t_assert(m.Initialize("ResultMapper"));
	Context ctx;
	t_assert(((ResultMapper &)m).Put("", is, ctx)); // key not necessary
	assertAnyEqual(GetConfig()["addResult"], ctx.GetTmpStore()["Mapper"]["HTTPHeader"]);
}
void HTTPMimeHeaderMapperTest::SuppressAndAddHeadersTest()
{
	StartTrace(HTTPMimeHeaderMapperTest.AddHeadersTest);
	{
		IStringStream is(GetConfig()["simpleHeader"].AsCharPtr());
		HTTPMimeHeaderMapper m("HTTPMimeHeaderWithSuppressAndAdd");
		t_assert(m.Initialize("ResultMapper"));
		Context ctx;
		t_assert(((ResultMapper &)m).Put("", is, ctx)); // key not necessary
		assertAnyEqual(GetConfig()["suppressAndAddResult"], ctx.GetTmpStore()["Mapper"]["HTTPHeader"]);
	}
	{
		IStringStream is(GetConfig()["simpleHeader"].AsCharPtr());
		HTTPMimeHeaderMapper m("HTTPMimeHeaderWithSuppressAndAddNoHeaderFieldSplit");
		t_assert(m.Initialize("ResultMapper"));
		Context ctx;
		t_assert(((ResultMapper &)m).Put("", is, ctx)); // key not necessary
		assertAnyEqual(GetConfig()["suppressAndAddResult"], ctx.GetTmpStore()["Mapper"]["HTTPHeader"]);
	}
}

// builds up a suite of tests, add a line for each testmethod
Test *HTTPMimeHeaderMapperTest::suite ()
{
	StartTrace(HTTPMimeHeaderMapperTest.suite);
	TestSuite *testSuite = new TestSuite;

	ADD_CASE(testSuite, HTTPMimeHeaderMapperTest, SimpleHeader);
	ADD_CASE(testSuite, HTTPMimeHeaderMapperTest, CorrectedDateHeadersTest);
	ADD_CASE(testSuite, HTTPMimeHeaderMapperTest, SuppressedHeadersTest);
	ADD_CASE(testSuite, HTTPMimeHeaderMapperTest, AddHeadersTest);
	ADD_CASE(testSuite, HTTPMimeHeaderMapperTest, SuppressAndAddHeadersTest);
	ADD_CASE(testSuite, HTTPMimeHeaderMapperTest, LiteralSuppressedHeadersTest);
	ADD_CASE(testSuite, HTTPMimeHeaderMapperTest, FieldsOccursMoreThanOnceHeaderTest);
	ADD_CASE(testSuite, HTTPMimeHeaderMapperTest, FieldsOccursMoreThanOnceHeaderTestDoSplit);
	return testSuite;
}
