/*
 * Copyright (c) 2008, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

//--- interface include --------------------------------------------------------
#include "StringTestExtreme.h"

//--- c-library modules used ---------------------------------------------------
#include <ctype.h>
#include <string.h>
#include <climits>
#include <float.h>

//--- standard modules used ----------------------------------------------------
#include "System.h"
#include "StringStream.h"

//--- test modules used --------------------------------------------------------
#include "TestSuite.h"

StringTestExtreme::StringTestExtreme (TString tname) : TestCase(tname)
{
}

StringTestExtreme::~StringTestExtreme()
{
}

void StringTestExtreme::setUp ()
{
	char s;
	extremelyLongString = "";

	trueString = "";

	iostream *is = System::OpenStream("longString", "txt");
	while (!is->eof()) {
		if (is->read(&s, 1)) {
			extremelyLongString.Append(s);
			trueString.append(1, s);
		}
	}

}

Test *StringTestExtreme::worksuite ()
{
	TestSuite *testSuite = new TestSuite;
	return testSuite;
}

Test *StringTestExtreme::suite ()
{
	TestSuite *testSuite = new TestSuite;

	testSuite->addTest (NEW_CASE(StringTestExtreme, extremeLength));

	return testSuite;
}

void StringTestExtreme::extremeLength ()
{
	// Init a string without parameters
	assertEqual(trueString.length(), extremelyLongString.Length());
}
