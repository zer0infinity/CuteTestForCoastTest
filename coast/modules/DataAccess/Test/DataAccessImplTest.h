/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _DataAccessImplTest_H
#define _DataAccessImplTest_H

//---- TestCase include -------------------------------------------------
#include "TestCase.h"

//---- forward declaration -----------------------------------------------

//---- DataAccessImplTest ----------------------------------------------------------
//!single line description of the class
//! further explanation of the purpose of the class
//! this may contain <B>HTML-Tags</B>
//! ...
class DataAccessImplTest : public TestCase
{
public:
	//--- constructors
	DataAccessImplTest(TString name);
	~DataAccessImplTest();

	static Test *suite ();

	void setUp();

	//--- public api
	void GetConfigNameTest();
	void DoLoadConfigTest();
	void GetInputMapperTest();
	void GetOutputMapperTest();
};

#endif
