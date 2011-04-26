/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#include "HTMLParserTest.h"
#include "TestSuite.h"
#include "HTMLParser.h"
#include "AnyUtils.h"

void HTMLParserTest::ParseFileTest() {
	StartTrace(HTMLParserTest.ParseFileTest);

	std::iostream *input = Coast::System::OpenIStream("eSport", "html");
	std::iostream *expectedResultInput = Coast::System::OpenStream("eSportResult", "any");

	if (t_assertm(input != NULL, "expected eSport.html to be there")
			&& t_assertm(expectedResultInput != NULL, "expected eSportResult.any to be there")) {
		Anything expected;
		expected.Import(*expectedResultInput, "eSportResult.any");

		AAT_HTMLReader mr(input);
		Anything allReqs;
		MyHTMLWriter mw(allReqs);
		AAT_StdHTMLParser p(mr, mw);

		// now we may read the HTML-Tags
		p.IntParse();
		mw.Flush();

		t_assertm(!mw.GetReqFailed(), "expected parsing to succeed");
		assertAnyCompareEqual(expected, allReqs, "Expected", '.', ':');
	}
}

Test *HTMLParserTest::suite() {
	StartTrace(HTMLParserTest.suite);
	TestSuite *testSuite = new TestSuite;
	ADD_CASE(testSuite, HTMLParserTest, ParseFileTest);
	return testSuite;
}
