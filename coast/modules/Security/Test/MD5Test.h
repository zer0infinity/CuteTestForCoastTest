/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _MD5Test_h_
#define _MD5Test_h_

//-*-Mode: C++;-*-

/* Purpose: Test the interface of the MD5Signer
   Initial Autor: Patru
*/

#include "TestCase.h"

//---- MD5Test -----------------------------------------------------------

class MD5Test : public TestFramework::TestCase
{
public:
	MD5Test(TString tstrName);
	virtual ~MD5Test();

	static Test *suite ();
	//! check signing ability of MD5Signer
	void signCheck();
	//! test plain hash value calculation of MD5Signer
	void hashCheck();
};

#endif
