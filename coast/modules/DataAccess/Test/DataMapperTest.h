/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _DataMapperTest_H
#define _DataMapperTest_H

//---- TestCase include -------------------------------------------------
#include "TestCase.h"

//---- forward declaration -----------------------------------------------

//---- DataMapperTest ----------------------------------------------------------
//!single line description of the class
//! further explanation of the purpose of the class
//! this may contain <B>HTML-Tags</B>
//! ...
class DataMapperTest : public TestCase
{
public:
	//--- constructors
	DataMapperTest(TString name);
	~DataMapperTest();

	static Test *suite ();

	void setUp();

	//--- public api
	void StdPutTest();
	void StdGetTest();
	void ExtendedPutTest();
	void NegativGetTest();

	void FixedSizeTest();
	void UppercaseTest();

protected:
	//--- subclass api

	//--- member variables declaration
	Anything fStdContextAny;
private:
	// use careful, you inhibit subclass use
	//--- private class api

	//--- private member variables

};

#endif
