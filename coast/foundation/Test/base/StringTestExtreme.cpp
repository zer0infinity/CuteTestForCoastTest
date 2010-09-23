/*
 * Copyright (c) 2008, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

//--- interface include --------------------------------------------------------
#include "StringTestExtreme.h"

//--- test modules used --------------------------------------------------------
#include "TestSuite.h"

//--- standard modules used ----------------------------------------------------
#include "SystemFile.h"
#include "StringStream.h"

using namespace Coast;

//--- c-library modules used ---------------------------------------------------
#include <ctype.h>
#include <string.h>
#include <climits>
#include <float.h>

StringTestExtreme::StringTestExtreme (TString tname)
	: TestCaseType(tname)
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

	iostream *is = System::OpenStream("longString", "txt", ios::in);
	while (is && !is->eof()) {
		if (is->read(&s, 1)) {
			extremelyLongString.Append(s);
			trueString.append(1, s);
		}
	}
}

Test *StringTestExtreme::suite ()
{
	TestSuite *testSuite = new TestSuite;
	ADD_CASE(testSuite, StringTestExtreme, extremeLength);
	return testSuite;
}

void StringTestExtreme::extremeLength ()
{
	// Init a string without parameters
	t_assert(extremelyLongString.Length()>0);
	assertEqual(trueString.length(), extremelyLongString.Length());
}
