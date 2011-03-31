/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#include "Anything.h"
#include "StringStream.h"
#include "SystemFile.h"
#include "Context.h"
#include "Dbg.h"
#include "TestSuite.h"
#include "HTMLParser.h"
#include "HTMLParserTest.h"
#include "AnyUtils.h"

//---- HTMLParserTest ----------------------------------------------------------------
HTMLParserTest::HTMLParserTest(TString tstrName) : TestCaseType(tstrName)
{
	StartTrace(HTMLParserTest.Ctor);

}

HTMLParserTest::~HTMLParserTest()
{
	StartTrace(HTMLParserTest.Dtor);
}

void HTMLParserTest::ParseFileTest()
{
	StartTrace(HTMLParserTest.ParseFileTest);

	std::iostream *input = Coast::System::OpenIStream("eSport", "html");
	std::iostream *expectedResultInput = Coast::System::OpenStream("eSportResult", "any");

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
		assertAnyCompareEqual(expected, allReqs, "Expected", '.', ':');
	}
}

// builds up a suite of testcases, add a line for each testmethod
Test *HTMLParserTest::suite ()
{
	StartTrace(HTMLParserTest.suite);
	TestSuite *testSuite = new TestSuite;

	ADD_CASE(testSuite, HTMLParserTest, ParseFileTest);

	return testSuite;

}
