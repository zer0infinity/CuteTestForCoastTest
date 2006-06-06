/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _AnythingUtilsTest_h_
#define _AnythingUtilsTest_h_

#include "FoundationTestTypes.h"

//---- AnythingUtilsTest -----------------------------------------------------------
class AnythingUtilsTest : public TestFramework::TestCaseWithConfig
{
public:
	AnythingUtilsTest(TString tstrName);
	virtual ~AnythingUtilsTest();

	virtual TString getConfigFileName();
	virtual void setUp();

	static Test *suite();
	void StoreCopierTest();
	void StorePutterTest();
	void StorePutterReplaceTest();
	void StorePutterReplaceRenderedTest();
	void StorePutterEmptySlotTest();
	void StoreFinderRenderedTest();
	void StoreFinderTest();

protected:
	Anything	fQuery;
};

#endif
