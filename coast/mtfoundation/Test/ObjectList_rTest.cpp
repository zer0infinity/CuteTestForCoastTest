/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

//--- interface include --------------------------------------------------------
#include "ObjectList_rTest.h"

//--- test modules used --------------------------------------------------------
#include "TestSuite.h"

//--- module under test --------------------------------------------------------
#include "ObjectList_r.h"

//--- standard modules used ----------------------------------------------------
#include "Dbg.h"

//--- c-modules used -----------------------------------------------------------

//---- ObjectList_rTest ----------------------------------------------------------------
ObjectList_rTest::ObjectList_rTest(TString name) : TestCase(name)
{
	StartTrace(ObjectList_rTest.Ctor);
}

ObjectList_rTest::~ObjectList_rTest()
{
	StartTrace(ObjectList_rTest.Dtor);
}

// setup for this TestCase
void ObjectList_rTest::setUp ()
{
	StartTrace(ObjectList_rTest.setUp);
}

void ObjectList_rTest::tearDown ()
{
	StartTrace(ObjectList_rTest.tearDown);
}

void ObjectList_rTest::CtorTest()
{
	StartTrace(ObjectList_rTest.CtorTest);
	StartTraceMem(ObjectList_rTest.CtorTest);
	{
		TraceMemDelta("before allocation");
		String *pString;
		const long lCount = 10;
		pString = new String("Guguseli");
		ObjectList_r<String *> aStringList("StringPointerList_r");
		aStringList.InsertTail(pString);
		assertEqual(1, aStringList.GetSize());
		TraceMemDelta("before deallocation");
		if ( t_assert(aStringList.RemoveHead(pString)) ) {
			delete pString;
		}
		assertEqual(0, aStringList.GetSize());
		TraceMemDelta("after deallocation");
		for (long lIdx = 0; lIdx < lCount; lIdx++) {
			pString = new String("Iteration:");
			pString->Append(lIdx);
			aStringList.InsertTail(pString);
		}
		assertEqual(lCount, aStringList.GetSize());
		TraceMemDelta("after reallocation of many strings");
		for (long lIdx2 = 0; lIdx2 < lCount; lIdx2++) {
			if ( aStringList.RemoveHead(pString) ) {
				Trace("current value [" << *pString << "]");
				assertCharPtrEqual(String("Iteration:").Append(lIdx2), *pString);
				delete pString;
			}
		}
		TraceMemDelta("after removal of many strings");
		pString = NULL;
		// wait on Remove for 10ms but it should fail
		t_assertm(!aStringList.RemoveHead(pString, 0L, 10000000L), "expected empty list!");
		t_assert(pString == NULL);
	}
	TraceMemDelta("before terminating");
}

// builds up a suite of testcases, add a line for each testmethod
Test *ObjectList_rTest::suite ()
{
	StartTrace(ObjectList_rTest.suite);
	TestSuite *testSuite = new TestSuite;

	ADD_CASE(testSuite, ObjectList_rTest, CtorTest);

	return testSuite;
}
