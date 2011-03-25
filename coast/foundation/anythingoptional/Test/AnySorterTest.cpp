/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#include "AnySorterTest.h"
#include "TestSuite.h"
#include "AnySorter.h"

//---- AnySorterTest ---------------------------------------------------------
AnySorterTest::AnySorterTest (TString tname)
	: TestCaseType(tname)
{
}

AnySorterTest::~AnySorterTest()
{
	StartTrace(AnySorterTest.Dtor);
}

void AnySorterTest::SorterTest()
{
	StartTrace(AnySorterTest.SorterTest);
	ROAnything cConfig;
	AnyExtensions::Iterator<ROAnything> aEntryIterator(GetTestCaseConfig());
	while ( aEntryIterator.Next(cConfig) ) {
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
	ADD_CASE(testSuite, AnySorterTest, SorterTest);
	return testSuite;
}
