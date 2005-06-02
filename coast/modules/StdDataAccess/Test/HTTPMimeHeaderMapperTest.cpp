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
	: ConfiguredTestCase(tstrName, "HTTPMimeHeaderMapperTestConfig")
{
	StartTrace(HTTPMimeHeaderMapperTest.Ctor);
}

HTTPMimeHeaderMapperTest::~HTTPMimeHeaderMapperTest()
{
	StartTrace(HTTPMimeHeaderMapperTest.Dtor);
}

// setup for this ConfiguredTestCase
void HTTPMimeHeaderMapperTest::setUp ()
{
	StartTrace(HTTPMimeHeaderMapperTest.setUp);
	ConfiguredTestCase::setUp();
}

void HTTPMimeHeaderMapperTest::tearDown ()
{
	StartTrace(HTTPMimeHeaderMapperTest.tearDown);
	ConfiguredTestCase::tearDown();
}

void HTTPMimeHeaderMapperTest::SimpleHeader()
{
	StartTrace(HTTPMimeHeaderMapperTest.SimpleHeader);
	IStringStream is(fConfig["simpleHeader"].AsCharPtr());
	HTTPMimeHeaderMapper m("HTTPMimeHeaderMapper");
	m.CheckConfig("ResultMapper");
	Context ctx;
	t_assert(((ResultMapper &)m).Put("", is, ctx)); // key not necessary
	assertAnyEqual(fConfig["simpleHeaderResult"], ctx.GetTmpStore()["Mapper"]["HTTPHeader"]);
}

void HTTPMimeHeaderMapperTest::FieldsOccursMoreThanOnceHeaderTest()
{
	StartTrace(HTTPMimeHeaderMapperTest.FieldsOccursMoreThanOnceHeaderTest);
	IStringStream is(fConfig["fieldsOccursMoreThanOnceHeader"].AsCharPtr());
	HTTPMimeHeaderMapper m("HTTPMimeHeaderMapper");
	m.CheckConfig("ResultMapper");
	Context ctx;
	t_assert(((ResultMapper &)m).Put("", is, ctx)); // key not necessary
	assertAnyEqual(fConfig["fieldsOccursMoreThanOnceHeaderResult"], ctx.GetTmpStore()["Mapper"]["HTTPHeader"]);
}

void HTTPMimeHeaderMapperTest::CorrectedDateHeadersTest()
{
	StartTrace(HTTPMimeHeaderMapperTest.CorrectedDateHeadersTest);
	Anything dateformats2correct(fConfig["DateHeaderFullyParsed"]);
	HTTPMimeHeaderMapper::CorrectDateFormats(dateformats2correct);
	assertAnyEqual(fConfig["DateHeaderOk"], dateformats2correct);
}
void HTTPMimeHeaderMapperTest::SuppressedHeadersTest()
{
	StartTrace(HTTPMimeHeaderMapperTest.SuppressedHeadersTest);
	IStringStream is(fConfig["simpleHeader"].AsCharPtr());
	HTTPMimeHeaderMapper m("HTTPMimeHeaderWithSuppress");
	t_assert(m.CheckConfig("ResultMapper"));
	Context ctx;
	t_assert(((ResultMapper &)m).Put("", is, ctx)); // key not necessary
	assertAnyEqual(fConfig["suppressedHeaderResult"], ctx.GetTmpStore()["Mapper"]["HTTPHeader"]);
}
void HTTPMimeHeaderMapperTest::LiteralSuppressedHeadersTest()
{
	StartTrace(HTTPMimeHeaderMapperTest.LiteralSuppressedHeadersTest);
	IStringStream is(fConfig["simpleHeader"].AsCharPtr());
	HTTPMimeHeaderMapper m("HTTPMimeHeader");
	ROAnything config(fConfig["literalSuppressedHeader"]);
	Context ctx;
	t_assert(((ResultMapper &)m).DoPutStream("", is, ctx, config)); // key not necessary
	assertAnyEqual(fConfig["suppressedHeaderResult"], ctx.GetTmpStore()["Mapper"]["HTTPHeader"]);
}
void HTTPMimeHeaderMapperTest::AddHeadersTest()
{
	StartTrace(HTTPMimeHeaderMapperTest.AddHeadersTest);
	IStringStream is(fConfig["simpleHeader"].AsCharPtr());
	HTTPMimeHeaderMapper m("HTTPMimeHeaderWithAdd");
	t_assert(m.CheckConfig("ResultMapper"));
	Context ctx;
	t_assert(((ResultMapper &)m).Put("", is, ctx)); // key not necessary
	assertAnyEqual(fConfig["addResult"], ctx.GetTmpStore()["Mapper"]["HTTPHeader"]);
}
void HTTPMimeHeaderMapperTest::SuppressAndAddHeadersTest()
{
	StartTrace(HTTPMimeHeaderMapperTest.AddHeadersTest);
	{
		IStringStream is(fConfig["simpleHeader"].AsCharPtr());
		HTTPMimeHeaderMapper m("HTTPMimeHeaderWithSuppressAndAdd");
		t_assert(m.CheckConfig("ResultMapper"));
		Context ctx;
		t_assert(((ResultMapper &)m).Put("", is, ctx)); // key not necessary
		assertAnyEqual(fConfig["suppressAndAddResult"], ctx.GetTmpStore()["Mapper"]["HTTPHeader"]);
	}
	{
		IStringStream is(fConfig["simpleHeader"].AsCharPtr());
		HTTPMimeHeaderMapper m("HTTPMimeHeaderWithSuppressAndAddNoHeaderFieldSplit");
		t_assert(m.CheckConfig("ResultMapper"));
		Context ctx;
		t_assert(((ResultMapper &)m).Put("", is, ctx)); // key not necessary
		assertAnyEqual(fConfig["suppressAndAddResult"], ctx.GetTmpStore()["Mapper"]["HTTPHeader"]);
	}
}

// builds up a suite of ConfiguredTestCases, add a line for each testmethod
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
	return testSuite;
}
