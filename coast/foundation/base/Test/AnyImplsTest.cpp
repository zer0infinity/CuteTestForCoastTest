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

void AnyImplsTest::DoubleToStringTest()
{
	StartTrace(AnyImplsTest.DoubleToStringTest);
	{
		String strBuf;
		double dValue = 1.0;
		String::DoubleToString(dValue, strBuf);
		assertCharPtrEqual( "1.0", strBuf);
	}
	{
		String strBuf;
		double dValue = 1000000;
		String::DoubleToString(dValue, strBuf);
		assertCharPtrEqual( "1000000.0", strBuf);
	}
	{
		String strBuf;
		double dValue = 1.23456789;
		String::DoubleToString(dValue, strBuf);
		assertCharPtrEqual( "1.23456789", strBuf);
	}
	{
		String strBuf;
		double dValue = 0.0;
		String::DoubleToString(dValue, strBuf);
		assertCharPtrEqual( "0.0", strBuf);
	}
	{
		String strBuf;
		double dValue = 0.056;
		String::DoubleToString(dValue, strBuf);
		assertCharPtrEqual( "0.056", strBuf);
	}
	{
		// string will be cut 15 decimals after decimal point
		String strBuf;
		double dValue = (1.0 / 3.0);
		String::DoubleToString(dValue, strBuf);
		assertCharPtrEqual( "0.333333333333333", strBuf);
	}
	{
		// string will be cut 15 decimals after decimal point
		String strBuf;
		double dValue = (100.0 / 3.0);
		String::DoubleToString(dValue, strBuf);
		assertCharPtrEqual( "33.333333333333336", strBuf);
	}
	{
		// string will be cut 15 decimals after decimal point
		String strBuf;
		double dValue = 27.16;
		String::DoubleToString(dValue, strBuf);
		assertCharPtrEqual( "27.16", strBuf);
	}
	{
		String strBuf;
		double dValue = 1e15 + 0.123456789;
		String::DoubleToString(dValue, strBuf);
		assertCharPtrEqual( "1000000000000000.125", strBuf);
	}
	{
		String strBuf;
		double dValue = 1e14 + 0.123456789;
		String::DoubleToString(dValue, strBuf);
		assertCharPtrEqual( "100000000000000.125", strBuf);
	}
	{
		String strBuf;
		double dValue = 1e12 + 0.123456789;
		String::DoubleToString(dValue, strBuf);
		assertCharPtrEqual( "1000000000000.1234130859375", strBuf);
	}
	{
		String strBuf;
		double dValue = (1e+20) + 0.22134;
		String::DoubleToString(dValue, strBuf);
		assertCharPtrEqual( "1e+20", strBuf);
	}
	{
		String strBuf;
		double dValue = 1e+30;
		String::DoubleToString(dValue, strBuf);
		assertCharPtrEqual( "1e+30", strBuf);
	}
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
	ADD_CASE(testSuite, AnyImplsTest, DoubleToStringTest);
	ADD_CASE(testSuite, AnyImplsTest, ThisToHexTest);
	return testSuite;
}
