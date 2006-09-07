/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

//--- c-modules used -----------------------------------------------------------

//--- standard modules used ----------------------------------------------------
#include "Anything.h"
#include "Dbg.h"

//--- project modules used -----------------------------------------------------

//--- test modules used --------------------------------------------------------
#include "TestSuite.h"

//--- module under test --------------------------------------------------------
#include "AnyLookupInterfaceAdapter.h"

//--- interface include --------------------------------------------------------
#include "ROAnyLookupAdapterTest.h"

//---- ROAnyLookupAdapterTest ----------------------------------------------------------------
ROAnyLookupAdapterTest::ROAnyLookupAdapterTest(TString tstrName) : TestCaseType(tstrName)
{
	StartTrace(ROAnyLookupAdapterTest.Ctor);
}

ROAnyLookupAdapterTest::~ROAnyLookupAdapterTest()
{
	StartTrace(ROAnyLookupAdapterTest.Dtor);
}

void ROAnyLookupAdapterTest::LookupTest()
{
	StartTrace(ROAnyLookupAdapterTest.LookupTest);

	{
		Anything base;
		base["foo"] = "bar";
		base["long"] = 31416L;
		base["compo"]["site"] = "bar";

		AnyLookupInterfaceAdapter<Anything> la(base);

		assertEqual("bar", la.Lookup("foo", "X"));
		assertEqual("bar", la.Lookup("compo@site", "X", '@'));
		t_assert(31416L == la.Lookup("long", 0L));
		assertEqual("X", la.Lookup("fox", "X"));
	}
	{
		Anything base;
		base["foo"] = "bar";
		base["long"] = 31416L;
		base["compo"]["site"] = "bar";

		AnyLookupInterfaceAdapter<Anything> la(base, "BaseLevel");

		assertEqual("X", la.Lookup("foo", "X"));
		assertAnyEqual(base, la.Lookup("BaseLevel"));
		assertEqual("bar", la.Lookup("BaseLevel.foo", "X"));
		assertEqual("bar", la.Lookup("BaseLevel:0", "X"));
		assertEqual("X", la.Lookup("BaseLevel.foo.NotExisting", "X"));
		assertEqual("bar", la.Lookup("BaseLevel.compo.site", "X"));
	}
}

// builds up a suite of testcases, add a line for each testmethod
Test *ROAnyLookupAdapterTest::suite ()
{
	StartTrace(ROAnyLookupAdapterTest.suite);
	TestSuite *testSuite = new TestSuite;
	ADD_CASE(testSuite, ROAnyLookupAdapterTest, LookupTest);
	return testSuite;
}
