/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _TextAreaRendererTest_h_
#define _TextAreaRendererTest_h_

//-*-Mode: C++;-*-

#include "FieldRendererTest.h"

//---- TextAreaRendererTest -----------------------------------------------------------

class TextAreaRendererTest : public FieldRendererTest
{
public:
	TextAreaRendererTest(TString tstrName);
	virtual ~TextAreaRendererTest();

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
	void AllAttributesTextArea();
	void WidthTextArea();
	void HeightTextArea();
	void WrapTextArea();
	void Options4TextArea();
	void Options3TextArea();
	void Options2TextArea();
	void Option1TextArea();
	void OptionsNoSlotName2TextArea();
	void OptionNoSlotName1TextArea();
	void OptionsMixedTextArea();
	void NoOptionsTextArea();
	void TextTextArea();

};

#endif
