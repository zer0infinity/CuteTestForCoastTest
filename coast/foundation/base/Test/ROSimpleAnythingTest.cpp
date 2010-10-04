/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

//--- interface include --------------------------------------------------------
#include "ROSimpleAnythingTest.h"

//--- module under test --------------------------------------------------------
#include "Anything.h"

//--- standard modules used ----------------------------------------------------
#include "IFAObject.h"

//--- test modules used --------------------------------------------------------
#include "TestSuite.h"

class DummyROIFAObj: public IFAObject
{
public:
	DummyROIFAObj(const char *tst = "foo") { }
	/*! @copydoc IFAObject::Clone(Allocator *) */
	IFAObject *Clone(Allocator *a) const {
		return new (a) DummyROIFAObj("dummy");
	}
};

//---- ROSimpleAnythingTest --------------------------------------------------
ROSimpleAnythingTest::ROSimpleAnythingTest(TString tname) : TestCaseType(tname)
{
}

ROSimpleAnythingTest::~ROSimpleAnythingTest()
{
}

void ROSimpleAnythingTest::ConstructorsAndConversions()
{
	EmptyConstructor();
	AnyConstructor();
} // ConstructorsAndConversions

void ROSimpleAnythingTest::AssignmentsAndConversions()
{
} // AssignmentsAndConversions

void ROSimpleAnythingTest::EmptyConstructor()
{
	ROAnything roTest;
	DummyROIFAObj dummyIFA;

	// test standard defaults
	assertEqual(0L, roTest.AsCharPtr());
	assertEqual(0L, roTest.AsLong(0));
	assertEqual(false, roTest.AsBool(false));
	assertEqual(0L, (long)roTest.AsDouble());
	assertEqual(0L, (long)roTest.AsIFAObject());

	// test custom defaults
	assertEqual("test", roTest.AsCharPtr("test"));
	assertEqual(10L, roTest.AsLong(10));
	assertEqual(0L, roTest.AsLong(0L));
	assertEqual(-10L, roTest.AsLong(-10));
	assertEqual(true, roTest.AsBool(true));
	assertEqual(false, roTest.AsBool(false));
	t_assert(1.0 < roTest.AsDouble(1.1));
	assertEqual((long)&dummyIFA, (long)roTest.AsIFAObject(&dummyIFA));
}

void ROSimpleAnythingTest::AnyConstructor()
{
	AnyIntConstructor();
	AnyBoolConstructor();
	AnyLongConstructor();
	AnyFloatConstructor();
	AnyDoubleConstructor();
	AnyIFAObjConstructor();
}

void ROSimpleAnythingTest::AnyIntConstructor()
{
	const int cTestVal = 7;
	Anything anyTest(cTestVal);
	ROAnything roTest(anyTest);
	DummyROIFAObj dummyIFA;

	// test standard defaults
	assertEqual("7", roTest.AsCharPtr());
	assertEqual(cTestVal, roTest.AsLong(0));
	assertEqual(true, roTest.AsBool());
	assertEqual(cTestVal, (long)roTest.AsDouble());
	assertEqual(0L, (long)roTest.AsIFAObject());

	// test custom defaults
	assertEqual("7", roTest.AsCharPtr("test"));
	assertEqual(cTestVal, roTest.AsLong(10));
	assertEqual(cTestVal, roTest.AsLong(0L));
	assertEqual(cTestVal, roTest.AsLong(-10));
	assertEqual(true, roTest.AsBool(true));
	assertEqual(true, roTest.AsBool(false));
	t_assert(1.0 < roTest.AsDouble(1.1));
	assertEqual((long)&dummyIFA, (long)roTest.AsIFAObject(&dummyIFA));
}

void ROSimpleAnythingTest::AnyBoolConstructor()
{
	const bool cTestVal = true;
	Anything anyTest(cTestVal);
	ROAnything roTest(anyTest);
	DummyROIFAObj dummyIFA;

	// test standard defaults
	assertEqual("1", roTest.AsCharPtr());
	assertEqual(cTestVal, roTest.AsLong(0));
	assertEqual(true, roTest.AsBool());
	assertEqual(cTestVal, (long)roTest.AsDouble());
	assertEqual(0L, (long)roTest.AsIFAObject());

	// test custom defaults
	assertEqual("1", roTest.AsCharPtr("test"));
	assertEqual(cTestVal, roTest.AsLong(10));
	assertEqual(cTestVal, roTest.AsLong(0L));
	assertEqual(cTestVal, roTest.AsLong(-10));
	assertEqual(cTestVal, roTest.AsBool(true));
	assertEqual(cTestVal, roTest.AsBool(false));
	assertEqual(cTestVal, (long)roTest.AsDouble(1.1));
	assertEqual((long)&dummyIFA, (long)roTest.AsIFAObject(&dummyIFA));
}

void ROSimpleAnythingTest::AnyLongConstructor()
{
	const long cTestVal = 1234L;
	Anything anyTest(cTestVal);
	ROAnything roTest(anyTest);
	DummyROIFAObj dummyIFA;

	// test standard defaults
	assertEqual("1234", roTest.AsCharPtr());
	assertEqual(cTestVal, roTest.AsLong(0));
	assertEqual(true, roTest.AsBool());
	assertEqual(cTestVal, (long)roTest.AsDouble());
	assertEqual(0L, (long)roTest.AsIFAObject());

	// test custom defaults
	assertEqual("1234", roTest.AsCharPtr("test"));
	assertEqual(cTestVal, roTest.AsLong(10));
	assertEqual(cTestVal, roTest.AsLong(0L));
	assertEqual(cTestVal, roTest.AsLong(-10));
	assertEqual(true, roTest.AsBool(true));
	assertEqual(true, roTest.AsBool(false));
	assertEqual(cTestVal, (long)roTest.AsDouble(1.1));
	assertEqual((long)&dummyIFA, (long)roTest.AsIFAObject(&dummyIFA));
}

void ROSimpleAnythingTest::AnyFloatConstructor()
{
	const float cTestVal = float(22.22);
	Anything anyTest(cTestVal);
	ROAnything roTest(anyTest);
	DummyROIFAObj dummyIFA;

	// test standard defaults
	assertEqual("22.21", roTest.AsString().SubString(0, 5));
	assertEqual((long)cTestVal, roTest.AsLong(0));
	assertEqual(false, roTest.AsBool());
	assertEqual((long)cTestVal, (long)roTest.AsDouble());
	assertEqual(0L, (long)roTest.AsIFAObject());

	// test custom defaults
	assertEqual("22.21", roTest.AsString("test").SubString(0, 5));
	assertEqual((long)cTestVal, roTest.AsLong(10));
	assertEqual((long)cTestVal, roTest.AsLong(0L));
	assertEqual((long)cTestVal, roTest.AsLong(-10));
	assertEqual(true, roTest.AsBool(true));
	assertEqual(false, roTest.AsBool(false));
	assertEqual((long)cTestVal, (long)roTest.AsDouble(1.1));
	assertEqual((long)&dummyIFA, (long)roTest.AsIFAObject(&dummyIFA));
}

void ROSimpleAnythingTest::AnyDoubleConstructor()
{
	const float cTestVal = float(33.33);
	Anything anyTest(cTestVal);
	ROAnything roTest(anyTest);
	DummyROIFAObj dummyIFA;

	// test standard defaults
	assertEqual("33.33", roTest.AsString().SubString(0, 5));
	assertEqual((long)cTestVal, roTest.AsLong(0));
	assertEqual(false, roTest.AsBool());
	assertEqual((long)cTestVal, (long)roTest.AsDouble());
	assertEqual(0L, (long)roTest.AsIFAObject());

	// test custom defaults
	assertEqual("33.33", roTest.AsString("test").SubString(0, 5));
	assertEqual((long)cTestVal, roTest.AsLong(10));
	assertEqual((long)cTestVal, roTest.AsLong(0L));
	assertEqual((long)cTestVal, roTest.AsLong(-10));
	assertEqual(true, roTest.AsBool(true));
	assertEqual(false, roTest.AsBool(false));
	assertEqual((long)cTestVal, (long)roTest.AsDouble(1.1));
	assertEqual((long)&dummyIFA, (long)roTest.AsIFAObject(&dummyIFA));
}

void ROSimpleAnythingTest::AnyIFAObjConstructor()
{
	DummyROIFAObj cTestVal("foo");
	Anything anyTest(&cTestVal);
	ROAnything roTest(anyTest);
	DummyROIFAObj dummyIFA;

	// test standard defaults
	assertEqual("IFAObject", roTest.AsCharPtr());
	assertEqual((long)&cTestVal, roTest.AsLong(0));
	assertEqual(false, roTest.AsBool());
	assertEqual(0L, (long)roTest.AsDouble());
	assertEqual((long)&cTestVal, (long)roTest.AsIFAObject());

	// test custom defaults
	assertEqual("IFAObject", roTest.AsCharPtr("test"));
	assertEqual((long)&cTestVal, roTest.AsLong(10));
	assertEqual((long)&cTestVal, roTest.AsLong(0L));
	assertEqual((long)&cTestVal, roTest.AsLong(-10));
	assertEqual(true, roTest.AsBool(true));
	assertEqual(false, roTest.AsBool(false));
	assertEqual((long)1.1, (long)roTest.AsDouble(1.1));
	assertEqual((long)&cTestVal, (long)roTest.AsIFAObject(&dummyIFA));
}

Test *ROSimpleAnythingTest::suite ()
{
	TestSuite *testSuite = new TestSuite;
	ADD_CASE(testSuite, ROSimpleAnythingTest, ConstructorsAndConversions);
	ADD_CASE(testSuite, ROSimpleAnythingTest, AssignmentsAndConversions);

	return testSuite;
}
