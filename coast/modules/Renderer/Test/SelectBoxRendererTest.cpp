/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

//--- standard modules used ----------------------------------------------------
#include "Anything.h"
#include "StringStream.h"
#include "Context.h"
#include "Renderer.h"

//--- test modules used --------------------------------------------------------
#include "TestSuite.h"

//--- module under test --------------------------------------------------------
#include "FormRenderer.h"

//--- interface include --------------------------------------------------------
#include "SelectBoxRendererTest.h"

SelectBoxRendererTest::SelectBoxRendererTest (TString tname) : FieldRendererTest(tname)
{
	fFieldRenderer = new SelectBoxRenderer("SelectBoxRenderer");
};

SelectBoxRendererTest::~SelectBoxRendererTest()
{
	if (fFieldRenderer) {
		delete fFieldRenderer;
	}
	fFieldRenderer = 0;
};

/*===============================================================*/
/*     Init                                                      */
/*===============================================================*/
void SelectBoxRendererTest::setUp ()
{
	FieldRendererTest::setUp();
}

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
	ADD_CASE(testSuite, SelectBoxRendererTest, TestCaseWithList);
	ADD_CASE(testSuite, SelectBoxRendererTest, TestCaseWithLookupList);
	ADD_CASE(testSuite, SelectBoxRendererTest, TestCaseWithListAndLookupList);
	ADD_CASE(testSuite, SelectBoxRendererTest, TestCaseWithListAndLookupListSkipped);
	ADD_CASE(testSuite, SelectBoxRendererTest, TestCaseWithUndefList);
	ADD_CASE(testSuite, SelectBoxRendererTest, TestCaseWithListAndUndefLookupList);
	ADD_CASE(testSuite, SelectBoxRendererTest, TestCaseAllAttributes);
	ADD_CASE(testSuite, SelectBoxRendererTest, TestCaseNameAndMultiple);
	ADD_CASE(testSuite, SelectBoxRendererTest, TestCaseListWithUndefOptions);
	ADD_CASE(testSuite, SelectBoxRendererTest, TestOptionRenderer);
	ADD_CASE(testSuite, SelectBoxRendererTest, TestOptionRendererOld);
	ADD_CASE(testSuite, SelectBoxRendererTest, TestFaultOptionRenderer);
	ADD_CASE(testSuite, SelectBoxRendererTest, TestFaultOptionRendererOld);
	ADD_CASE(testSuite, SelectBoxRendererTest, TestOptionSelectUnselectRenderer);
	return testSuite;

}

/*===============================================================*/
/*     Check where all is correctly defined                      */
/*===============================================================*/
void SelectBoxRendererTest::TestCase0()
{
	fCurrentTestMethod = "SelectBox-TestCase0";
	this->TestField0();
	assertCharPtrEqual( _QUOTE_(<SELECT NAME="fld_the name of field" SIZE="50" MULTIPLE the options of field></SELECT>), fReply.str() );
}

void SelectBoxRendererTest::TestCase1()
{
	fCurrentTestMethod = "SelectBox-TestCase1";
	this->TestField1();
	assertCharPtrEqual( _QUOTE_(<SELECT NAME="fld_the name of field" SIZE="50" MULTIPLE 0="the option nr. 0 of field" 1="the option nr. 1 of field" 2="the option nr. 2 of field"></SELECT>), fReply.str() );
}

void SelectBoxRendererTest::TestCase2()
{
	fCurrentTestMethod = "SelectBox-TestCase2";
	this->TestField2();
	assertCharPtrEqual( _QUOTE_(<SELECT NAME="fld_the name of field" SIZE="1" MULTIPLE 0="the option nr. 0 of field" the option nr. 1 of field2="the option nr. 2 of field"></SELECT>), fReply.str() );
}

void SelectBoxRendererTest::TestCase3()
{
	fCurrentTestMethod = "SelectBox-TestCase3";
	this->TestField3();
	assertCharPtrEqual( _QUOTE_(<SELECT NAME="fld_the name of field" SIZE="1" MULTIPLE 0="the option nr. 0 of field" the option nr. 1 of field2="the option nr. 2 of field"></SELECT>), fReply.str() );
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
	assertCharPtrEqual( _QUOTE_(<SELECT NAME="fld_the name of field" SIZE="50" MULTIPLE the options of field></SELECT>), fReply.str() );
}

void SelectBoxRendererTest::TestCaseWithoutValue()
{
	fCurrentTestMethod = "SelectBox-TestFieldWithoutValue";
	this->TestFieldWithoutValue();
	assertCharPtrEqual( _QUOTE_(<SELECT NAME="fld_the name of field" SIZE="50" MULTIPLE the options of field></SELECT>), fReply.str() );
}

void SelectBoxRendererTest::TestCaseWithoutSource()
{
	fCurrentTestMethod = "SelectBox-TestFieldWithoutSource";
	this->TestFieldWithoutSource();
	assertCharPtrEqual( _QUOTE_(<SELECT NAME="fld_the name of field" SIZE="50" MULTIPLE the options of field></SELECT>), fReply.str() );
}

void SelectBoxRendererTest::TestCaseWithoutMultiple()
{
	fCurrentTestMethod = "SelectBox-TestFieldWithoutMultiple";
	this->TestFieldWithoutMultiple();
	assertCharPtrEqual( _QUOTE_(<SELECT NAME="fld_the name of field" SIZE="50"the options of field></SELECT>), fReply.str() );
}

void SelectBoxRendererTest::TestCaseWithoutChecked()
{
	fCurrentTestMethod = "SelectBox-TestFieldWithoutChecked";
	this->TestFieldWithoutChecked();
	assertCharPtrEqual( _QUOTE_(<SELECT NAME="fld_the name of field" SIZE="50" MULTIPLE the options of field></SELECT>), fReply.str() );
}

void SelectBoxRendererTest::TestCaseWithoutOptions()
{
	fCurrentTestMethod = "SelectBox-TestFieldWithoutOptions";
	this->TestFieldWithoutOptions();
	assertCharPtrEqual( _QUOTE_(<SELECT NAME="fld_the name of field" SIZE="50" MULTIPLE></SELECT>), fReply.str() );
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
/*     Check List and LookupList                                 */
/*===============================================================*/
void SelectBoxRendererTest::TestCaseWithList()
{
	String str("<SELECT NAME=\"fld_the name of field\" SIZE=\"1\" MULTIPLE 0=\"the option nr. 0 of field\" 1=\"the option nr. 1 of field\" 2=\"the option nr. 2 of field\">\n<OPTION VALUE=\"the value from list[0]\" SELECTED>the text from list[0]\n<OPTION VALUE=\"the value from list[1]\" SELECTED>\n<OPTION VALUE=\"the value from list[2]\">the text from list[2]\n<OPTION SELECTED>the text from list[3]\n<OPTION VALUE=\"the value from list[4]\">\n<OPTION SELECTED>\n<OPTION>the text from list[6]\n<OPTION>\n</SELECT>\n");

	fCurrentTestMethod = "SelectBox-TestCaseWithList";
	this->TestFieldWithList();
	assertCharPtrEqual( (const char *)str, fReply.str() );
}

void SelectBoxRendererTest::TestCaseWithLookupList()
{
	String str("<SELECT NAME=\"fld_the name of field\" SIZE=\"1\" MULTIPLE 0=\"the option nr. 0 of field\" 1=\"the option nr. 1 of field\" 2=\"the option nr. 2 of field\">\n<OPTION VALUE=\"the value from list[0]\" SELECTED>the text from list[0]\n<OPTION VALUE=\"the value from list[1]\" SELECTED>\n<OPTION VALUE=\"the value from list[2]\">the text from list[2]\n<OPTION SELECTED>the text from list[3]\n<OPTION VALUE=\"the value from list[4]\">\n<OPTION SELECTED>\n<OPTION>the text from list[6]\n<OPTION>\n</SELECT>\n");

	fCurrentTestMethod = "SelectBox-TestCaseWithLookupList";
	this->TestFieldWithLookupList();
	assertCharPtrEqual( (const char *)str, fReply.str() );
}

void SelectBoxRendererTest::TestCaseWithListAndLookupList()
{
	String str("<SELECT NAME=\"fld_the name of field\" SIZE=\"1\" MULTIPLE 0=\"the option nr. 0 of field\" 1=\"the option nr. 1 of field\" 2=\"the option nr. 2 of field\">\n<OPTION VALUE=\"the value from LookupList[0]\" SELECTED>the text from LookupList[0]\n</SELECT>\n");

	fCurrentTestMethod = "SelectBox-TestCaseWithListAndLookupList";
	this->TestFieldWithListAndLookupList();
	assertCharPtrEqual( (const char *)str, fReply.str() );
}

void SelectBoxRendererTest::TestCaseWithListAndLookupListSkipped()
{
	String str("<SELECT NAME=\"fld_the name of field\" SIZE=\"1\" MULTIPLE 0=\"the option nr. 0 of field\" 1=\"the option nr. 1 of field\" 2=\"the option nr. 2 of field\">\n</SELECT>\n");

	fCurrentTestMethod = "SelectBox-TestCaseWithListAndLookupListSkipped";
	this->TestFieldWithListAndLookupListSkipped();
	assertCharPtrEqual( (const char *)str, fReply.str() );
}

void SelectBoxRendererTest::TestCaseWithUndefList()
{
	String str("<SELECT NAME=\"fld_the name of field\" SIZE=\"1\" MULTIPLE 0=\"the option nr. 0 of field\" 1=\"the option nr. 1 of field\" 2=\"the option nr. 2 of field\">\n<OPTION>\n</SELECT>\n");

	fCurrentTestMethod = "SelectBox-TestCaseWithUndefList";
	this->TestFieldWithUndefList();
	assertCharPtrEqual( (const char *)str, fReply.str() );
}

void SelectBoxRendererTest::TestCaseWithListAndUndefLookupList()
{
	String str("<SELECT NAME=\"fld_the name of field\" SIZE=\"1\" MULTIPLE 0=\"the option nr. 0 of field\" 1=\"the option nr. 1 of field\" 2=\"the option nr. 2 of field\">\n<OPTION>\n</SELECT>\n");

	fCurrentTestMethod = "SelectBox-TestCaseWithListAndUndefLookupList";
	this->TestFieldWithListAndUndefLookupList();
	assertCharPtrEqual( (const char *)str, fReply.str() );
}

void SelectBoxRendererTest::TestCaseAllAttributes()
{
	String str("<SELECT NAME=\"fld_EinName\" SIZE=\"10\" MULTIPLE>\n<OPTION VALUE=\"11\" SELECTED>Das ist der Text1.\n<OPTION VALUE=\"22\" SELECTED>Das ist der Text2.\n<OPTION VALUE=\"33\" SELECTED>Das ist der Text3.\n</SELECT>\n");

	fCurrentTestMethod = "SelectBox-TestCaseAllAttributes";
	this->TestAllAttributes();
	assertCharPtrEqual( (const char *)str, fReply.str() );
}

void SelectBoxRendererTest::TestCaseNameAndMultiple()
{
	String str("<SELECT NAME=\"fld_EinName\" SIZE=\"1\" MULTIPLE>\n</SELECT>\n");

	fCurrentTestMethod = "SelectBox-TestCaseNameAndMultiple";
	this->TestNameAndMultiple();
	assertCharPtrEqual( (const char *)str, fReply.str() );
}

void SelectBoxRendererTest::TestCaseListWithUndefOptions()
{
	String str("<SELECT NAME=\"fld_EinName\" SIZE=\"1\">\n<OPTION>\n</SELECT>\n");

	fCurrentTestMethod = "SelectBox-TestCaseListWithUndefOptions";
	this->TestListWithUndefOptions();
	assertCharPtrEqual( (const char *)str, fReply.str() );
}

/*===============================================================*/
/*     Check the indirection                                     */
/*===============================================================*/
void SelectBoxRendererTest::TestOptionRendererOld()
{
	fCurrentTestMethod = "SelectBoxRendererTest-TestOptionRendererOld";
	this->TestFieldOptionRendererOld();
	assertCharPtrEqual( _QUOTE_(<SELECT NAME="fld_the name of field" SIZE="50" MULTIPLE 0="the option nr. 0 of field" 1="07.08.97" 2="the option nr. 2 of field"></SELECT>), fReply.str() );
}

void SelectBoxRendererTest::TestFaultOptionRendererOld()
{
	fCurrentTestMethod = "SelectBoxRendererTest-TestFaultOptionRendererOld";
	this->TestFieldFaultOptionRendererOld();
	assertCharPtrEqual( _QUOTE_(<SELECT NAME="fld_the name of field" SIZE="50" MULTIPLE 0="the option nr. 0 of field" 1="" 2="the option nr. 2 of field"></SELECT>), fReply.str() );
}

void SelectBoxRendererTest::TestOptionRenderer()
{
	fCurrentTestMethod = "SelectBoxRendererTest-TestOptionRenderer";
	this->TestFieldOptionRenderer();
	assertCharPtrEqual( _QUOTE_(<SELECT NAME="fld_the name of field" SIZE="50" MULTIPLE 0="the option nr. 0 of field" 1="07.08.97" 2="the option nr. 2 of field"></SELECT>), fReply.str() );
}

void SelectBoxRendererTest::TestFaultOptionRenderer()
{
	fCurrentTestMethod = "SelectBoxRendererTest-TestFaultOptionRenderer";
	this->TestFieldFaultOptionRenderer();
	assertCharPtrEqual( _QUOTE_(<SELECT NAME="fld_the name of field" SIZE="50" MULTIPLE 0="the option nr. 0 of field" 1="%d.%m.%y870912000" 2="the option nr. 2 of field"></SELECT>), fReply.str() );
}

void SelectBoxRendererTest::TestOptionSelectUnselectRenderer()
{
	fCurrentTestMethod = "SelectBoxRendererTest-TestOptionSelectUnselectRenderer";
	this->TestFieldOptionSelectUnselectRenderer();
	assertCharPtrEqual( _QUOTE_(<SELECT NAME="fld_the name of field" SIZE="1">\n<OPTION VALUE="Value 1" SELECTED>Text 1\n<OPTION VALUE="Value 2">Text 2\n</SELECT>\n), fReply.str() );
}

