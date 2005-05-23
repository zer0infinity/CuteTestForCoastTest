/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _URLUtilsTest_h_
#define _URLUtilsTest_h_

#include "TestCase.h"

//---- URLUtilsTest -----------------------------------------------------------
//!testcases for URLUtils
class URLUtilsTest : public TestCase
{
public:
	URLUtilsTest(TString tstrName);
	virtual ~URLUtilsTest();

	static Test *suite ();
	void setUp ();
	void urlDecodeTest();
	void ExhaustiveUrlDecodeTest();
	void ExhaustiveHTMLDecodeTest();
	void urlEncodeTest();
	void strHasEscapesTest();
	void PairTest();
	void SplitTest();
	int  CalcEncodedLength(int);
	void EnDecodeBase64Test();
	void EncodeTest();
	void DecodeAllTest();
	void HandleURITest();
	void HandleURI2Test();
	void TrimBlanksTest();
	void HTMLEscapeTest();
	void RemoveUnwantedCharsTest();
	void HTMLDecodeTest();
	void CheckUrlEncodingTest();
	void CheckUrlArgEncodingTest();
	void CheckUrlPathContainsUnsafeCharsTest();

protected:

};

#endif
