/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

//--- test modules used --------------------------------------------------------
#include "TestSuite.h"

//--- module under test --------------------------------------------------------
#include "AccessManager.h"

//--- interface include --------------------------------------------------------
#include "AccessManagerModuleTest.h"

//--- standard modules used ----------------------------------------------------
#include "Dbg.h"

//--- project modules used -----------------------------------------------------

//--- c-modules used -----------------------------------------------------------
#include <typeinfo>
#if !defined (WIN32)
#include <stdio.h>
#endif

class TestAccessManager : public AccessManager
{
public:
	TestAccessManager(const char *name) : AccessManager(name) {}
	IFAObject *Clone() const {
		return new TestAccessManager(fName);
	}
	virtual bool Validate(String uid) {
		return false;
	}
	virtual bool AuthenticateWeak(String uid, String passwd, String &newRole) {
		return false;
	}
	virtual bool AuthenticateStrong(String uid, String passwd, String otp, long window, String &newRole) {
		return false;
	}
	virtual bool ChangePassword(String uid, String newpwd, String oldpwd = "") {
		return false;
	}
	virtual bool ResetPassword(String uid) {
		return false;
	}
	virtual bool IsAllowed(String who, String entity) {
		return false;
	}
	virtual bool GetAllowedEntitiesFor(Anything who, Anything &allowed) {
		return false;
	}
};
RegisterAccessManager(TestAccessManager);

class MyAccessManager : public TestAccessManager
{
public:
	MyAccessManager(const char *name) : TestAccessManager(name) {}
	IFAObject *Clone() const {
		return new MyAccessManager(fName);
	}
};
RegisterAccessManager(MyAccessManager);

//---- AccessManagerModuleTest ----------------------------------------------------------------
AccessManagerModuleTest::AccessManagerModuleTest(TString name)
	: ConfiguredTestCase(name, "AccessManagerModuleTest")
{
	StartTrace(AccessManagerModuleTest.Ctor);
}

AccessManagerModuleTest::~AccessManagerModuleTest()
{
	StartTrace(AccessManagerModuleTest.Dtor);
}

// setup for this TestCase
void AccessManagerModuleTest::setUp ()
{
	StartTrace(AccessManagerModuleTest.setUp);

	ConfiguredTestCase::setUp();
	WDModule::Install(fConfig);
}

void AccessManagerModuleTest::tearDown ()
{
	StartTrace(AccessManagerModuleTest.tearDown);

	WDModule::Terminate(fConfig);
	ConfiguredTestCase::tearDown();
}

void AccessManagerModuleTest::testCase()
{
	StartTrace(AccessManagerModuleTest.testCase);
}

String GetName(RegisterableObject *o)
{
	String ret;
	o->GetName(ret);
	return ret;
}

void AccessManagerModuleTest::testInit()
{
	StartTrace(AccessManagerModuleTest.testInit);

	AccessManager *am_alpha = AccessManagerModule::GetAccessManager("alpha");
	t_assert(am_alpha != NULL);
	assertEquals("alpha", GetName(am_alpha));
	t_assertm((typeid(MyAccessManager) == typeid(*am_alpha)), typeid(*am_alpha).name());

	AccessManager *am_beta = AccessManagerModule::GetAccessManager("AccessManagerBeta");
	t_assert(am_beta != NULL);
	assertEquals("AccessManagerBeta", GetName(am_beta));
	t_assertm(typeid(MyAccessManager) == typeid(*am_beta), typeid(*am_beta).name());

	AccessManager *am_default = AccessManagerModule::GetAccessManager();
	t_assert(am_beta == am_default);

	assertEquals("bar", am_default->Lookup("foo").AsString(""));

	AccessManager *am_test = AccessManagerModule::GetAccessManager("TestAccessManager");
	t_assert(am_test != NULL);
	assertEquals("TestAccessManager", GetName(am_test));
	t_assertm(typeid(TestAccessManager) == typeid(*am_test), typeid(*am_test).name());

	AccessManager *am_gamma = AccessManagerModule::GetAccessManager("gamma");
	t_assert(am_gamma != NULL);
	assertEquals("gamma", GetName(am_gamma));
	t_assertm(typeid(TestAccessManager) == typeid(*am_gamma), typeid(*am_gamma).name());
}

void AccessManagerModuleTest::testFinis()
{
	StartTrace(AccessManagerModuleTest.testFinis);

	AccessManager *am_alpha = AccessManagerModule::GetAccessManager("alpha");
	t_assert(am_alpha != NULL);
	assertEquals("alpha", GetName(am_alpha));

	WDModule::Terminate(fConfig);

	am_alpha = AccessManagerModule::GetAccessManager("alpha");
	t_assert(am_alpha == 0);

	AccessManager *am = AccessManagerModule::GetAccessManager("TestAccessManager");
	t_assert(am != NULL);
	t_assertm(typeid(TestAccessManager) == typeid(*am), typeid(*am).name());
}

// builds up a suite of testcases, add a line for each testmethod
Test *AccessManagerModuleTest::suite ()
{
	StartTrace(AccessManagerModuleTest.suite);
	TestSuite *testSuite = new TestSuite;

	ADD_CASE(testSuite, AccessManagerModuleTest, testCase);
	ADD_CASE(testSuite, AccessManagerModuleTest, testInit);
	ADD_CASE(testSuite, AccessManagerModuleTest, testFinis);

	return testSuite;
}
