/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

//--- interface include --------------------------------------------------------
#include "ObjectListTest.h"

//--- test modules used --------------------------------------------------------
#include "TestSuite.h"

//--- module under test --------------------------------------------------------
#include "ObjectList.h"

//--- standard modules used ----------------------------------------------------
#include "ITOString.h"
#include "Dbg.h"

//--- c-modules used -----------------------------------------------------------

//---- ObjectListTest ----------------------------------------------------------------
ObjectListTest::ObjectListTest(TString tstrName) : TestCaseType(tstrName)
{
	StartTrace(ObjectListTest.Ctor);
}

ObjectListTest::~ObjectListTest()
{
	StartTrace(ObjectListTest.Dtor);
}

void ObjectListTest::CtorTest()
{
	StartTrace(ObjectListTest.CtorTest);
	StartTraceMem(ObjectListTest.CtorTest);
	{
		TraceMemDelta("before allocation");
		String *pString;
		const long lCount = 10;
		pString = new String("Guguseli");
		ObjectList<String *> aStringList("StringPointerList");
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
		t_assertm(!aStringList.RemoveHead(pString), "expected empty list!");
		t_assert(pString == NULL);
	}
	TraceMemDelta("before terminating");
}

void ObjectListTest::DtorTest()
{
	StartTrace(ObjectListTest.DtorTest);
	StartTraceMem(ObjectListTest.DtorTest);
	{
		TraceMemDelta("before allocation");
		String *pString;
		const long lCount = 10;
		{
			ObjectList<String *> aStringList("StringPointerList");
			for (long lIdx = 0; lIdx < lCount; lIdx++) {
				pString = new String("Iteration:");
				pString->Append(lIdx);
				aStringList.InsertTail(pString);
			}
			assertEqual(lCount, aStringList.GetSize());
			TraceMemDelta("after allocation of many strings");
			// set to destructive shutdown which should delete pointees hold in list
			aStringList.SignalShutdown(true);
		}
		TraceMemDelta("after destruction of list");
	}
	{
		TraceMemDelta("before allocation");
		const long lCount = 10;
		{
			ObjectList<int> aIntList("IntList");
			for (long lIdx = 0; lIdx < lCount; lIdx++) {
				aIntList.InsertTail((int)lCount);
			}
			assertEqual(lCount, aIntList.GetSize());
			TraceMemDelta("after allocation of many ints");
			// set to destructive shutdown which should delete pointees hold in list
			aIntList.SignalShutdown(true);
		}
		TraceMemDelta("after destruction of list");
	}
}

// builds up a suite of testcases, add a line for each testmethod
Test *ObjectListTest::suite ()
{
	StartTrace(ObjectListTest.suite);
	TestSuite *testSuite = new TestSuite;

	ADD_CASE(testSuite, ObjectListTest, CtorTest);
	ADD_CASE(testSuite, ObjectListTest, DtorTest);

	return testSuite;
}
