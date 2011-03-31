/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#include "MockAccessControllerTests.h"
#include "SimpleAccessControllers.h"
#include "TestSuite.h"
#include "Dbg.h"
#include "WDModule.h"

void MockAccessControllerTests::MockUDACTest()
{
	StartTrace(MockAccessControllerTests.MockUDACTest);

	MockUDAC *mudac = static_cast<MockUDAC *>(UserDataAccessController::FindUserDataAccessController("MockUserData"));
	doTestUDAC(mudac);

	// check results
	assertAnyEqual(GetConfig()["Results"]["MockUDACTest"], mudac->GetMockedState());
}

void MockAccessControllerTests::MockTDACTest()
{
	StartTrace(MockAccessControllerTests.MockTDACTest);

	MockTDAC *mtdac = static_cast<MockTDAC *>(TokenDataAccessController::FindTokenDataAccessController("MockTokenData"));
	doTestTDAC(mtdac);

	// check results
	assertAnyEqual(GetConfig()["Results"]["MockTDACTest"], mtdac->GetMockedState());
}

void MockAccessControllerTests::MockEDACTest()
{
	StartTrace(MockAccessControllerTests.MockEDACTest);

	MockEDAC *medac = static_cast<MockEDAC *>(EntityDataAccessController::FindEntityDataAccessController("MockEntityData"));
	doTestEDAC(medac);

	// check results
	assertAnyEqual(GetConfig()["Results"]["MockEDACTest"], medac->GetMockedState());
}

Test *MockAccessControllerTests::suite ()
{
	StartTrace(FileUDACTest.suite);
	TestSuite *testSuite = new TestSuite;
	ADD_CASE(testSuite, MockAccessControllerTests, MockUDACTest);
	ADD_CASE(testSuite, MockAccessControllerTests, MockTDACTest);
	ADD_CASE(testSuite, MockAccessControllerTests, MockEDACTest);
	return testSuite;
}
