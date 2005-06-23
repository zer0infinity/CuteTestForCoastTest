/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _BlowfishTest_h_
#define _BlowfishTest_h_

//-*-Mode: C++;-*-

/* Purpose: Test the interface of the BlowfishScrambler
   Initial Autor: Patru
*/

#include "TestCase.h"
#define NUM_TESTS 34
//---- BlowfishTest -----------------------------------------------------------
class Scrambler;

class BlowfishTest : public TestCase
{
public:
	BlowfishTest(TString tstrName);
	virtual ~BlowfishTest();

	virtual void setUp ();
	static Test *suite ();

	void scrambleUnscramble();
	void differentKey();
	void differentLength();
	void textWithNull();
	void modifyScramble();
	void rawECBCyphers();

	void paddingTest();
	void scrambleLongerStringTheSame();
//!tests for CBC blowfish encoding
	void paddingFrontTest();
	void paddingIsRandomTest();
	void scrambleSameKeyButDifferentIvec();
	void scrambleLongerStringDifferent();
	void scrambleCarefullyPaddedString();
	void cbcCrossPlatform();

protected:
	void DoScrambleUnscramble(Scrambler &bfscrambler, Scrambler &bfscrambler2);
	void DoDifferentKey(Scrambler &bfscrambler, Scrambler &bfscrambler2);

};

#endif
