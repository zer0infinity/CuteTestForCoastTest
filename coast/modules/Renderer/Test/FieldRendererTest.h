/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _FieldRendererTest_h_
#define _FieldRendererTest_h_

#include "RendererTest.h"

class String;
class Renderer;

//---- FieldRendererTest -----------------------------------------------------------
class FieldRendererTest : public RendererTest
{
public:
	FieldRendererTest(TString tstrName);
	virtual ~FieldRendererTest();

	static Test *suite ();
	void setUp ();

protected:
	Renderer	*fFieldRenderer;
	String fCurrentTestMethod;

	void RenderConf();
	void PrintResult();

	// Useful for all renderers
	void ConfigureField0();
	void TestField0();
	void ConfigureField1();
	void TestField1();
	void ConfigureField2();
	void TestField2();
	void ConfigureField3();
	void TestField3();
	void ConfigureFieldOptionRendererOld();
	void TestFieldOptionRendererOld();
	void ConfigureFieldOptionRenderer();
	void TestFieldOptionRenderer();
	void ConfigureFieldFaultOptionRendererOld();
	void TestFieldFaultOptionRendererOld();
	void ConfigureFieldFaultOptionRenderer();
	void TestFieldFaultOptionRenderer();
	void TestFieldWithoutName();
	void TestFieldWithoutLabel();
	void TestFieldWithoutValue();
	void TestFieldWithoutSource();
	void TestFieldWithoutMultiple();
	void TestFieldWithoutChecked();
	void TestFieldWithoutOptions();
	void ConfigureWrongField();
	void TestWrongField();
	void ConfigureFieldWithRenderer();
	void TestConfigureFieldWithRenderer();
	void ConfigureEmptyConf();
	void TestEmptyConf();
	void ConfigureFieldOptionSelectUnselectRenderer();
	void TestFieldOptionSelectUnselectRenderer();

	// Useful only for certain renderers
	void ConfigureFieldWithList();
	void TestFieldWithList();
	void ConfigureFieldWithLookupList();
	void TestFieldWithLookupList();
	void ConfigureFieldWithListAndLookupList();
	void TestFieldWithListAndLookupList();
	void ConfigureFieldWithListAndLookupListSkipped();
	void TestFieldWithListAndLookupListSkipped();
	void ConfigureFieldWithUndefList();
	void TestFieldWithUndefList();
	void ConfigureFieldWithListAndUndefLookupList();
	void TestFieldWithListAndUndefLookupList();
	void ConfigureAllAttributes();
	void TestAllAttributes();
	void ConfigureNameAndMultiple();
	void TestNameAndMultiple();
	void ConfigureListWithUndefOptions();
	void TestListWithUndefOptions();

	void ConfigureValue();
	void TestFieldValue();
	void ConfigureUnreadable();
	void TestFieldUnreadable();
	void ConfigureSize();
	void TestFieldSize();
	void ConfigureMaxlength();
	void TestFieldMaxlength();
	void ConfigureWidth();
	void TestFieldWidth();
	void ConfigureHeight();
	void TestFieldHeight();
	void ConfigureWrap();
	void TestFieldWrap();
	void Configure4Opt();
	void TestField4Opt();
	void Configure3Opt();
	void TestField3Opt();
	void Configure2Opt();
	void TestField2Opt();
	void Configure1Opt();
	void TestField1Opt();
	void Configure2OptNoSlotName();
	void TestField2OptNoSlotName();
	void Configure1OptNoSlotName();
	void TestField1OptNoSlotName();
	void Configure3OptMixed();
	void TestField3OptMixed();
	void ConfigureOptEmpty();
	void TestFieldOptEmpty();

};

#endif

