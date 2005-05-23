/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _HiddenFieldRendererTest_h_
#define _HiddenFieldRendererTest_h_

//-*-Mode: C++;-*-

#include "RendererTest.h"
#include "FieldRendererTest.h"

//---- HiddenFieldRendererTest -----------------------------------------------------------

class HiddenFieldRendererTest : public FieldRendererTest
{
public:
	HiddenFieldRendererTest(TString tstrName);
	virtual ~HiddenFieldRendererTest();

	static Test *suite ();
	void setUp ();

protected:

	void TestCaseEmptyConf();
	void TestCase0();
	void TestCase1();
	void TestCase2();
	void TestCase3();
	void TestCaseWithoutName();
	void TestCaseWithoutLabel();
	void TestCaseWithoutValue();
	void TestCaseWithoutSource();
	void TestCaseWithoutMultiple();
	void TestCaseWithoutChecked();
	void TestCaseWithoutOptions();
	void TestCaseWrong();
	void TestOptionRenderer();
	void TestFaultOptionRenderer();
	void TestOptionRendererOld();
	void TestFaultOptionRendererOld();

};

#endif
