/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

//--- interface include --------------------------------------------------------
#include "HTTPStreamStackTest.h"
//--- test modules used --------------------------------------------------------
#include "TestSuite.h"
//--- module under test --------------------------------------------------------
#include "HTTPStreamStack.h"

//--- standard modules used ----------------------------------------------------
#include "Anything.h"
#include "Dbg.h"

#include "StringStream.h"
#include "Context.h"
#include "HTTPChunkedOStream.h"
#include "ZipStream.h"
//--- c-modules used -----------------------------------------------------------

//---- HTTPStreamStackTest ----------------------------------------------------------------
HTTPStreamStackTest::HTTPStreamStackTest(TString name) : TestCase(name)
{
	StartTrace(HTTPStreamStackTest.Ctor);
}

HTTPStreamStackTest::~HTTPStreamStackTest()
{
	StartTrace(HTTPStreamStackTest.Dtor);
}

// setup for this TestCase
void HTTPStreamStackTest::setUp ()
{
	StartTrace(HTTPStreamStackTest.setUp);
}

void HTTPStreamStackTest::tearDown ()
{
	StartTrace(HTTPStreamStackTest.tearDown);
}

//! Test with no special encoding
void HTTPStreamStackTest::NoEncoding()
{
	StartTrace(HTTPStreamStackTest.NoEncoding);

	StringStream out;
	{
		HTTPStreamStack stack(out, false, false);
		ostream &body = stack.GetBodyStream();
		body << "TEST";
	}
	StringStream expected;
	expected << ENDL << "TEST";
	assertEqual(expected.str(), out.str());
}

//! Test with chunked encoding only
void HTTPStreamStackTest::ChunkedOnlyEncoding()
{
	StartTrace(HTTPStreamStackTest.CunkedOnlyEncoding);

	StringStream out;
	{
		HTTPStreamStack stack(out, true, false);
		ostream &body = stack.GetBodyStream();
		body << "TEST";
	}

	StringStream expected;
	expected << "Transfer-Encoding: chunked" << ENDL << ENDL;
	{
		HTTPChunkedOStream chunked(expected);
		chunked << "TEST";
	}

	assertEqual(expected.str(), out.str());
}

//! Test with gzip encoding only
void HTTPStreamStackTest::GzipOnlyEncoding()
{
	StartTrace(HTTPStreamStackTest.GzipOnlyEncoding);

	StringStream out;
	{
		HTTPStreamStack stack(out, false, true);
		ostream &body = stack.GetBodyStream();
		body << "TEST";
	}

	StringStream expected;
	expected << "Content-Encoding: gzip" << ENDL << ENDL;
	{
		ZipOStream zipped(expected);
		zipped << "TEST";
	}

	assertEqual(expected.str(), out.str());
}

//! Test with chunked and gzip encoding
void HTTPStreamStackTest::ChunkedGzipEncoding()
{
	StartTrace(HTTPStreamStackTest.CunkedGzipEncoding);

	StringStream out;
	{
		HTTPStreamStack stack(out, true, true);
		ostream &body = stack.GetBodyStream();
		body << "TEST";
	}

	StringStream expected;
	expected << "Transfer-Encoding: chunked" << ENDL;
	expected << "Content-Encoding: gzip" << ENDL << ENDL;
	{
		HTTPChunkedOStream chunked(expected);
		ZipOStream zipped(chunked);
		zipped << "TEST";
	}

	assertEqual(expected.str(), out.str());
}

void HTTPStreamStackTest::testCase()
{
	StartTrace(HTTPStreamStackTest.testCase);
}
// builds up a suite of testcases, add a line for each testmethod
Test *HTTPStreamStackTest::suite ()
{
	StartTrace(HTTPStreamStackTest.suite);
	TestSuite *testSuite = new TestSuite;

	testSuite->addTest (NEW_CASE(HTTPStreamStackTest, NoEncoding));
	testSuite->addTest (NEW_CASE(HTTPStreamStackTest, ChunkedOnlyEncoding));
	testSuite->addTest (NEW_CASE(HTTPStreamStackTest, GzipOnlyEncoding));
	testSuite->addTest (NEW_CASE(HTTPStreamStackTest, ChunkedGzipEncoding));
	testSuite->addTest (NEW_CASE(HTTPStreamStackTest, testCase));

	return testSuite;
}
