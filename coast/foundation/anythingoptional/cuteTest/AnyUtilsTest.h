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

#ifndef _AnyUtilsTest_h_
#define _AnyUtilsTest_h_

#include "cute.h"
#include "Anything.h"//lint !e537
#include "Tracer.h"
#include "SystemFile.h"
#include "AssertionAnything.h"

class AnyUtilsTest : public Assertion {
public:
	static void runAllTests(cute::suite &s);
	void CompareTest();
	void printEmptyXmlTest();
	void printSimpleXmlTest();
	void printSequenceXmlTest();
	void printHashXmlTest();
	void printMixedXmlTest();
	void MergeTest();

protected:
	bool DoXMLTest(const char *expect, ROAnything foroutput);
	// utility
	void DoCheck(Anything testCases, bool expectedResult, String description);
};

#endif
