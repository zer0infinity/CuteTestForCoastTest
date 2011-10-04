/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#include "ObjectList_rTest.h"
#include "TestSuite.h"
#include "ObjectList_r.h"
#include <deque>

#define OBJECTLISTMACRO(listtype, allocatortype)\
{\
	TraceMemDelta("before allocation");\
	const long lCount=200;\
	MemChecker mc("ObjectList_rTest.CtorTest", Coast::Storage::Global());\
	{\
		Trace(_QUOTE_( testing listtype##allocatortype ));\
		ObjectList_r<String*, listtype, allocatortype> aStringList(_QUOTE_( listtype##allocatortype ));\
		String *pString;\
		pString=new String("Guguseli");\
		aStringList.InsertTail(pString);\
		assertCompare(1L, equal_to, (long)aStringList.GetSize());\
		TraceMemDelta("before deallocation");\
		if ( t_assert(aStringList.RemoveHead(pString)) )\
		{\
			delete pString;\
		}\
		assertCompare(0L, equal_to, (long)aStringList.GetSize());\
		TraceMemDelta("after deallocation");\
		for (long lIdx=0; lIdx < lCount; ++lIdx)\
		{\
			pString = new String("Iteration:");\
			pString->Append(lIdx);\
			aStringList.InsertTail(pString);\
		}\
		assertCompare(lCount, equal_to, (long)aStringList.GetSize());\
		TraceMemDelta("after reallocation of many strings");\
		for (long lIdx2=0; lIdx2 < (lCount/2); ++lIdx2)\
		{\
			if ( aStringList.RemoveHead(pString) )\
			{\
				Trace("current value [" << *pString << "]");\
				assertCharPtrEqual(String("Iteration:").Append(lIdx2), *pString);\
				delete pString;\
			}\
		}\
		TraceMemDelta("after removal of many strings");\
		assertCompare(0LL, less, mc.CheckDelta());\
		pString=NULL;\
		Trace("Remove another element which should succeed"); \
		if ( t_assertm(aStringList.RemoveHead(pString, 0L, 10000000L), "expected RemoveHead to still succeed") )\
		{\
			delete pString;\
		}\
		Trace("destructive shutdown of list, expect remaining elements getting deleted"); \
		aStringList.SignalShutdown(true);\
	}\
	assertCompare(0LL, equal_to, mc.CheckDelta());\
}

void ObjectList_rTest::CtorTest()
{
	std::auto_ptr<MemTracker> newTracker( Coast::Storage::MakeMemTracker("GlobalAllocator", false) );
	MemTracker *oldTracker = Coast::Storage::Global()->ReplaceMemTracker( newTracker.get() );

	StartTrace(ObjectList_rTest.CtorTest);
	StartTraceMem(ObjectList_rTest.CtorTest);
	{
		// Allocate several tester objects.  Delete two.
		Trace("with deque. . .");
		MemChecker mc("deque-test", Coast::Storage::Global());
		{
#if defined(__GNUG__)  && ( __GNUC__ >= 4 )
			std::deque<String *, STLStorage::fast_pool_allocator_global<String *> > l;
#else
			std::deque<String *, std::allocator<String *> > l;
#endif
			String *pS(NULL);
			for (long i = 0; i < 150; ++i) {
				pS = new String("gaga");
				pS->Append(i);
				l.push_back(pS);
			}
			delete l.back();
			Trace("deleted back");
			l.pop_back();
			delete l.front();
			Trace("deleted front");
			l.pop_front();
			assertCompare(148L, equal_to, (long)l.size());
			for (int j = 0; j < (150 - 2); ++j) {
				String *pS2 = l.front();
				if ( pS2 ) {
					Trace(*pS2);
				}
				delete pS2;
				l.pop_front();
			}
			Trace("clearing list");
			l.clear();
		}
		assertCompare(0LL, equal_to, mc.CheckDelta());
	}
	{
		// Allocate several tester objects.  Delete two.
		Trace("with list. . .");
		MemChecker mc("list-test", Coast::Storage::Global());
		{
#if defined(__GNUG__)  && ( __GNUC__ >= 4 )
			std::list<String *, STLStorage::fast_pool_allocator_global<String *> > l;
#else
			std::list<String *, std::allocator<String *> > l;
#endif
			String *pS(NULL);
			for (long i = 0; i < 150; ++i) {
				pS = new String("gaga");
				pS->Append(i);
				l.push_back(pS);
			}
			delete l.back();
			Trace("deleted back");
			l.pop_back();
			delete l.front();
			Trace("deleted front");
			l.pop_front();
			assertCompare(148L, equal_to, (long)l.size());
			for (int j = 0; j < (150 - 2); ++j) {
				String *pS2 = l.front();
				if ( pS2 ) {
					Trace(*pS2);
				}
				delete pS2;
				l.pop_front();
			}
			Trace("clearing list");
			l.clear();
		}
		assertCompare(0LL, equal_to, mc.CheckDelta());
	}
#if defined(__GNUG__)  && ( __GNUC__ >= 4 )
	OBJECTLISTMACRO(std::list, STLStorage::pool_allocator_global);
	OBJECTLISTMACRO(std::list, STLStorage::fast_pool_allocator_global);
	Trace("continuing ObjectList_rTests");
	OBJECTLISTMACRO(std::deque, STLStorage::pool_allocator_global);
	OBJECTLISTMACRO(std::deque, STLStorage::fast_pool_allocator_global);
#else
	OBJECTLISTMACRO(std::list, std::allocator);
#endif
	TraceMemDelta("before terminating");

	Coast::Storage::Global()->ReplaceMemTracker( oldTracker );
}

// builds up a suite of testcases, add a line for each testmethod
Test *ObjectList_rTest::suite ()
{
	StartTrace(ObjectList_rTest.suite);
	TestSuite *testSuite = new TestSuite;

	ADD_CASE(testSuite, ObjectList_rTest, CtorTest);

	return testSuite;
}
