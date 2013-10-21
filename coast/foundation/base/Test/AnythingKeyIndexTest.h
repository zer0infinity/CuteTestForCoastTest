/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _AnythingKeyIndexTest_H
#define _AnythingKeyIndexTest_H

#include "TestCase.h"//lint !e537
#include "Anything.h"//lint !e537
class AnythingKeyIndexTest: public testframework::TestCase {
	Anything fArray;
	Anything fSequence;
public:
	AnythingKeyIndexTest(TString tname) :
		TestCaseType(tname) {
	}
	virtual void setUp();
	static Test *suite();

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
