/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

//--- standard modules used ----------------------------------------------------
#include "Anything.h"
#include "StringStream.h"
#include "System.h"
#include "Context.h"
#include "Dbg.h"

//--- test modules used --------------------------------------------------------
#include "TestSuite.h"

//--- module under test --------------------------------------------------------
#include "HTMLParser.h"

//--- interface include --------------------------------------------------------
#include "HTMLParserTest.h"

//---- HTMLParserTest ----------------------------------------------------------------
HTMLParserTest::HTMLParserTest(TString name) : TestCase(name)
{
	StartTrace(HTMLParserTest.Ctor);

}

HTMLParserTest::~HTMLParserTest()
{
	StartTrace(HTMLParserTest.Dtor);
}

// setup for this TestCase
void HTMLParserTest::setUp ()
{
	StartTrace(HTMLParserTest.setUp);
} // setUp

void HTMLParserTest::tearDown ()
{
	StartTrace(HTMLParserTest.tearDown);
} // tearDown

void HTMLParserTest::ParseFileTest()
{
	StartTrace(HTMLParserTest.ParseFileTest);

	iostream *input = System::OpenIStream("eSport", "html");
	iostream *expectedResultInput = System::OpenIStream("eSportResult", "any");

	if (	t_assertm(input != NULL, "expected eSport.html to be there") &&
			t_assertm(expectedResultInput != NULL, "expected eSportResult.any to be there")
	   ) {
		Anything expected;
		expected.Import(*expectedResultInput, "eSportResult.any");

		AAT_HTMLReader mr(input);
		Anything allReqs;
		MyHTMLWriter mw(allReqs);
		AAT_StdHTMLParser p( mr, mw );

		// now we may read the HTML-Tags
		p.IntParse();
		mw.Flush();

		t_assertm(!mw.GetReqFailed(), "expected parsing to succeed");
		assertAnyEqual(expected, allReqs);
	}
}

// builds up a suite of testcases, add a line for each testmethod
Test *HTMLParserTest::suite ()
{
	StartTrace(HTMLParserTest.suite);
	TestSuite *testSuite = new TestSuite;

	testSuite->addTest (NEW_CASE(HTMLParserTest, ParseFileTest));

	return testSuite;

} // suite
