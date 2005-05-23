/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _ImageButtonRendererTest_h_
#define _ImageButtonRendererTest_h_

//-*-Mode: C++;-*-

#include "RendererTest.h"
#include "FieldRendererTest.h"

//---- ImageButtonRendererTest -----------------------------------------------------------

class ImageButtonRendererTest : public FieldRendererTest
{
public:
	ImageButtonRendererTest(TString tstrName);
	virtual ~ImageButtonRendererTest();

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
	void ConfigurationWithRenderer();
	void TestOptionRenderer();
	void TestFaultOptionRenderer();
	void TestOptionRendererOld();
	void TestFaultOptionRendererOld();
};

#endif
