/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _AnyBuiltInSortTest_H
#define _AnyBuiltInSortTest_H

//---- baseclass include -------------------------------------------------
#include "TestCase.h"

//---- AnyBuiltInSortTest ----------------------------------------------------------
//! <B>really brief class description</B>
/*!
further explanation of the purpose of the class
this may contain <B>HTML-Tags</B>
*/
class AnyBuiltInSortTest : public TestFramework::TestCase
{
public:
	//--- constructors

	/*! \param name name of the test */
	AnyBuiltInSortTest(TString tstrName);

	//! destroys the test case
	~AnyBuiltInSortTest();

	//--- public api

	//! builds up a suite of testcases for this test
	static Test *suite ();

	//! empty any can be sorted
	void SortEmpty();
	//! one element always sorted
	void SortOne();
	//! two elements might need to be swapped
	void SortTwo();
	//! three elements  need to be sorted
	void SortThree();

	//! many elements  need to be sorted
	void SortMany();
	//! many elements  need to be sorted by a String Value
	void SortManyStringValues();

	//! many elements  need to be sorted,
	//! without keys, elements should keep their order
	void SortIsStable();

protected:
	bool checksorted(const Anything &a, bool shouldfail = false);
	bool checksortedbyvalue(const Anything &a, bool shouldfail = false);
};

#endif