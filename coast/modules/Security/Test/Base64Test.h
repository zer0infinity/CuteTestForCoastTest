/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _Base64Test_H
#define _Base64Test_H

#include "TestCase.h"

//---- Base64Test ----------------------------------------------------------
//!single line description of the class
//! further explanation of the purpose of the class
//! this may contain <B>HTML-Tags</B>
//! ...
class Base64Test : public TestFramework::TestCase
{
public:
	//--- constructors
	Base64Test(TString tstrName);
	~Base64Test();

	static Test *suite ();

	void EncodeDecodeTest();
	void EncodeDecodeOriginalTest();

protected:
	//--- subclass api
	int CalcEncodedLength( int Decodedlength );
};

#endif
