/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#include "StringStream.h"
#include "SystemFile.h"
#include "TestSuite.h"
#include "StrSpecialTest.h"

using namespace Coast;

void StrSpecialTest::simpleAppendTest() {
	String s("Hallo");
	OStringStream os(&s);
	os << 5L << " = " << 2L << '+' << 3L;
	os.flush();
	assertEqual("Hallo5 = 2+3", s);
}
void StrSpecialTest::umlauteTest() {
	// standard query case
	Anything test;
	std::istream *is = System::OpenStream("UmlautTest", "any");
	t_assert(is != 0);

	if (is) {
		test.Import(*is);
		String testString(test["fields"][0L].AsString());
		assertEqual("Ausführen", testString.SubString(0, 9));

		delete is;
	}
}

Test *StrSpecialTest::suite() {
	TestSuite *testSuite = new TestSuite;
	ADD_CASE(testSuite, StrSpecialTest, simpleAppendTest);
	ADD_CASE(testSuite, StrSpecialTest, umlauteTest);
	return testSuite;
}
