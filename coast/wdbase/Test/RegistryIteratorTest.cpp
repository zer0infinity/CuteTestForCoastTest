/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#include "RegistryIteratorTest.h"
#include "Tracer.h"
#include "TestSuite.h"
#include "Registry.h"

RegistryIteratorTest::RegistryIteratorTest(TString tname) : TestCaseType(tname)
{
	StartTrace(RegistryIteratorTest.Ctor);
}

RegistryIteratorTest::~RegistryIteratorTest()
{
	StartTrace(RegistryIteratorTest.Dtor);
}

void RegistryIteratorTest::IterateOverNullRegistry()
{
	StartTrace(RegistryIteratorTest.IterateOverNullRegistry);
	{
		Registry *r = 0;
		String key("undefined");
		RegistryIterator ri(r);
		t_assertm(!ri.HasMore(), "expected RegistryIterator to have no elements");
		t_assertm(ri.Next(key) == 0, "expected RegistryIterator to return null element");
		t_assertm(!ri.HasMore(), "expected RegistryIterator to have no elements");
		t_assertm(ri.RemoveNext(key) == 0, "expected RegistryIterator to return null element");
		t_assertm(!ri.HasMore(), "expected RegistryIterator to have no elements");
		assertEqual("undefined", key);
	}
	{
		Registry *r = 0;
		String key("undefined");
		RegistryIterator ri(r, false);
		t_assertm(!ri.HasMore(), "expected RegistryIterator to have no elements");
		t_assertm(ri.Next(key) == 0, "expected RegistryIterator to return null element");
		t_assertm(!ri.HasMore(), "expected RegistryIterator to have no elements");
		t_assertm(ri.RemoveNext(key) == 0, "expected RegistryIterator to return null element");
		t_assertm(!ri.HasMore(), "expected RegistryIterator to have no elements");
		assertEqual("undefined", key);
	}

}

void RegistryIteratorTest::IterateOverEmptyRegistry()
{
	StartTrace(RegistryIteratorTest.IterateOverEmptyRegistry);
	{
		Registry r("emptyforward");
		String key("undefined");
		RegistryIterator ri(&r);
		t_assertm(!ri.HasMore(), "expected RegistryIterator to have no elements");
		t_assertm(ri.Next(key) == 0, "expected RegistryIterator to return null element");
		t_assertm(!ri.HasMore(), "expected RegistryIterator to have no elements");
		t_assertm(ri.RemoveNext(key) == 0, "expected RegistryIterator to return null element");
		t_assertm(!ri.HasMore(), "expected RegistryIterator to have no elements");
		assertEqual("undefined", key);
	}
	{
		Registry r("emptybackward");
		String key("undefined");
		RegistryIterator ri(&r, false);
		t_assertm(!ri.HasMore(), "expected RegistryIterator to have no elements");
		t_assertm(ri.Next(key) == 0, "expected RegistryIterator to return null element");
		t_assertm(!ri.HasMore(), "expected RegistryIterator to have no elements");
		t_assertm(ri.RemoveNext(key) == 0, "expected RegistryIterator to return null element");
		t_assertm(!ri.HasMore(), "expected RegistryIterator to have no elements");
		assertEqual("undefined", key);
	}

}

void RegistryIteratorTest::IterateOverOneElementRegistry()
{
	StartTrace(RegistryIteratorTest.IterateOverOneElementRegistry);
	{
		Registry r("oneElementForward");
		Registry r1("test");
		r.RegisterRegisterableObject("One", &r1);
		String key("undefined");
		RegistryIterator ri(&r);
		t_assertm(ri.HasMore(), "expected RegistryIterator to have one element");
		t_assertm(ri.Next(key) == &r1, "expected RegistryIterator to return null element");
		t_assertm(!ri.HasMore(), "expected RegistryIterator to have no more elements");
		assertEqual("One", key);
	}
	{
		Registry r("oneElementBackward");
		Registry r1("test");
		r.RegisterRegisterableObject("One", &r1);
		String key("undefined");
		RegistryIterator ri(&r, false);
		t_assertm(ri.HasMore(), "expected RegistryIterator to have one element");
		t_assertm(ri.Next(key) == &r1, "expected RegistryIterator to return null element");
		t_assertm(!ri.HasMore(), "expected RegistryIterator to have no more elements");
		assertEqual("One", key);
	}

}

void RegistryIteratorTest::IterateOverNElementRegistry()
{
	StartTrace(RegistryIteratorTest.IterateOverNElementRegistry);
	{
		Registry r("oneElementForward");
		Registry r1("test1");
		Registry r2("test2");
		r.RegisterRegisterableObject("One", &r1);
		r.RegisterRegisterableObject("Two", &r2);
		String key("undefined");
		RegistryIterator ri(&r);
		t_assertm(ri.HasMore(), "expected RegistryIterator to have one element");
		t_assertm(ri.Next(key) == &r1, "expected RegistryIterator to return null element");
		t_assertm(ri.HasMore(), "expected RegistryIterator to have no more elements");
		assertEqual("One", key);
		t_assertm(ri.HasMore(), "expected RegistryIterator to have one element");
		t_assertm(ri.Next(key) == &r2, "expected RegistryIterator to return null element");
		t_assertm(!ri.HasMore(), "expected RegistryIterator to have no more elements");
		assertEqual("Two", key);
	}
	{
		Registry r("oneElementBackward");
		Registry r1("test1");
		Registry r2("test2");
		r.RegisterRegisterableObject("One", &r1);
		r.RegisterRegisterableObject("Two", &r2);
		String key("undefined");
		RegistryIterator ri(&r, false);
		t_assertm(ri.HasMore(), "expected RegistryIterator to have one element");
		t_assertm(ri.Next(key) == &r2, "expected RegistryIterator to return null element");
		t_assertm(ri.HasMore(), "expected RegistryIterator to have no more elements");
		assertEqual("Two", key);
		t_assertm(ri.HasMore(), "expected RegistryIterator to have one element");
		t_assertm(ri.Next(key) == &r1, "expected RegistryIterator to return null element");
		t_assertm(!ri.HasMore(), "expected RegistryIterator to have no more elements");
		assertEqual("One", key);
	}

}

void RegistryIteratorTest::IterateWhileRemoving()
{
	StartTrace(RegistryIteratorTest.IterateWhileRemoving);
	{
		Registry r("oneElementForward");
		Registry r1("test1");
		Registry r2("test2");
		r.RegisterRegisterableObject("One", &r1);
		r.RegisterRegisterableObject("Two", &r2);
		String key("undefined");
		RegistryIterator ri(&r);
		t_assertm(ri.HasMore(), "expected RegistryIterator to have one element");
		t_assertm(ri.RemoveNext(key) == &r1, "expected RegistryIterator to return null element");
		t_assertm(ri.HasMore(), "expected RegistryIterator to have no more elements");
		assertEqual("One", key);
		t_assertm(ri.HasMore(), "expected RegistryIterator to have one element");
		t_assertm(ri.RemoveNext(key) == &r2, "expected RegistryIterator to return null element");
		t_assertm(!ri.HasMore(), "expected RegistryIterator to have no more elements");
		assertEqual("Two", key);
		t_assertm(ri.RemoveNext(key) == 0, "expected RegistryIterator to return null element");
		t_assertm(!ri.HasMore(), "expected RegistryIterator to have no more elements");
		assertEqual("Two", key);
	}
	{
		Registry r("oneElementBackward");
		Registry r1("test1");
		Registry r2("test2");
		r.RegisterRegisterableObject("One", &r1);
		r.RegisterRegisterableObject("Two", &r2);
		String key("undefined");
		RegistryIterator ri(&r, false);
		t_assertm(ri.HasMore(), "expected RegistryIterator to have one element");
		t_assertm(ri.RemoveNext(key) == &r2, "expected RegistryIterator to return null element");
		t_assertm(ri.HasMore(), "expected RegistryIterator to have no more elements");
		assertEqual("Two", key);
		t_assertm(ri.HasMore(), "expected RegistryIterator to have one element");
		t_assertm(ri.RemoveNext(key) == &r1, "expected RegistryIterator to return null element");
		t_assertm(!ri.HasMore(), "expected RegistryIterator to have no more elements");
		assertEqual("One", key);
		t_assertm(!ri.HasMore(), "expected RegistryIterator to have one element");
		t_assertm(ri.RemoveNext(key) == 0, "expected RegistryIterator to return null element");
		t_assertm(!ri.HasMore(), "expected RegistryIterator to have no more elements");
		assertEqual("One", key);
	}

}

// builds up a suite of testcases, add a line for each testmethod
Test *RegistryIteratorTest::suite ()
{
	StartTrace(RegistryIteratorTest.suite);
	TestSuite *testSuite = new TestSuite;

	ADD_CASE(testSuite, RegistryIteratorTest, IterateOverNullRegistry);
	ADD_CASE(testSuite, RegistryIteratorTest, IterateOverEmptyRegistry);
	ADD_CASE(testSuite, RegistryIteratorTest, IterateOverOneElementRegistry);
	ADD_CASE(testSuite, RegistryIteratorTest, IterateOverNElementRegistry);
	ADD_CASE(testSuite, RegistryIteratorTest, IterateWhileRemoving);

	return testSuite;

}
