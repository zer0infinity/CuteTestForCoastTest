/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _SS1Test_h_
#define _SS1Test_h_

/* Purpose: Test the interface of the StringStream
   Initial Autor: Peter
*/

#include "TestCase.h"

//---- SS1Test -----------------------------------------------------------
//!testcases for StringStream
class SS1Test : public TestCase
{
public:
	SS1Test(TString name);
	virtual ~SS1Test();

	virtual void setUp ();
	static Test *suite ();

	void SimpleTest();

};

#endif
