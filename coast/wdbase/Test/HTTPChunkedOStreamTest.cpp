/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

//--- c-modules used -----------------------------------------------------------

//--- standard modules used ----------------------------------------------------
#include "Anything.h"
#include "Dbg.h"

//--- test modules used --------------------------------------------------------
#include "TestSuite.h"

//--- module under test --------------------------------------------------------

//--- interface include --------------------------------------------------------
#include "HTTPChunkedOStreamTest.h"
#include "StringStream.h"
#include "HTTPChunkedOStream.h"

//---- HTTPChunkedOStreamTest ----------------------------------------------------------------
HTTPChunkedOStreamTest::HTTPChunkedOStreamTest(TString tstrName) : TestCase(tstrName)
{
	StartTrace(HTTPChunkedOStreamTest.Ctor);
}

HTTPChunkedOStreamTest::~HTTPChunkedOStreamTest()
{
	StartTrace(HTTPChunkedOStreamTest.Dtor);
}

// setup for this TestCase
void HTTPChunkedOStreamTest::setUp ()
{
	StartTrace(HTTPChunkedOStreamTest.setUp);
}

void HTTPChunkedOStreamTest::tearDown ()
{
	StartTrace(HTTPChunkedOStreamTest.tearDown);
}

void HTTPChunkedOStreamTest::testCase()
{
	StartTrace(HTTPChunkedOStreamTest.testCase);
}
void HTTPChunkedOStreamTest::OverflowTest()
{
	StartTrace(HTTPChunkedOStreamTest.OverflowTest);
	StringStream s;
	HTTPChunkedOStream os(s, 10);
	os << "0123456789ABCDEF";
	s << flush;
	assertCharPtrEqual("a\r\n0123456789\r\n", s.str());
	os.close();
	assertCharPtrEqual("a\r\n0123456789\r\n6\r\nABCDEF\r\n0\r\n\r\n", s.str());

}

void HTTPChunkedOStreamTest::SimpleCloseNoFlush()
{
	StartTrace(HTTPChunkedOStreamTest.SimpleCloseNoFlush);
	StringStream s;
	HTTPChunkedOStream os(s);
	os << "foo";
	t_assert(os.good());
	os.close();
	assertCharPtrEqual("3\r\nfoo\r\n0\r\n\r\n", s.str());
}
void HTTPChunkedOStreamTest::SimpleWithClose()
{
	StartTrace(HTTPChunkedOStreamTest.SimpleWithClose);
	StringStream s;
	HTTPChunkedOStream os(s);
	os << "foo" << flush;
	t_assert(os.good());
	assertCharPtrEqual("3\r\nfoo\r\n", s.str());
	os.close();
	assertCharPtrEqual("3\r\nfoo\r\n0\r\n\r\n", s.str());
}

void HTTPChunkedOStreamTest::SimpleFlush()
{
	StartTrace(HTTPChunkedOStreamTest.SimpleFlush);

	StringStream s;
	HTTPChunkedOStream os(s);
	os << "foo" << flush;
	t_assert(os.good());
	assertCharPtrEqual("3\r\nfoo\r\n", s.str());

}

void HTTPChunkedOStreamTest::HexManipulator()
{
	StartTrace(HTTPChunkedOStreamTest.SimpleFlush);

	StringStream s;
	HTTPChunkedOStream os(s);
	os << "foo" ;
	os.close();
	s << 10 << flush;
	t_assert(os.good());
	assertCharPtrEqual("3\r\nfoo\r\n0\r\n\r\n10", s.str());
}

// builds up a suite of testcases, add a line for each testmethod
Test *HTTPChunkedOStreamTest::suite ()
{
	StartTrace(HTTPChunkedOStreamTest.suite);
	TestSuite *testSuite = new TestSuite;

	testSuite->addTest (NEW_CASE(HTTPChunkedOStreamTest, SimpleFlush));
	testSuite->addTest (NEW_CASE(HTTPChunkedOStreamTest, SimpleWithClose));
	testSuite->addTest (NEW_CASE(HTTPChunkedOStreamTest, SimpleCloseNoFlush));
	testSuite->addTest (NEW_CASE(HTTPChunkedOStreamTest, OverflowTest));
	testSuite->addTest (NEW_CASE(HTTPChunkedOStreamTest, HexManipulator));

	return testSuite;
}
