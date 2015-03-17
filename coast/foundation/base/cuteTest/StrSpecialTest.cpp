/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * Copyright (c) 2015, David Tran, Faculty of Computer Science,
 * University of Applied Sciences Rapperswil (HSR),
 * 8640 Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#include "StringStream.h"
#include "SystemFile.h"
#include "StrSpecialTest.h"

using namespace coast;

void StrSpecialTest::simpleAppendTest() {
	String s("Hallo");
	OStringStream os(&s);
	os << 5L << " = " << 2L << '+' << 3L;
	os.flush();
	ASSERT_EQUAL("Hallo5 = 2+3", s);
}

void StrSpecialTest::umlauteTest() {
	// standard query case
	Anything test;
	std::istream *is = system::OpenStream("UmlautTest", "any");
	ASSERT(is != 0);

	if (is) {
		test.Import(*is);
		String testString(test["fields"][0L].AsString());
		ASSERT_EQUAL("Ausführen", testString.SubString(0, 10));

		delete is;
	}
}

void StrSpecialTest::runAllTests(cute::suite &s) {
	s.push_back(CUTE_SMEMFUN(StrSpecialTest, simpleAppendTest));
	s.push_back(CUTE_SMEMFUN(StrSpecialTest, umlauteTest));
}
