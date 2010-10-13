/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

//--- interface include --------------------------------------------------------
#include "ROAnyConfNamedObjectLookupAdapterTest.h"

//--- module under test --------------------------------------------------------
#include "AnyLookupInterfaceAdapter.h"

//--- test modules used --------------------------------------------------------
#include "TestSuite.h"

//--- standard modules used ----------------------------------------------------
#include "Dbg.h"
#include <iostream>

//--- c-modules used -----------------------------------------------------------

RegCacheImpl(TestConfNamedObj);				// FindTestConfNamedObj()
RegisterTestConfNamedObj(TestConfNamedObj);

//---- ROAnyConfNamedObjectLookupAdapterTest ----------------------------------------------------------------
ROAnyConfNamedObjectLookupAdapterTest::ROAnyConfNamedObjectLookupAdapterTest(TString tstrName) : TestCaseType(tstrName)
{
	StartTrace(ROAnyConfNamedObjectLookupAdapterTest.Ctor);
}

ROAnyConfNamedObjectLookupAdapterTest::~ROAnyConfNamedObjectLookupAdapterTest()
{
	StartTrace(ROAnyConfNamedObjectLookupAdapterTest.Dtor);
}

void ROAnyConfNamedObjectLookupAdapterTest::LookupTest()
{
	StartTrace(ROAnyConfNamedObjectLookupAdapterTest.LookupTest);

	Anything base;
	TestConfNamedObj *tcfno = TestConfNamedObj::FindTestConfNamedObj("TestConfNamedObj");
	t_assertm(tcfno != NULL, "Ooops, Expected TestConfNamedObj pointer not to be null!");

	tcfno->Initialize("TestConfNamedObj");

	base["hank"] = "bar";
	base["long"] = 31416L;
	base["compo"]["site"] = "bar";

	ROAnything robase(base);
	ROAnyConfNamedObjectLookupAdapter la(robase, tcfno);

	//Test AnyLookupInterfaceAdapter functionality
	assertEqual("bar", la.Lookup("hank", "X"));
	assertEqual("bar", la.Lookup("compo@site", "X", '@'));
	t_assert(31416L == la.Lookup("long", 0L));
	std::cerr << "testConfNamedObject" << "1" << std::endl;
	assertEqual("X", la.Lookup("fox", "X"));
	std::cerr << "testConfNamedObject" << "2" << std::endl;

	//Test lookups aiming at the ConfiguredNamedObject's config
	assertEqual("FromConfigString", la.Lookup("FromConfigString", "X"));
	assertEqual(9L, la.Lookup("FromConfigLong", 0L));

	Anything base2;
	base2["FromConfigString"]	= "FromConfigStringBase2";
	base2["FromConfigLong"]		= 999L;
	ROAnything robase2(base2);
	ROAnyConfNamedObjectLookupAdapter la2(robase2, tcfno);

	//Test lookups testing precedence of ROAnythings content over ConfiguredNamedObject ones
	assertEqual("FromConfigStringBase2", la2.Lookup("FromConfigString", "X"));
	assertEqual(999L, la2.Lookup("FromConfigLong", 0L));
}

// builds up a suite of testcases, add a line for each testmethod
Test *ROAnyConfNamedObjectLookupAdapterTest::suite ()
{
	StartTrace(ROAnyConfNamedObjectLookupAdapterTest.suite);
	TestSuite *testSuite = new TestSuite;
	ADD_CASE(testSuite, ROAnyConfNamedObjectLookupAdapterTest, LookupTest);
	ADD_CASE(testSuite, ROAnyConfNamedObjectLookupAdapterTest, NoConfNamedObjectTest);
	ADD_CASE(testSuite, ROAnyConfNamedObjectLookupAdapterTest, NothingAtAllTest);
	return testSuite;
}

void ROAnyConfNamedObjectLookupAdapterTest::NoConfNamedObjectTest()
{
	StartTrace(ROAnyConfNamedObjectLookupAdapterTest.NoConfNamedObjectTest);

	Anything base;

	base["hank"] = "bar";
	base["long"] = 31416L;
	base["compo"]["site"] = "bar";

	ROAnything robase(base);
	ROAnyConfNamedObjectLookupAdapter la(robase, NULL);

	//Test AnyLookupInterfaceAdapter functionality
	assertEqual("bar", la.Lookup("hank", "X"));
	assertEqual("bar", la.Lookup("compo@site", "X", '@'));
	t_assert(31416L == la.Lookup("long", 0L));
	assertEqual("X", la.Lookup("fox", "X"));

	//Test lookups aiming at the ConfiguredNamedObject's config
	assertEqual("XXXX", la.Lookup("FromConfigString", "XXXX"));
	assertEqual(4L, la.Lookup("FromConfigLong", 4L));
}

void ROAnyConfNamedObjectLookupAdapterTest::NothingAtAllTest()
{
	StartTrace(ROAnyConfNamedObjectLookupAdapterTest.NothingAtAllTest);

	Anything base;
	ROAnything robase(base);
	ROAnyConfNamedObjectLookupAdapter la(robase, NULL);

	//Test AnyLookupInterfaceAdapter functionality
	assertEqual("X", la.Lookup("hank", "X"));
	assertEqual("X", la.Lookup("compo@site", "X", '@'));
	t_assert(0L == la.Lookup("long", 0L));
}

//---- TestConfNamedObj ----------------------------------------------------------------
TestConfNamedObj::TestConfNamedObj(const char *name) : ConfNamedObject(name)
{
	StartTrace(TestConfNamedObj.TestConfNamedObj);
}

IFAObject *TestConfNamedObj::Clone(Allocator *a) const
{
	StartTrace(TestConfNamedObj.Clone);
	// SINGLETON, see NotConed object
	TestConfNamedObj *nonconst_this = (TestConfNamedObj *) this;
	return nonconst_this;
}

bool TestConfNamedObj::DoGetConfigName(const char *category, const char *objName, String &configFileName) const
{
	StartTrace(TestConfNamedObj.DoGetConfigName);
	configFileName = "TestConfNamedObj";
	return true;
}

bool TestConfNamedObj::DoLoadConfig(const char *category)
{
	StartTrace(TestConfNamedObj.DoLoadConfig);
	ConfNamedObject::DoLoadConfig(category);
	TraceAny(fConfig, "Config of TestConfNamedObj");
	return true;
}
