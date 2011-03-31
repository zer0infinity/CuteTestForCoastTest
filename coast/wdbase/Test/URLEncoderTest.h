/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _URLEncoderTest_H
#define _URLEncoderTest_H

#include "TestCase.h"

//---- forward declaration -----------------------------------------------

//---- URLEncoderTest ----------------------------------------------------------
//!single line description of the class
//! further explanation of the purpose of the class
//! this may contain <B>HTML-Tags</B>
//! ...
class URLEncoderTest : public TestFramework::TestCase
{
public:
	//--- constructors
	URLEncoderTest(TString tstrName);
	~URLEncoderTest();

	static Test *suite ();

	void EncodeDecodeTest();
	void EncodeTest();
	void DecodeTest();
};

#endif
