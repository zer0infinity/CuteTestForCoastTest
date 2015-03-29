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

#ifndef _AnythingLookupTest_H
#define _AnythingLookupTest_H

#include "cute.h"
#include "Anything.h"//lint !e537
class AnythingLookupTest {
public:
	static void runAllTests(cute::suite &s);
	void LookUp0Test();
	void LookUp1Test();
	void LookupPathByIndex();
	void EmptyLookup();
	void invPathLookup();
	void LookUpWithSpecialCharsTest();
	void LookupCaseSensitiveTest();
protected:
	Anything init5DimArray(long);
	void intLookupPathCheck(Anything &test, const char *path);
};

#endif		//ifndef _AnythingLookupTest_H
