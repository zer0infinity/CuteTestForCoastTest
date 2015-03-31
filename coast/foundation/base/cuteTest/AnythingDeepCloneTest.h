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

#ifndef _AnythingDeepCloneTest_H
#define _AnythingDeepCloneTest_H

#include "cute.h"
#include "Anything.h"//lint !e537
#include "SystemFile.h"
#include "Tracer.h"
#include "StringStream.h"

class AnythingDeepCloneTest {
public:
	static void runAllTests(cute::suite &s);

	void DeepClone0Test();
	void DeepClone1Test();
	void DeepClone2Test();
	void DeepClone3Test();
	void DeepClone4Test();
	void DeepClone5Test();
	//-- new test check for ref integrity with deepclone
	void DeepCloneWithRef();
	void DeepCloneBug232Test();
protected:
	Anything init5DimArray(long);
	bool check5DimArray(Anything &, Anything &, long);
};

#endif		//ifndef _AnythingDeepCloneTest_H
