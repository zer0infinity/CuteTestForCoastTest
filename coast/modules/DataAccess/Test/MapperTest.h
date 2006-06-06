/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _MapperTest_H
#define _MapperTest_H

//---- baseclass include -------------------------------------------------
#include "TestCase.h"

//---- forward declaration -----------------------------------------------
class Context;

//---- MapperTest ----------------------------------------------------------
//!single line description of the class
//! further explanation of the purpose of the class
//! this may contain <B>HTML-Tags</B>
//! ...
class MapperTest : public TestFramework::TestCase
{
public:
	//--- constructors
	MapperTest(TString tstrName);
	~MapperTest();

	static Test *suite ();

	void setUp();

	//--- public api
	void GetTests();
	void StdGetTest();
	void StdGetNoDataTest();
	void StdPutTest();
	void ExtendedPutTest();
	void GetConfigNameTest();
	void DoLoadConfigTest();

	void HardConfiguredGet();
	void MixedConfiguredGet();

	void LookupMapperGetTest();

	void MapperWithCfgAndCtx();

	//! new tests for scripting/config mechanics
	void RenameSlotWithConfigPutTest();
	void ScriptedPutTest();
	void RenameSlotWithConfigGetTest();

protected:
	//--- member variables declaration
	Anything fStdContextAny;
};

#endif
