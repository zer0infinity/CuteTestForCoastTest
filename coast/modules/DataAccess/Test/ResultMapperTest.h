/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _ResultMapperTest_H
#define _ResultMapperTest_H

//---- baseclass include -------------------------------------------------
#include "WDBaseTestPolicies.h"
#include "Mapper.h"

class PathTestMapper : public ResultMapper
{
public:
	PathTestMapper(const char *name): ResultMapper(name) {}
	virtual ~PathTestMapper() {}
	IFAObject *Clone() const {
		return new PathTestMapper(fName);
	}

protected:
	String DoGetDestinationSlot(Context &ctx, const char *pcDefault) {
		return "Mapper.x.y.z";
	}

	friend class ResultMapperTest;
};

//---- ResultMapperTest ----------------------------------------------------------
//! <B>Tests functionality of ResultMapper and EagerResultMapper.</B>
/*!
To understand the results of those tests, you should additionally
consult "OutputMapperMeta.any" and "MapperTestScripts.any".
*/
class ResultMapperTest : public TestFramework::TestCaseWithGlobalConfigDllAndModuleLoading
{
public:
	//--- constructors

	/*! \param name name of the test */
	ResultMapperTest(TString tstrName);

	//! destroys the test case
	~ResultMapperTest();

	//--- public api

	//! builds up a suite of testcases for this test
	static Test *suite ();

	void DoSelectScriptTest();
	void DoLoadConfigTest();
	void DoGetConfigNameTest();
	void DoFinalPutAnyTest();
	void DoFinalPutStreamTest();
	void DoPutAnyTest();
	void DoPutStreamTest();
	void PutTest();
	void DoSetDestinationSlotDynamicallyTest();
	void DoGetDestinationSlotWithPathTest();

	void EagerDoSelectScriptTest();
	void EagerPutTest();
};

#endif
