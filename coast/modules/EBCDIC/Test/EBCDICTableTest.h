/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _EBCDICTableTest_h_
#define _EBCDICTableTest_h_

//-*-Mode: C++;-*-
/* Some dummy Test cases to start with */

#include "TestCase.h"

//---- EBCDICTableTest ---------------------------------------------------------

class EBCDICTableTest : public TestCase
{
public:
	EBCDICTableTest(TString name);
	virtual ~EBCDICTableTest();

	virtual void setUp ();
	static Test *suite ();
	void WholeTable ();
};

#endif
