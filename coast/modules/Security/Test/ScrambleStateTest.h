/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _ScrambleStateTest_H
#define _ScrambleStateTest_H

//---- TestCase include -------------------------------------------------
#include "TestCase.h"
#include "Anything.h"
//---- ScrambleStateTest ----------------------------------------------------------
//!single line description of the class
//! further explanation of the purpose of the class
//! this may contain <B>HTML-Tags</B>
//! ...
class ScrambleStateTest : public TestCase
{
public:
	//--- constructors
	ScrambleStateTest(TString name);
	~ScrambleStateTest();

	static Test *suite ();

	void setUp();

	void ScrambleUnscrambleTest();
	// Test against an approved, predefined scramble and unscramble result
	void DoReferenceUncsramble(const Anything &config);

protected:
	//--- subclass api
	void DoScrambleTest(const char *configname);

	//--- member variables declaration
	Anything fStdContextAny;

private:
	// use careful, you inhibit subclass use
	//--- private class api

	//--- private member variables

};

#endif
