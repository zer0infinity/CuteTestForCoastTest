/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _SS1Test_h_
#define _SS1Test_h_

#include "TestCase.h"

class SS1Test : public TestFramework::TestCase
{
public:
	SS1Test(TString tstrName);
	virtual ~SS1Test();

	static Test *suite ();

	void SimpleTest();
};

#endif
