/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _FileAccessControllerTests_H
#define _FileAccessControllerTests_H

//---- baseclass include -------------------------------------------------
#include "WDBaseTestPolicies.h"

class UserDataAccessController;
class TokenDataAccessController;
class EntityDataAccessController;

//---- FileCreator ----------------------------------------------------------
//! <B>Writea a given Anything to a file.</B>
class FileCreator
{
public:
	static bool CreateFile(String dataAccessName, ROAnything data);
};

//---- FileAccessControllerTests ----------------------------------------------------------
//! <B>Tests functionality of FileAccessControllers (FileUDAC, FileTDAC, FileEDAC)
class FileAccessControllerTests : public TestFramework::TestCaseWithGlobalConfigDllAndModuleLoading
{
public:
	//--- constructors
	FileAccessControllerTests(TString tstrName)
		: TestCaseType(tstrName) {}
	~FileAccessControllerTests() {};

	TString getConfigFileName() {
		return "FileAccessControllerTestsConfig";
	}
	//--- public api
	static bool createTestFile(String daName, ROAnything data);

	//! builds up a suite of tests
	static Test *suite ();

	// general testing methods
	void doTestUDAC(UserDataAccessController *udac);
	void doTestTDAC(TokenDataAccessController *tdac);
	void doTestEDAC(EntityDataAccessController *edac);

	//! Tests functionality of FileUDAC (FileUserDataAccessController)
	void FileUDACTest();

	//! Tests functionality of FileTDAC (FileTokenDataAccessController)
	void FileTDACTest();

	//! Tests functionality of FileEDAC (FileEntityDataAccessController)
	void FileEDACTest();
};

#endif
