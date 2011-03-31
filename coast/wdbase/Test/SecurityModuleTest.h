/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _SecurityModuleTest_H
#define _SecurityModuleTest_H

#include "TestCase.h"

//---- SecurityModuleTest ----------------------------------------------------------
//!single line description of the class
//! further explanation of the purpose of the class
//! this may contain <B>HTML-Tags</B>
//! ...
class SecurityModuleTest : public TestFramework::TestCase
{
public:
	//--- constructors
	SecurityModuleTest(TString tstrName);
	~SecurityModuleTest();

	static Test *suite ();

	void EncodeDecodeTest();
	void InitWithConfigTest();
};

#endif
