/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

//--- interface include --------------------------------------------------------
#include "MockAccessControllerTests.h"

//--- project modules used -----------------------------------------------------
#include "SimpleAccessControllers.h"

//--- test modules used --------------------------------------------------------
#include "TestSuite.h"

//--- standard modules used ----------------------------------------------------
#include "Dbg.h"
#include "WDModule.h"

//--- c-modules used -----------------------------------------------------------

void MockAccessControllerTests::setUp ()
{
	StartTrace(MockAccessControllerTests.setUp);

	ConfiguredTestCase::setUp();
	WDModule::Install(fConfig["Config"]);
}

void MockAccessControllerTests::tearDown ()
{
	StartTrace(MockAccessControllerTests.tearDown);

	WDModule::Terminate(fConfig["Config"]);
	ConfiguredTestCase::tearDown();
}

void MockAccessControllerTests::testMockUDAC()
{
	StartTrace(MockAccessControllerTests.testMockUDAC);

	MockUDAC *mudac = static_cast<MockUDAC *>(UserDataAccessController::FindUserDataAccessController("MockUserData"));
	doTestUDAC(mudac);

	// check results
	assertAnyEqual(fConfig["Results"]["MockUDACTest"], mudac->GetMockedState());
}

void MockAccessControllerTests::testMockTDAC()
{
	StartTrace(MockAccessControllerTests.testMockTDAC);

	MockTDAC *mtdac = static_cast<MockTDAC *>(TokenDataAccessController::FindTokenDataAccessController("MockTokenData"));
	doTestTDAC(mtdac);

	// check results
	assertAnyEqual(fConfig["Results"]["MockTDACTest"], mtdac->GetMockedState());
}

void MockAccessControllerTests::testMockEDAC()
{
	StartTrace(MockAccessControllerTests.testMockEDAC);

	MockEDAC *medac = static_cast<MockEDAC *>(EntityDataAccessController::FindEntityDataAccessController("MockEntityData"));
	doTestEDAC(medac);

	// check results
	assertAnyEqual(fConfig["Results"]["MockEDACTest"], medac->GetMockedState());
}

Test *MockAccessControllerTests::suite ()
{
	StartTrace(FileUDACTest.suite);
	TestSuite *testSuite = new TestSuite;

	ADD_CASE(testSuite, MockAccessControllerTests, testMockUDAC);
	ADD_CASE(testSuite, MockAccessControllerTests, testMockTDAC);
	ADD_CASE(testSuite, MockAccessControllerTests, testMockEDAC);

	return testSuite;
}
