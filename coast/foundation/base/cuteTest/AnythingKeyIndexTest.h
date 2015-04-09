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

#ifndef _AnythingKeyIndexTest_H
#define _AnythingKeyIndexTest_H

#include "cute.h"
#include "Anything.h"//lint !e537
class AnythingKeyIndexTest {
	Anything fArray;
	Anything fSequence;
public:
	AnythingKeyIndexTest();
	static void runAllTests(cute::suite &s);

	void SimpleRemove();
	void RemoveInvKeys();

	void IndexAccess();

	void KeyAccess0();
	void KeyAccess1();
	void KeyAccess2();
	void MixKeysAndArray();
	void Recursive();

	void EmptyAccess0();
	void EmptyAccess1();

protected:
	Anything init5DimArray(long);
	bool check5DimArray(Anything &, Anything &, long);
};

#endif		//ifndef _AnythingKeyIndexTest_H
