/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#include "HTTPChunkedOStreamTest.h"
#include "HTTPChunkedOStream.h"
#include "TestSuite.h"
#include "Tracer.h"

//---- HTTPChunkedOStreamTest ----------------------------------------------------------------
HTTPChunkedOStreamTest::HTTPChunkedOStreamTest(TString tstrName) : TestCaseType(tstrName)
{
	StartTrace(HTTPChunkedOStreamTest.Ctor);
}

HTTPChunkedOStreamTest::~HTTPChunkedOStreamTest()
{
	StartTrace(HTTPChunkedOStreamTest.Dtor);
}

void HTTPChunkedOStreamTest::OverflowTest()
{
	StartTrace(HTTPChunkedOStreamTest.OverflowTest);
	StringStream s;
	HTTPChunkedOStream os(s, 10);
	os << "0123456789ABCDEF";
	s << std::flush;
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
	os << "foo" << std::flush;
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
	os << "foo" << std::flush;
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
	s << 10 << std::flush;
	t_assert(os.good());
	assertCharPtrEqual("3\r\nfoo\r\n0\r\n\r\n10", s.str());
}

// builds up a suite of testcases, add a line for each testmethod
Test *HTTPChunkedOStreamTest::suite ()
{
	StartTrace(HTTPChunkedOStreamTest.suite);
	TestSuite *testSuite = new TestSuite;

	ADD_CASE(testSuite, HTTPChunkedOStreamTest, SimpleFlush);
	ADD_CASE(testSuite, HTTPChunkedOStreamTest, SimpleWithClose);
	ADD_CASE(testSuite, HTTPChunkedOStreamTest, SimpleCloseNoFlush);
	ADD_CASE(testSuite, HTTPChunkedOStreamTest, OverflowTest);
	ADD_CASE(testSuite, HTTPChunkedOStreamTest, HexManipulator);

	return testSuite;
}
