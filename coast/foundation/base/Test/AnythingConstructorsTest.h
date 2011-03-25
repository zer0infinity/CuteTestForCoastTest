/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _AnythingConstructorsTest_H
#define _AnythingConstructorsTest_H

#include "TestCase.h"
#include "Anything.h"

//---- AnythingConstructorsTest -----------------------------------------------------------
//!testcases for Anything
class AnythingConstructorsTest : public TestFramework::TestCase
{
protected:
	Anything fString;
	Anything fLong;
	Anything fBool;
	Anything fFloat;
	Anything fDouble;
	Anything fDouble2;
	Anything fNull;

public:
	AnythingConstructorsTest (TString tstrName);

	virtual void	setUp ();
	static Test	*suite ();
	void		DefaultConstrTest();
	void		IntConstrTest();
	void		LongConstrTest();
	void		FloatConstrTest();
	void		DoubleConstr0Test();
	void		DoubleConstr1Test();
	void		CharStarConstrTest();
	void		CharStarLongConstr0Test();
	void		CharStarLongConstr1Test();
	void		CharStarLongConstr2Test();
	void		CharStarLongConstr3Test();
	void		StringConstrTest();
	void		VoidStarLenConstrTest();
	void		EmptyVoidStarLenConstrTest();
	void		IFAObjectStarConstrTest();
	void		AnythingConstrTest();
};

#endif		//ifndef _AnythingConstructorsTest_H
