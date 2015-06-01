/*
 * Copyright (c) 2015, David Tran, Faculty of Computer Science,
 * University of Applied Sciences Rapperswil (HSR),
 * 8640 Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#include "cute_case.h"
#include "AnyUtilsTest.h"
#include "GenericXMLParserTest.h"
#include "AnySorterTest.h"

const char * setupSuite(cute::suite &s) {
	AnyUtilsTest::runAllTests(s);
	GenericXMLParserTest::runAllTests(s);
	AnySorterTest::runAllTests(s);
	return "CuteFoundationAnythingOptionalTest";
}
