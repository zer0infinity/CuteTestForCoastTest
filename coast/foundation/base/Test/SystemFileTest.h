/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef SYSTEMFILETEST_H_
#define SYSTEMFILETEST_H_

//---- baseclass include -------------------------------------------------
#include "FoundationTestTypes.h"

//---- SystemFileTest ----------------------------------------------------------
//!testcases for SystemFile
class SystemFileTest : public TestFramework::TestCaseWithConfig
{
public:
	//!constructors
	SystemFileTest(TString tstrName);
	~SystemFileTest();

	//!builds up a suite of testcases for this test
	static Test *suite ();

	//!tests pathlist accessor
	void initPathTest();
	void pathListTest();
	void rooDirTest();
	void IsAbsolutePathTest();
	void ResolvePathTest();
	void OpenStreamTest();
	void OpenStreamWithSearchTest();
	void OpenOStreamTest();
	void OpenIStreamTest();
	void GetFilePathTest();
	void GetFilePathOrInputTest();
	void GetFilePathTest2();
	void dirFileListTest();
	void IStreamTest();
	void OStreamTest();
	void IOStreamTest();
	void statTests();
	void CWDTests();
	void MkRmDirTest();
	void MakeRemoveDirectoryTest();
	void MakeDirectoryTest();
	void SymbolicLinkTest();
	void MakeDirectoryExtendTest();
	void GetFileSizeTest();
	void BlocksLeftOnFSTest();

	void LoadConfigFileTest();

	void TimeTest();
};

#endif /* SYSTEMFILETEST_H_ */
