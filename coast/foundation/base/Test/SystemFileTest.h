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
#include "boost/function.hpp"

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
	void dirFileListTest();
	void IStreamTest();
	void OStreamTest();
	void IOStreamTest();
	void statTests();
	void CWDTests();
	void MkRmDirTest();
	void MakeRemoveDirectoryTest();
	void MakeDirectoryTest();
#if !defined(WIN32)
	void SymbolicLinkTest();
#endif
	void MakeDirectoryExtendTest();
	void GetFileSizeTest();
	void BlocksLeftOnFSTest();

	void LoadConfigFileTest();

	void TimeTest();
private:
	void testGetFilePath(boost::function<String ()> func, const String& notFoundResult);
};

#endif /* SYSTEMFILETEST_H_ */
