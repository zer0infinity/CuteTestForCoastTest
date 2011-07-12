/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#include "HTTPStreamStackTest.h"
#include "TestSuite.h"
#include "HTTPStreamStack.h"
#include "Tracer.h"
#include "HTTPChunkedOStream.h"
#include "ZipStream.h"

//---- HTTPStreamStackTest ----------------------------------------------------------------
HTTPStreamStackTest::HTTPStreamStackTest(TString tstrName) : TestCaseType(tstrName)
{
	StartTrace(HTTPStreamStackTest.Ctor);
}

HTTPStreamStackTest::~HTTPStreamStackTest()
{
	StartTrace(HTTPStreamStackTest.Dtor);
}

//! Test with no special encoding
void HTTPStreamStackTest::NoEncoding()
{
	StartTrace(HTTPStreamStackTest.NoEncoding);

	StringStream out;
	{
		HTTPStreamStack stack(out, false, false);
		std::ostream &body = stack.GetBodyStream();
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
		std::ostream &body = stack.GetBodyStream();
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
		std::ostream &body = stack.GetBodyStream();
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
		std::ostream &body = stack.GetBodyStream();
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

// builds up a suite of testcases, add a line for each testmethod
Test *HTTPStreamStackTest::suite ()
{
	StartTrace(HTTPStreamStackTest.suite);
	TestSuite *testSuite = new TestSuite;
	ADD_CASE(testSuite, HTTPStreamStackTest, NoEncoding);
	ADD_CASE(testSuite, HTTPStreamStackTest, ChunkedOnlyEncoding);
	ADD_CASE(testSuite, HTTPStreamStackTest, GzipOnlyEncoding);
	ADD_CASE(testSuite, HTTPStreamStackTest, ChunkedGzipEncoding);
	return testSuite;
}
