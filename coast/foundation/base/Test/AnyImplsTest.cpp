/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

//--- interface include --------------------------------------------------------
#include "AnyImplsTest.h"

//--- test modules used --------------------------------------------------------
#include "TestSuite.h"

//--- module under test --------------------------------------------------------
#include "AnyImpls.h"

//--- standard modules used ----------------------------------------------------
#include "Dbg.h"
#include "StringStream.h"
#include <iomanip>

//---- AnyImplsTest ----------------------------------------------------------------
AnyImplsTest::AnyImplsTest(TString tstrName) : TestCaseType(tstrName)
{
	StartTrace(AnyImplsTest.Ctor);
}

AnyImplsTest::~AnyImplsTest()
{
	StartTrace(AnyImplsTest.Dtor);
}

void AnyImplsTest::ThisToHexTest()
{
	StartTrace(AnyImplsTest.ThisToHexTest);
	{
		AnyImpl *d=new(Storage::Current())AnyLongImpl(123L, Storage::Current());
		String res = d->ThisToHex();
		assertCompare(sizeof(void *)*2L,equal_to,static_cast<unsigned long>(res.Length()));
		StringStream os;
		os << std::hex << std::setw(sizeof(d)*2)<<std::setfill('0')<<std::noshowbase<<reinterpret_cast<unsigned long long>(static_cast<void*>(d));
		assertCharPtrEqual(os.str(),res);
	}
}

// builds up a suite of testcases, add a line for each testmethod
Test *AnyImplsTest::suite ()
{
	StartTrace(AnyImplsTest.suite);
	TestSuite *testSuite = new TestSuite;
	ADD_CASE(testSuite, AnyImplsTest, ThisToHexTest);
	return testSuite;
}
