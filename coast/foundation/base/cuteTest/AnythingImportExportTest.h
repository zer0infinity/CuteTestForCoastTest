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

#ifndef _AnythingImportExportTest_H
#define _AnythingImportExportTest_H

#include "AssertionAnything.h"
#include "cute.h"
#include "Anything.h"//lint !e537
#include "SystemFile.h"
#include "StringStream.h"

class AnythingImportExportTest : public Assertion {
public:
	static void runAllTests(cute::suite &s);

	void ImportTest();

	void ReadFailsTest();
	void WriteRead0Test();
	void WriteRead1Test();
	void WriteRead5Test();
	void WriteRead7Test();
	void WriteRead8Test();
	void AnyIncludeTest();

	void RefSlotTest();
	void RefBug227Test();
	void RefBug231Test();
	void RefBug220Test();

protected:
	Anything init5DimArray(long);
	bool check5DimArray(Anything &, Anything &, long);
};

#endif		//ifndef _AnythingImportExportTest_H
