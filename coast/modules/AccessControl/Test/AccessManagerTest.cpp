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
#include "AccessManagerTest.h"

//--- standard modules used ----------------------------------------------------
#include "Dbg.h"
#include "System.h"
#include "FileAccessControllerTests.h"

//--- project modules used -----------------------------------------------------

//--- c-modules used -----------------------------------------------------------
#if !defined (WIN32)
#include <stdio.h>
#endif

//---- AccessManagerTest ----------------------------------------------------------------
AccessManagerTest::AccessManagerTest(TString name)
	: ConfiguredTestCase(name, "AccessManagerTestConfig")
{
	StartTrace(AccessManagerTest.Ctor);
}

AccessManagerTest::~AccessManagerTest()
{
	StartTrace(AccessManagerTest.Dtor);
}

// setup for this TestCase
void AccessManagerTest::setUp ()
{
	StartTrace(AccessManagerTest.setUp);

	ConfiguredTestCase::setUp();
	// must install modules BEFORE we can use data accesses to create test files!
	WDModule::Install(fConfig["Config"]);
	// create test files
	t_assertm( FileCreator::CreateFile("WriteUserData", fConfig["InitData"]["UserFile"]), "Creation of test file failed" );
	t_assertm( FileCreator::CreateFile("WriteTokenData", fConfig["InitData"]["TokenFile"]), "Creation of test file failed" );
	t_assertm( FileCreator::CreateFile("WriteEntityData", fConfig["InitData"]["EntityFile"]), "Creation of test file failed" );
}

void AccessManagerTest::tearDown ()
{
	StartTrace(AccessManagerTest.tearDown);

	WDModule::Terminate(fConfig["Config"]);
	System::IO::unlink("config/FileTestUserDB.any");
	System::IO::unlink("config/FileTestActerDB.any");
	System::IO::unlink("config/FileTestRightsDB.any");
	ConfiguredTestCase::tearDown();
}

void AccessManagerTest::doTestAccessManager(ROAnything config, AccessManager *am)
{
	StartTrace(AccessManagerTest.doTestAccessManager);

	bool res;
	ROAnything subconf, testconf;
	String newRole;
	Anything allowedEntities;

	// -- Validate tests
	subconf = config["Validate"];
	for (long i = 0; i < subconf.GetSize(); i++) {
		Trace("test case name = " << subconf.SlotName(i));
		testconf = subconf[i];
		res = am->Validate(testconf["uid"].AsString());
		assertEqual(testconf["result"].AsBool(), res);
	}

	// -- AuthenticateWeak tests
	subconf = config["AuthenticateWeak"];
	for (long i2 = 0; i2 < subconf.GetSize(); i2++) {
		Trace("test case name = " << subconf.SlotName(i2));
		testconf = subconf[i2];
		res = am->AuthenticateWeak(
				  testconf["uid"].AsString(),
				  testconf["pwd"].AsString(),
				  newRole
			  );
		assertEqual(testconf["result"].AsBool(false), res);
		assertEqual(testconf["resultRole"].AsString(""), newRole);
	}

	// -- AuthenticateStrong tests
	subconf = config["AuthenticateStrong"];
	for (long i3 = 0; i3 < subconf.GetSize(); i3++) {
		Trace("test case name = " << subconf.SlotName(i3));
		testconf = subconf[i3];
		res = am->AuthenticateStrong(
				  testconf["uid"].AsString(),
				  testconf["pwd"].AsString(),
				  testconf["otp"].AsString(),
				  testconf["window"].AsLong(0),
				  newRole
			  );
		assertEqual(testconf["result"].AsBool(false), res);
		assertEqual(testconf["resultRole"].AsString(""), newRole);
	}

	// -- ChangePassword tests
	subconf = config["ChangePassword"];
	for (long i4 = 0; i4 < subconf.GetSize(); i4++) {
		Trace("test case name = " << subconf.SlotName(i4));
		testconf = subconf[i4];
		res = am->ChangePassword(
				  testconf["uid"].AsString(),
				  testconf["newpwd"].AsString(),
				  testconf["oldpwd"].AsString()
			  );
		assertEqual(testconf["result"].AsBool(false), res);
		// does login work after change?
		if (res) {
			String dummyRole;
			t_assert( am->AuthenticateWeak(
						  testconf["uid"].AsString(),
						  testconf["newpwd"].AsString(),
						  dummyRole
					  ));
		}
	}

	// -- ResetPassword tests
	subconf = config["ResetPassword"];
	for (long i5 = 0; i5 < subconf.GetSize(); i5++) {
		Trace("test case name = " << subconf.SlotName(i5));
		testconf = subconf[i5];
		res = am->ResetPassword(
				  testconf["uid"].AsString()
			  );
		assertEqual(testconf["result"].AsBool(false), res);
		// does login work after reset?
		if (res) {
			String dummyRole;
			t_assert( am->AuthenticateWeak(
						  testconf["uid"].AsString(),
						  testconf["uid"].AsString(),
						  dummyRole
					  ));
		}
	}

	// -- IsAllowed tests
	subconf = config["IsAllowed"];
	for (long i6 = 0; i6 < subconf.GetSize(); i6++) {
		Trace("test case name = " << subconf.SlotName(i6));
		testconf = subconf[i6];
		res = am->IsAllowed(
				  testconf["uid"].AsString(),
				  testconf["entity"].AsString()
			  );
		assertEqual(testconf["result"].AsBool(false), res);
	}

	// -- GetAllowed tests
	subconf = config["GetAllowed"];
	for (long i7 = 0; i7 < subconf.GetSize(); i7++) {
		Trace("test case name = " << subconf.SlotName(i7));
		testconf = subconf[i7];
		allowedEntities = Anything();
		res = am->GetAllowedEntitiesFor(
				  testconf["uid"].AsString(),
				  allowedEntities
			  );
		assertEqual(testconf["result"].AsBool(false), res);
		assertAnyEqual(testconf["resultAllowed"], allowedEntities);
	}

}

void AccessManagerTest::testRegularAccessManagers()
{
	StartTrace(AccessManagerTest.testRegularAccessManagers);

	AccessManager *am;

	// run tests for registered/configured access managers (have names)
	FOREACH_ENTRY("Tests", caseConfig, name) {
		Trace("Running tests for '" << name << "' access manager ...");
		if ( fConfig["RunOnly"].GetSize() == 0 || fConfig["RunOnly"].Contains(name) ) {
			am = AccessManagerModule::GetAccessManager(name);
			if (t_assert(am)) {
				doTestAccessManager(caseConfig, am);
			}
		} else {
			Trace("[skipped]");
		}
	}

	// check if the default access manager exists (no name given)
	am = AccessManagerModule::GetAccessManager();
	t_assertm(am, "default access manager not found!");
}

// builds up a suite of testcases, add a line for each testmethod
Test *AccessManagerTest::suite ()
{
	StartTrace(AccessManagerTest.suite);
	TestSuite *testSuite = new TestSuite;

	ADD_CASE(testSuite, AccessManagerTest, testRegularAccessManagers);

	return testSuite;
}
