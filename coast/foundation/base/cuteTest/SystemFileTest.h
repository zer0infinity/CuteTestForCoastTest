/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * Copyright (c) 2015, David Tran, Faculty of Computer Science,
 * University of Applied Sciences Rapperswil (HSR),
 * 8640 Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef SYSTEMFILETEST_H_
#define SYSTEMFILETEST_H_

#include "cute.h"
#include "boost/function.hpp"
#include "Tracer.h"
#include "StringStream.h"
#include "SystemFile.h"
#include "SystemBase.h"
#include "../anythingoptional/AnyIterators.h"

class SystemFileTest {
	void testGetFilePath(boost::function<String()> func, const String& notFoundResult);
public:
	static void runAllTests(cute::suite &s);
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
	void SymbolicLinkTest();
	void MakeDirectoryExtendTest();
	void GetFileSizeTest();
	void BlocksLeftOnFSTest();
	void LoadConfigFileTest();
	void TimeTest();
};

#endif /* SYSTEMFILETEST_H_ */
