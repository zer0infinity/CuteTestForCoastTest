/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _StreamingAnythingMapperTest_H
#define _StreamingAnythingMapperTest_H

//---- baseclass include -------------------------------------------------
#include "TestCase.h"

//---- StreamingAnythingMapperTest ----------------------------------------------------------
//!TestClass for StreamingAnythingMapper functionality
//! this mapper should handle Anythings exported and imported to client streams properly
class StreamingAnythingMapperTest : public TestCase
{
public:

	//--- constructors
	StreamingAnythingMapperTest(TString name);
	~StreamingAnythingMapperTest();

	//--- public api
	//!generate test suite
	//! this method generates the tests for the class
	//! \return a new test is created by this method
	static Test *suite ();

	//!set up for tests does nothing so far
	void setUp ();

	//!delete set up for tests does nothing so far
	void tearDown ();

	//! Stream an Anything from context on client's stream
	void GetTest();
	//! Reads an Anything form client's stream and stores it in context
	void PutTest();

private:
	// use careful, you inhibit subclass use
	//--- private class api

	//--- private member variables

};

#endif
