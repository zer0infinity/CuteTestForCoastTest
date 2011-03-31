/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#include "AccessManagerTest.h"
#include "AccessManager.h"
#include "TestSuite.h"
#include "AnyIterators.h"
#include "FileAccessControllerTests.h"
#if !defined (WIN32)
#include <stdio.h>
#endif

//---- AccessManagerTest ----------------------------------------------------------------
AccessManagerTest::AccessManagerTest(TString tstrName)
	: TestCaseType(tstrName)
{
	StartTrace(AccessManagerTest.AccessManagerTest);
}

TString AccessManagerTest::getConfigFileName()
{
	return "AccessManagerTestConfig";
}

AccessManagerTest::~AccessManagerTest()
{
	StartTrace(AccessManagerTest.Dtor);
}

void AccessManagerTest::setUp ()
{
	StartTrace(AccessManagerTest.setUp);
	// create test files
	t_assertm( FileCreator::CreateFile("WriteUserData", GetConfig()["InitData"]["UserFile"]), "Creation of test file failed" );
	t_assertm( FileCreator::CreateFile("WriteTokenData", GetConfig()["InitData"]["TokenFile"]), "Creation of test file failed" );
	t_assertm( FileCreator::CreateFile("WriteEntityData", GetConfig()["InitData"]["EntityFile"]), "Creation of test file failed" );
}

void AccessManagerTest::tearDown ()
{
	StartTrace(AccessManagerTest.tearDown);
	Coast::System::IO::unlink("config/FileTestUserDB.any");
	Coast::System::IO::unlink("config/FileTestActerDB.any");
	Coast::System::IO::unlink("config/FileTestRightsDB.any");
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
		String strUid = testconf["uid"].AsString();
		res = am->Validate(strUid);
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

void AccessManagerTest::RegularAccessManagersTest()
{
	StartTrace(AccessManagerTest.RegularAccessManagersTest);

	AccessManager *am = NULL;

	// run tests for registered/configured access managers (have names)
	ROAnything caseConfig;
	AnyExtensions::Iterator<ROAnything, ROAnything, TString> aEntryIterator(GetTestCaseConfig());
	while ( aEntryIterator.Next(caseConfig) ) {
		TString strName;
		aEntryIterator.SlotName(strName);
		Trace("Running tests for '" << strName << "' access manager ...");
		if ( GetConfig()["RunOnly"].GetSize() == 0 || GetConfig()["RunOnly"].Contains(strName) ) {
			am = AccessManagerModule::GetAccessManager(strName);
			if (t_assertm(am, TString("expected AccessManager [") << strName << "] to be registered")) {
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
	ADD_CASE(testSuite, AccessManagerTest, RegularAccessManagersTest);
	return testSuite;
}
