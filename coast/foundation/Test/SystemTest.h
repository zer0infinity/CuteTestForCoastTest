/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _SystemTest_H
#define _SystemTest_H

//---- baseclass include -------------------------------------------------
#include "FoundationTestTypes.h"

//---- SystemTest ----------------------------------------------------------
//!testcases for System
class SystemTest : public TestFramework::TestCaseWithConfig
{
public:
	//!constructors
	SystemTest(TString tstrName);
	~SystemTest();

	//!builds up a suite of testcases for this test
	static Test *suite ();

	//!tests pathlist accessor
	void initPathTest();
	void pathListTest();
	void rooDirTest();
	void IsAbsolutePathTest();
	void ResolvePathTest();
	void openStreamTest();
	void getFilePathTest();
	void dirFileListTest();
	void GetProcessEnvironmentTest();
	void IStreamTest();
	void OStreamTest();
	void IOStreamTest();
	void statTests();
	void CWDTests();
	void MkRmDirTest();
	void MakeRemoveDirectoryTest();
	void MakeDirectoryTest();
	void GetFileSizeTest();
	void BlocksLeftOnFSTest();
	void LockFileTest();

	void DoSingleSelectTest();
	void MicroSleepTest();
	//!storage tests
	void allocFreeTests();
	void LoadConfigFileTest();

	void TimeTest();
};

#endif
