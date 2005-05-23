/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _TextFieldRendererTest_h_
#define _TextFieldRendererTest_h_

//-*-Mode: C++;-*-

#include "RendererTest.h"
#include "FieldRendererTest.h"

//---- TextFieldRendererTest -----------------------------------------------------------

class TextFieldRendererTest : public FieldRendererTest
{
public:
	TextFieldRendererTest(TString tstrName);
	virtual ~TextFieldRendererTest();

	static Test *suite ();
	void setUp ();
protected:
	// Generic test cases
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

	// Specific test cases for TextAreaRenderer
	void TestValue();
	void TestUnreadable();
	void TestSize();
	void TestMaxlength();
	void TestWidth();
	void TestHeigth();
	void TestWrap();
	void Options4TextArea();
	void Options3TextArea();
	void Options2TextArea();
	void Option1TextArea();
	void OptionsNoSlotName2TextArea();
	void OptionNoSlotName1TextArea();
	void OptionsMixedTextArea();
	void NoOptionsTextArea();

};

#endif
