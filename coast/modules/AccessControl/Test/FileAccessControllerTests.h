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
#include "ConfiguredTestCase.h"

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
class FileAccessControllerTests : public ConfiguredTestCase
{
public:
	//--- constructors
	FileAccessControllerTests(TString name) : ConfiguredTestCase(name, "FileAccessControllerTestsConfig") {}
	FileAccessControllerTests(TString name, const char *file) : ConfiguredTestCase(name, file) {}
	~FileAccessControllerTests() {};

	//--- public api
	static bool createTestFile(String daName, ROAnything data);

	//! builds up a suite of ConfiguredTestCases for this test
	static Test *suite ();

	//! sets the environment for this test
	void setUp ();

	//! deletes the environment for this test
	void tearDown ();

	// general testing methods
	void doTestUDAC(UserDataAccessController *udac);
	void doTestTDAC(TokenDataAccessController *tdac);
	void doTestEDAC(EntityDataAccessController *edac);

	//! Tests functionality of FileUDAC (FileUserDataAccessController)
	void testFileUDAC();

	//! Tests functionality of FileTDAC (FileTokenDataAccessController)
	void testFileTDAC();

	//! Tests functionality of FileEDAC (FileEntityDataAccessController)
	void testFileEDAC();

};

#endif
