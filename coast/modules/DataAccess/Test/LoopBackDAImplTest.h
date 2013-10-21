/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _LoopBackDAImplTest_H
#define _LoopBackDAImplTest_H

#include "TestCase.h"
#include "Anything.h"

//---- LoopBackDAImplTest ----------------------------------------------------------
//!testcases for LoopBackDAImpl
class LoopBackDAImplTest : public testframework::TestCase
{
public:
	//--- constructors
	LoopBackDAImplTest(TString tstrName);
	~LoopBackDAImplTest();

	static Test *suite ();

	void setUp();

	//--- public api
	void ExecTest();

protected:
	//--- member variables declaration
	Anything fStdContextAny;
};

#endif
