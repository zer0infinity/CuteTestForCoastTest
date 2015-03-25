/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#include "SelectBoxRendererTest.h"
#include "FormRenderer.h"
#include "TestSuite.h"

SelectBoxRendererTest::SelectBoxRendererTest (TString tname)
	: FieldRendererTest(tname)
{
	fFieldRenderer = new (coast::storage::Global()) SelectBoxRenderer("SelectBoxRenderer");
}

SelectBoxRendererTest::~SelectBoxRendererTest()
{
	if (fFieldRenderer) {
		delete fFieldRenderer;
	}
	fFieldRenderer = 0;
}

/*===============================================================*/
/*     Init                                                      */
/*===============================================================*/
Test *SelectBoxRendererTest::suite ()
{
	TestSuite *testSuite = new TestSuite;

	ADD_CASE(testSuite, SelectBoxRendererTest, TestCaseEmptyConf);
	ADD_CASE(testSuite, SelectBoxRendererTest, TestCase0);
	ADD_CASE(testSuite, SelectBoxRendererTest, TestCase1);
	ADD_CASE(testSuite, SelectBoxRendererTest, TestCase2);
	ADD_CASE(testSuite, SelectBoxRendererTest, TestCase3);
	ADD_CASE(testSuite, SelectBoxRendererTest, TestCaseWithoutName);
	ADD_CASE(testSuite, SelectBoxRendererTest, TestCaseWithoutLabel);
	ADD_CASE(testSuite, SelectBoxRendererTest, TestCaseWithoutValue);
	ADD_CASE(testSuite, SelectBoxRendererTest, TestCaseWithoutSource);
	ADD_CASE(testSuite, SelectBoxRendererTest, TestCaseWithoutMultiple);
	ADD_CASE(testSuite, SelectBoxRendererTest, TestCaseWithoutChecked);
	ADD_CASE(testSuite, SelectBoxRendererTest, TestCaseWithoutOptions);
	ADD_CASE(testSuite, SelectBoxRendererTest, TestCaseWrong);
	ADD_CASE(testSuite, SelectBoxRendererTest, TestCaseWithUndefList);
	ADD_CASE(testSuite, SelectBoxRendererTest, TestCaseNameAndMultiple);
	ADD_CASE(testSuite, SelectBoxRendererTest, TestOptionRenderer);
	ADD_CASE(testSuite, SelectBoxRendererTest, TestOptionRendererOld);
	ADD_CASE(testSuite, SelectBoxRendererTest, TestFaultOptionRenderer);
	ADD_CASE(testSuite, SelectBoxRendererTest, TestFaultOptionRendererOld);
	return testSuite;
}

/*===============================================================*/
/*     Check where all is correctly defined                      */
/*===============================================================*/
void SelectBoxRendererTest::TestCase0()
{
	fCurrentTestMethod = "SelectBox-TestCase0";
	this->TestField0();
	assertCharPtrEqual( _QUOTE_(<select name="fld_the name of field" size="50" multiple the options of field>\n</select>\n), fReply.str() );
}

void SelectBoxRendererTest::TestCase1()
{
	fCurrentTestMethod = "SelectBox-TestCase1";
	this->TestField1();
	assertCharPtrEqual( _QUOTE_(<select name="fld_the name of field" size="50" multiple 0="the option nr. 0 of field" 1="the option nr. 1 of field" 2="the option nr. 2 of field">\n</select>\n), fReply.str() );
}

void SelectBoxRendererTest::TestCase2()
{
	fCurrentTestMethod = "SelectBox-TestCase2";
	this->TestField2();
	assertCharPtrEqual( _QUOTE_(<select name="fld_the name of field" size="1" multiple 0="the option nr. 0 of field" the option nr. 1 of field 2="the option nr. 2 of field">\n</select>\n), fReply.str() );
}

void SelectBoxRendererTest::TestCase3()
{
	fCurrentTestMethod = "SelectBox-TestCase3";
	this->TestField3();
	assertCharPtrEqual( _QUOTE_(<select name="fld_the name of field" size="1" multiple 0="the option nr. 0 of field" the option nr. 1 of field 2="the option nr. 2 of field">\n</select>\n), fReply.str() );
}

/*===============================================================*/
/*     Check where not all is correctly defined                  */
/*===============================================================*/
void SelectBoxRendererTest::TestCaseEmptyConf()
{
	fCurrentTestMethod = "SelectBox-TestCaseEmptyConf";
	this->TestEmptyConf();
	assertCharPtrEqual( "", fReply.str() );
}

void SelectBoxRendererTest::TestCaseWithoutName()
{
	fCurrentTestMethod = "SelectBox-TestCaseWithoutName";
	this->TestFieldWithoutName();
	assertCharPtrEqual( "", fReply.str() );
}

void SelectBoxRendererTest::TestCaseWithoutLabel()
{
	fCurrentTestMethod = "SelectBox-TestCaseWithoutLabel";
	this->TestFieldWithoutLabel();
	assertCharPtrEqual( _QUOTE_(<select name="fld_the name of field" size="50" multiple the options of field>\n</select>\n), fReply.str() );
}

void SelectBoxRendererTest::TestCaseWithoutValue()
{
	fCurrentTestMethod = "SelectBox-TestFieldWithoutValue";
	this->TestFieldWithoutValue();
	assertCharPtrEqual( _QUOTE_(<select name="fld_the name of field" size="50" multiple the options of field>\n</select>\n), fReply.str() );
}

void SelectBoxRendererTest::TestCaseWithoutSource()
{
	fCurrentTestMethod = "SelectBox-TestFieldWithoutSource";
	this->TestFieldWithoutSource();
	assertCharPtrEqual( _QUOTE_(<select name="fld_the name of field" size="50" multiple the options of field>\n</select>\n), fReply.str() );
}

void SelectBoxRendererTest::TestCaseWithoutMultiple()
{
	fCurrentTestMethod = "SelectBox-TestFieldWithoutMultiple";
	this->TestFieldWithoutMultiple();
	assertCharPtrEqual( _QUOTE_(<select name="fld_the name of field" size="50" the options of field>\n</select>\n), fReply.str() );
}

void SelectBoxRendererTest::TestCaseWithoutChecked()
{
	fCurrentTestMethod = "SelectBox-TestFieldWithoutChecked";
	this->TestFieldWithoutChecked();
	assertCharPtrEqual( _QUOTE_(<select name="fld_the name of field" size="50" multiple the options of field>\n</select>\n), fReply.str() );
}

void SelectBoxRendererTest::TestCaseWithoutOptions()
{
	fCurrentTestMethod = "SelectBox-TestFieldWithoutOptions";
	this->TestFieldWithoutOptions();
	assertCharPtrEqual( _QUOTE_(<select name="fld_the name of field" size="50" multiple>\n</select>\n), fReply.str() );
}

/*===============================================================*/
/*     Check where there is a fault fConfig                      */
/*===============================================================*/
void SelectBoxRendererTest::TestCaseWrong()
{
	fCurrentTestMethod = "SelectBox-TestCaseWrong";
	this->TestFieldWithoutName();
	assertCharPtrEqual( "", fReply.str() );
}

/*===============================================================*/
/*     Check List and ListName                                 */
/*===============================================================*/
void SelectBoxRendererTest::TestCaseWithUndefList()
{
	String str("<select name=\"fld_the name of field\" size=\"1\" multiple 0=\"the option nr. 0 of field\" 1=\"the option nr. 1 of field\" 2=\"the option nr. 2 of field\">\n<option></option>\n</select>\n");

	fCurrentTestMethod = "SelectBox-TestCaseWithUndefList";
	this->TestFieldWithUndefList();
	assertCharPtrEqual( (const char *)str, fReply.str() );
}

void SelectBoxRendererTest::TestCaseNameAndMultiple()
{
	String str("<select name=\"fld_EinName\" size=\"1\" multiple>\n</select>\n");

	fCurrentTestMethod = "SelectBox-TestCaseNameAndMultiple";
	this->TestNameAndMultiple();
	assertCharPtrEqual( (const char *)str, fReply.str() );
}

/*===============================================================*/
/*     Check the indirection                                     */
/*===============================================================*/
void SelectBoxRendererTest::TestOptionRendererOld()
{
	fCurrentTestMethod = "SelectBoxRendererTest-TestOptionRendererOld";
	this->TestFieldOptionRendererOld();
	assertCharPtrEqual( _QUOTE_(<select name="fld_the name of field" size="50" multiple 0="the option nr. 0 of field" 1="07.08.97" 2="the option nr. 2 of field">\n</select>\n), fReply.str() );
}

void SelectBoxRendererTest::TestFaultOptionRendererOld()
{
	fCurrentTestMethod = "SelectBoxRendererTest-TestFaultOptionRendererOld";
	this->TestFieldFaultOptionRendererOld();
	assertCharPtrEqual( _QUOTE_(<select name="fld_the name of field" size="50" multiple 0="the option nr. 0 of field" 1="" 2="the option nr. 2 of field">\n</select>\n), fReply.str() );
}

void SelectBoxRendererTest::TestOptionRenderer()
{
	fCurrentTestMethod = "SelectBoxRendererTest-TestOptionRenderer";
	this->TestFieldOptionRenderer();
	assertCharPtrEqual( _QUOTE_(<select name="fld_the name of field" size="50" multiple 0="the option nr. 0 of field" 1="07.08.97" 2="the option nr. 2 of field">\n</select>\n), fReply.str() );
}

void SelectBoxRendererTest::TestFaultOptionRenderer()
{
	fCurrentTestMethod = "SelectBoxRendererTest-TestFaultOptionRenderer";
	this->TestFieldFaultOptionRenderer();
	assertCharPtrEqual( _QUOTE_(<select name="fld_the name of field" size="50" multiple 0="the option nr. 0 of field" 1="%d.%m.%y870912000" 2="the option nr. 2 of field">\n</select>\n), fReply.str() );
}
