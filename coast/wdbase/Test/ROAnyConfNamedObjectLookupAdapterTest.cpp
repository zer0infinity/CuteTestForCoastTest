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
#include "IFAConfObject.h"

//--- project modules used -----------------------------------------------------

//--- test modules used --------------------------------------------------------
#include "TestSuite.h"

//--- module under test --------------------------------------------------------
#include "ROAnyLookupAdapter.h"

//--- interface include --------------------------------------------------------
#include "ROAnyConfNamedObjectLookupAdapterTest.h"

RegCacheImpl(TestConfNamedObj);				// FindTestConfNamedObj()
RegisterTestConfNamedObj(TestConfNamedObj);

//---- ROAnyConfNamedObjectLookupAdapterTest ----------------------------------------------------------------
ROAnyConfNamedObjectLookupAdapterTest::ROAnyConfNamedObjectLookupAdapterTest(TString name) : TestCase(name)
{
	StartTrace(ROAnyConfNamedObjectLookupAdapterTest.Ctor);
}

ROAnyConfNamedObjectLookupAdapterTest::~ROAnyConfNamedObjectLookupAdapterTest()
{
	StartTrace(ROAnyConfNamedObjectLookupAdapterTest.Dtor);
}

// setup for this TestCase
void ROAnyConfNamedObjectLookupAdapterTest::setUp ()
{
	StartTrace(ROAnyConfNamedObjectLookupAdapterTest.setUp);
}

void ROAnyConfNamedObjectLookupAdapterTest::tearDown ()
{
	StartTrace(ROAnyConfNamedObjectLookupAdapterTest.tearDown);
}

void ROAnyConfNamedObjectLookupAdapterTest::testCase()
{
	StartTrace(ROAnyConfNamedObjectLookupAdapterTest.testCase);
}

void ROAnyConfNamedObjectLookupAdapterTest::testLookup()
{
	StartTrace(ROAnyConfNamedObjectLookupAdapterTest.testLookup);

	Anything base;
	TestConfNamedObj *tcfno = TestConfNamedObj::FindTestConfNamedObj("TestConfNamedObj");
	t_assertm(tcfno != NULL, "Ooops, Expected TestConfNamedObj pointer not to be null!");

	tcfno->CheckConfig("TestConfNamedObj", true);

	base["hank"] = "bar";
	base["long"] = 31416L;
	base["compo"]["site"] = "bar";

	ROAnything robase(base);
	ROAnyConfNamedObjectLookupAdapter la(robase, tcfno);

	//Test ROAnyLookupAdapter functionality
	assertEquals("bar", la.Lookup("hank", "X"));
	assertEquals("bar", la.Lookup("compo@site", "X", '@'));
	t_assert(31416L == la.Lookup("long", 0L));
	cerr << "testConfNamedObject" << "1" << endl;
	assertEquals("X", la.Lookup("fox", "X"));
	cerr << "testConfNamedObject" << "2" << endl;

	//Test lookups aiming at the ConfiguredNamedObject's config
	assertEquals("FromConfigString", la.Lookup("FromConfigString", "X"));
	assertEquals(9L, la.Lookup("FromConfigLong", 0L));

	Anything base2;
	base2["FromConfigString"]	= "FromConfigStringBase2";
	base2["FromConfigLong"]		= 999L;
	ROAnything robase2(base2);
	ROAnyConfNamedObjectLookupAdapter la2(robase2, tcfno);

	//Test lookups testing precedence of ROAnythings content over ConfiguredNamedObject ones
	assertEquals("FromConfigStringBase2", la2.Lookup("FromConfigString", "X"));
	assertEquals(999L, la2.Lookup("FromConfigLong", 0L));
}

// builds up a suite of testcases, add a line for each testmethod
Test *ROAnyConfNamedObjectLookupAdapterTest::suite ()
{
	StartTrace(ROAnyConfNamedObjectLookupAdapterTest.suite);
	TestSuite *testSuite = new TestSuite;

	ADD_CASE(testSuite, ROAnyConfNamedObjectLookupAdapterTest, testCase);
	ADD_CASE(testSuite, ROAnyConfNamedObjectLookupAdapterTest, testLookup);
	ADD_CASE(testSuite, ROAnyConfNamedObjectLookupAdapterTest, testNoConfNamedObject);

	return testSuite;
}

void ROAnyConfNamedObjectLookupAdapterTest::testNoConfNamedObject()
{
	StartTrace(ROAnyConfNamedObjectLookupAdapterTest.testNoConfNamedObject);

	Anything base;

	base["hank"] = "bar";
	base["long"] = 31416L;
	base["compo"]["site"] = "bar";

	ROAnything robase(base);
	ROAnyConfNamedObjectLookupAdapter la(robase, NULL);

	//Test ROAnyLookupAdapter functionality
	assertEquals("bar", la.Lookup("hank", "X"));
	assertEquals("bar", la.Lookup("compo@site", "X", '@'));
	t_assert(31416L == la.Lookup("long", 0L));
	cerr << "NOtestNoConfNamedObject" << "1" << endl;
	assertEquals("X", la.Lookup("fox", "X"));
	cerr << "NOtestNoConfNamedObject" << "2" << endl;

	//Test lookups aiming at the ConfiguredNamedObject's config
	assertEquals("XXXX", la.Lookup("FromConfigString", "XXXX"));
	assertEquals(4L, la.Lookup("FromConfigLong", 4L));
}

//---- TestConfNamedObj ----------------------------------------------------------------
TestConfNamedObj::TestConfNamedObj(const char *name) : ConfNamedObject(name)
{
	StartTrace(TestConfNamedObj.TestConfNamedObj);
}

IFAObject *TestConfNamedObj::Clone() const
{
	StartTrace(TestConfNamedObj.Clone);
	// SINGLETON, see NotConed object
	TestConfNamedObj *nonconst_this = (TestConfNamedObj *) this;
	return nonconst_this;
}

bool TestConfNamedObj::DoGetConfigName(const char *category, const char *objName, String &configFileName)
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
