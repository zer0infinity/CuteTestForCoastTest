/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _DataAccessTest_H
#define _DataAccessTest_H

//---- TestCase include -------------------------------------------------
#include "TestCase.h"

//---- DataAccessTest ----------------------------------------------------------
//!single line description of the class
//! further explanation of the purpose of the class
//! this may contain <B>HTML-Tags</B>
//! ...
class DataAccessTest : public TestCase
{
public:
	//--- constructors
	DataAccessTest(TString name);
	~DataAccessTest();

	static Test *suite ();

	void setUp();

	void GetImplTest();
	void ExecTest();
	void SessionUnlockTest();

protected:
	//--- subclass api

	//--- member variables declaration
	Anything fConfig;
private:
	// use careful, you inhibit subclass use
	//--- private class api

	//--- private member variables

};

#endif
