/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _StrSpecialTest_h_
#define _StrSpecialTest_h_

#include "TestCase.h"

//---- StrSpecialTest -----------------------------------------------------------
//!some special tests for String
class StrSpecialTest : public TestFramework::TestCase
{
public:
	StrSpecialTest(TString tstrName);
	virtual ~StrSpecialTest();

	static Test *suite ();
	void setUp ();
	void simpleAppendTest();
	void umlauteTest();

protected:
	// utility
	void Dump(std::ostream &os, const Anything &data, const String &str);
};

#endif
