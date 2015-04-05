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

#ifndef _AnythingConstructorsTest_H
#define _AnythingConstructorsTest_H

#include "cute.h"
#include "Anything.h"

class AnythingConstructorsTest {
protected:
	Anything fString;
	Anything fLong;
	Anything fBool;
	Anything fFloat;
	Anything fDouble;
	Anything fDouble2;
	Anything fNull;

public:
	AnythingConstructorsTest();
	static void runAllTests(cute::suite &s);
	void DefaultConstrTest();
	void IntConstrTest();
	void LongConstrTest();
	void FloatConstrTest();
	void DoubleConstr0Test();
	void DoubleConstr1Test();
	void CharStarConstrTest();
	void CharStarLongConstr0Test();
	void CharStarLongConstr1Test();
	void CharStarLongConstr2Test();
	void CharStarLongConstr3Test();
	void StringConstrTest();
	void VoidStarLenConstrTest();
	void EmptyVoidStarLenConstrTest();
	void IFAObjectStarConstrTest();
	void AnythingConstrTest();
};

#endif		//ifndef _AnythingConstructorsTest_H
