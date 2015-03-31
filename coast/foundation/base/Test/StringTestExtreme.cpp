/*
 * Copyright (c) 2008, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#include "StringTestExtreme.h"
#include "TestSuite.h"
#include "SystemFile.h"

using namespace coast;

void StringTestExtreme::setUp() {
	char s;
	extremelyLongString = "";
	trueString = "";

	std::iostream *is = system::OpenStream("longString", "txt", std::ios::in);
	while (is && !is->eof()) {
		if (is->read(&s, 1)) {
			extremelyLongString.Append(s);
			trueString.append(1, s);
		}
	}
	delete is;
}

Test *StringTestExtreme::suite() {
	TestSuite *testSuite = new TestSuite;
	ADD_CASE(testSuite, StringTestExtreme, extremeLength);
	return testSuite;
}

void StringTestExtreme::extremeLength() {
	// Init a string without parameters
	t_assert(extremelyLongString.Length()>0);
	assertEqual(trueString.length(), extremelyLongString.Length());
}
