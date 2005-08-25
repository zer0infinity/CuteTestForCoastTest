/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

//--- interface include --------------------------------------------------------
#include "AnySorterTest.h"

//--- test modules used --------------------------------------------------------
#include "TestSuite.h"

//---module under test  --------------------------------------------------------
#include "AnySorter.h"

//--- standard modules used ----------------------------------------------------
#include "Dbg.h"
#include "System.h"

//--- c-library modules used ---------------------------------------------------

//---- AnySorterTest ---------------------------------------------------------

AnySorterTest::AnySorterTest (TString tname)
	: TestCase(tname)
{
}

AnySorterTest::~AnySorterTest()
{
	StartTrace(AnySorterTest.Dtor);
}

// setup for this TestCase
void AnySorterTest::setUp ()
{
	StartTrace(AnySorterTest.setUp);
	if ( t_assertm( System::LoadConfigFile(fConfig, getClassName(), "any"), TString("expected ") << getClassName() << " to be readable!" ) ) {
		fTestCaseConfig = fConfig[name()];
	}
}

void AnySorterTest::tearDown ()
{
	StartTrace(AnySorterTest.tearDown);
}

void AnySorterTest::testAnySorter()
{
	StartTrace(AnySorterTest.testAnySorter);
	ROAnything cConfig(fTestCaseConfig[0L]);
	for (long lIdx = 0; lIdx < fTestCaseConfig.GetSize(); lIdx++, cConfig = fTestCaseConfig[lIdx]) {
		TraceAny(cConfig, "the config");
		Anything sorted;
		AnySorter::EMode mode(cConfig["Mode"].AsString() == "asc" ? AnySorter::asc : AnySorter::desc);
		bool sortCritIsNumber(cConfig["SortCritIsNumber"].AsBool(0));
		sorted = cConfig["TestArray"].DeepClone();
		AnySorter::SortByKeyInArray(cConfig["SortKey"].AsString(), sorted, mode, sortCritIsNumber);
		assertAnyEqual(	cConfig["ExpectedResult"], sorted);
	}
}

Test *AnySorterTest::suite ()
{
	TestSuite *testSuite = new TestSuite;
	ADD_CASE(testSuite, AnySorterTest, testAnySorter);
	return testSuite;
}
