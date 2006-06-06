/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

//--- interface include --------------------------------------------------------
#include "FileAccessControllerTests.h"

//--- project modules used -----------------------------------------------------
#include "SimpleAccessControllers.h"

//--- test modules used --------------------------------------------------------
#include "TestSuite.h"

//--- standard modules used ----------------------------------------------------
#include "Dbg.h"
#include "System.h"
#include "DataAccess.h"

//--- c-modules used -----------------------------------------------------------

bool FileCreator::CreateFile(String dataAccessName, ROAnything data)
{
	Context c;
	c.GetTmpStore()["FileContent"] = data.DeepClone();
	return DataAccess(dataAccessName).StdExec(c);
}

void FileAccessControllerTests::setUp ()
{
	StartTrace(FileAccessControllerTests.setUp);
	WDModule::Install(GetConfig()["Config"]);
}

void FileAccessControllerTests::tearDown ()
{
	StartTrace(FileUDACTest.tearDown);
	WDModule::Terminate(GetConfig()["Config"]);
}

// -------------------- UDAC Tests ----------------------
void FileAccessControllerTests::doTestUDAC(UserDataAccessController *udac)
{
	StartTrace(FileAccessControllerTests.doTestUDAC);

	if (t_assert(udac != NULL)) {
		String tester("tester"), unknown("unknownUser"), empty("");
		Anything dummy;

		t_assert(udac->Exists(tester));
		t_assert(!udac->Exists(unknown));

		assertEqual(String("000100002"), udac->GetTokenId(tester));
		assertEqual(empty, udac->GetTokenId(unknown));

		assertEqual(String("secret"), udac->GetPassword(tester));
		assertEqual(empty, udac->GetPassword(unknown));

		t_assert(udac->SetPassword(tester, "verySecret"));
		assertEqual(String("verySecret"), udac->GetPassword(tester));
		t_assert(!udac->SetPassword(unknown, "doesntMatter"));

		t_assert(udac->ResetPassword(tester));
		assertEqual(tester, udac->GetPassword(tester));
		t_assert(!udac->ResetPassword(unknown));

		t_assert(!udac->GetGroups(unknown, dummy));
		t_assert(udac->GetGroups(tester, dummy));
		t_assert(dummy.Contains("superhero"));
		t_assert(dummy.Contains("deadserious"));

		dummy.Remove(dummy.FindValue("deadserious"));
		dummy.Append("singsong");
		t_assert(udac->SetGroups(tester, dummy));
		t_assert(!udac->SetGroups(unknown, dummy));
	}
}

void FileAccessControllerTests::FileUDACTest()
{
	StartTrace(FileAccessControllerTests.FileUDACTest);

	// create test file
	t_assertm( FileCreator::CreateFile("WriteUserData", GetConfig()["InitData"]["UDACTest"]), "Creation of test file failed"	);

	// do generic tests
	UserDataAccessController *fudac = UserDataAccessController::FindUserDataAccessController("UserDataFile");
	doTestUDAC(fudac);

	// check results
	Context c;
	t_assert(DataAccess("ReadUserData").StdExec(c));
	TraceAny(c.GetTmpStore()["FileContent"], "FileContentAfterTest");
	assertAnyEqual(GetConfig()["FileContent"]["UDACTest"], c.GetTmpStore()["FileContent"]);

	// clean up
	System::IO::unlink("config/FileTestUserDB.any");
}

// -------------------- TDAC Tests ----------------------
void FileAccessControllerTests::doTestTDAC(TokenDataAccessController *tdac)
{
	StartTrace(FileAccessControllerTests.doTestTDAC);

	if (t_assert(tdac != NULL)) {
		String tid("000100002"), unknown("399249002"), empty(""), expectedSeed("e5a4103be54395b743c1c70365d47603");
		unsigned long count, expectedCount = 612314215;
		long increment = 23;

		assertEqualm(expectedCount, tdac->GetCount(tid), "must not be 0");
		assertEqualm(0, tdac->GetCount(unknown), "should be 0");

		assertEqual(expectedSeed, tdac->GetSeed(tid));
		assertEqual(empty, tdac->GetSeed(unknown));

		t_assert(tdac->IncCount(tid, increment));
		t_assert( ( count = tdac->GetCount(tid) ) > 0);
		expectedCount += increment;
		assertEqual(expectedCount, count);

		t_assert(!tdac->IncCount(unknown, increment));
		assertEqual(expectedCount, tdac->GetCount(tid));
	}
}

void FileAccessControllerTests::FileTDACTest()
{
	StartTrace(FileAccessControllerTests.FileTDACTest);

	// create test file
	t_assertm( FileCreator::CreateFile("WriteTokenData", GetConfig()["InitData"]["TDACTest"]), "Creation of test file failed"	);

	// do generic tests
	TokenDataAccessController *ftdac = TokenDataAccessController::FindTokenDataAccessController("TokenDataFile");
	doTestTDAC(ftdac);

	// check results
	Context c;
	t_assert(DataAccess("ReadTokenData").StdExec(c));
	TraceAny(c.GetTmpStore()["FileContent"], "FileContentAfterTest");
	assertAnyEqual(GetConfig()["FileContent"]["TDACTest"], c.GetTmpStore()["FileContent"]);

	// clean up
	System::IO::unlink("config/FileTestActerDB.any");
}

// -------------------- EDAC Tests ----------------------
void FileAccessControllerTests::doTestEDAC(EntityDataAccessController *edac)
{
	StartTrace(FileAccessControllerTests.doTestEDAC);

	if (t_assert(edac != NULL)) {
		String user("user"), guest("guest"), unknown("unknown"), coffee("coffee");
		Anything groups;
		groups.Append("coffee");
		groups.Append("movies");
		Anything allowed;
		MetaThing empty;

		// empty group
		t_assert(edac->GetAllowedEntitiesForGroup(guest, allowed));
		t_assert(!allowed.IsNull());
		assertEqual(0L, allowed.GetSize());

		// single group
		t_assert(edac->GetAllowedEntitiesForGroup(user, allowed));
		assertEqual(2L, allowed.GetSize());
		t_assert(allowed.Contains("changePW"));
		t_assert(allowed.Contains("resetPW"));

		// group list
		t_assert(edac->GetAllowedEntitiesForGroups(groups, allowed));
		assertEqual(5L, allowed.GetSize());
		t_assert(allowed.Contains("tv"));
		t_assert(allowed.Contains("espressomachine"));
		t_assert(!allowed.Contains("changePW"));

		t_assert(edac->GetAllowedEntitiesForGroups(empty, allowed));
		t_assert(!allowed.IsNull());
		assertEqual(0L, allowed.GetSize());

		// specific tests
		t_assert(edac->IsAllowed(user, "resetPW"));
		t_assert(edac->IsAllowed(coffee, "cookiejar"));
		t_assert(!edac->IsAllowed(guest, "everything"));

		// tests with unknown
		t_assert(!edac->IsAllowed(unknown, "format c:"));
		t_assert(!edac->GetAllowedEntitiesForGroup(unknown, allowed));
		empty.Append(coffee);
		empty.Append(unknown);
		t_assert(!edac->GetAllowedEntitiesForGroups(empty, allowed));
	}
}

void FileAccessControllerTests::FileEDACTest()
{
	StartTrace(FileAccessControllerTests.FileEDACTest);

	// create test file
	t_assertm( FileCreator::CreateFile("WriteEntityData", GetConfig()["InitData"]["EDACTest"]), "Creation of test file failed"	);

	// do generic tests
	EntityDataAccessController *fedac = EntityDataAccessController::FindEntityDataAccessController("EntityDataFile");
	doTestEDAC(fedac);

	// check results
	Context c;
	t_assert(DataAccess("ReadEntityData").StdExec(c));
	TraceAny(c.GetTmpStore()["FileContent"], "FileContentAfterTest");
	assertAnyEqual(GetConfig()["FileContent"]["EDACTest"], c.GetTmpStore()["FileContent"]);

	// clean up
	System::IO::unlink("config/FileTestRightsDB.any");
}

// ------------------------- suite ------------------------------
Test *FileAccessControllerTests::suite ()
{
	StartTrace(FileAccessControllerTests.suite);
	TestSuite *testSuite = new TestSuite;
	ADD_CASE(testSuite, FileAccessControllerTests, FileUDACTest);
	ADD_CASE(testSuite, FileAccessControllerTests, FileTDACTest);
	ADD_CASE(testSuite, FileAccessControllerTests, FileEDACTest);
	return testSuite;
}
