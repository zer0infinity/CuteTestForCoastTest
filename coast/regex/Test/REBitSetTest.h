/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _REBitSetTest_H
#define _REBitSetTest_H

//---- baseclass include -------------------------------------------------
#include "TestCase.h"

//---- REBitSetTest ----------------------------------------------------------
//!TestCases description
class REBitSetTest : public TestCase
{
public:
	//--- constructors

	//!TestCase constructor
	//! \param name name of the test
	REBitSetTest(TString name);

	//!destroys the test case
	~REBitSetTest();

	//--- public api

	//!builds up a suite of testcases for this test
	static Test *suite ();

	//!sets the environment for this test
	void setUp ();

	//!deletes the environment for this test
	void tearDown ();

	//!test all operators
	void ManyTests();
	//!test pathological loop case in REBitSet::Set()
	void SetTo255();
	//!test Set(predicate) method
	void TestPredicateSet();

	//!test pathological loop case in REBitSet::Set()
	void GeneratePosixSet(ostream &cppfile, const char *name, REBitSet::Predicate predicate);
	void GeneratePosixSets();
};

#endif
