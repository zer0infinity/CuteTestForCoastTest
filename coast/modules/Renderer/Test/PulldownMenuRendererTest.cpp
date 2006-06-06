/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

//--- standard modules used ----------------------------------------------------
#include "Anything.h"
#include "Context.h"
#include "StringStream.h"
#include "Renderer.h"

//--- test modules used --------------------------------------------------------
#include "TestSuite.h"

//--- module under test --------------------------------------------------------
#include "FormRenderer.h"

//--- interface include --------------------------------------------------------
#include "PulldownMenuRendererTest.h"

PulldownMenuRendererTest::PulldownMenuRendererTest (TString tname) : FieldRendererTest(tname)
{
	fFieldRenderer = new PulldownMenuRenderer("PulldownMenuRenderer");
};

PulldownMenuRendererTest::~PulldownMenuRendererTest()
{
	if (fFieldRenderer) {
		delete fFieldRenderer;
	}
	fFieldRenderer = 0;
};

/*===============================================================*/
/*     Init                                                      */
/*===============================================================*/
void PulldownMenuRendererTest::setUp ()
{
	FieldRendererTest::setUp();
}

Test *PulldownMenuRendererTest::suite ()
{
	TestSuite *testSuite = new TestSuite;

	ADD_CASE(testSuite, PulldownMenuRendererTest, TestCaseEmptyConf);
	ADD_CASE(testSuite, PulldownMenuRendererTest, TestCase0);
	ADD_CASE(testSuite, PulldownMenuRendererTest, TestCase1);
	ADD_CASE(testSuite, PulldownMenuRendererTest, TestCase2);
	ADD_CASE(testSuite, PulldownMenuRendererTest, TestCase3);
	ADD_CASE(testSuite, PulldownMenuRendererTest, TestCaseWithoutName);
	ADD_CASE(testSuite, PulldownMenuRendererTest, TestCaseWithoutLabel);
	ADD_CASE(testSuite, PulldownMenuRendererTest, TestCaseWithoutValue);
	ADD_CASE(testSuite, PulldownMenuRendererTest, TestCaseWithoutSource);
	ADD_CASE(testSuite, PulldownMenuRendererTest, TestCaseWithoutMultiple);
	ADD_CASE(testSuite, PulldownMenuRendererTest, TestCaseWithoutChecked);
	ADD_CASE(testSuite, PulldownMenuRendererTest, TestCaseWithoutOptions);
	ADD_CASE(testSuite, PulldownMenuRendererTest, TestCaseWrong);
	ADD_CASE(testSuite, PulldownMenuRendererTest, TestCaseWithList);
	ADD_CASE(testSuite, PulldownMenuRendererTest, TestCaseWithLookupList);
	ADD_CASE(testSuite, PulldownMenuRendererTest, TestCaseWithListAndLookupList);
	ADD_CASE(testSuite, PulldownMenuRendererTest, TestCaseWithListAndLookupListSkipped);
	ADD_CASE(testSuite, PulldownMenuRendererTest, TestCaseWithUndefList);
	ADD_CASE(testSuite, PulldownMenuRendererTest, TestCaseWithListAndUndefLookupList);
	ADD_CASE(testSuite, PulldownMenuRendererTest, TestCaseAllAttributes);
	ADD_CASE(testSuite, PulldownMenuRendererTest, TestCaseNameAndMultiple);
	ADD_CASE(testSuite, PulldownMenuRendererTest, TestCaseListWithUndefOptions);
	ADD_CASE(testSuite, PulldownMenuRendererTest, TestOptionRenderer);
	ADD_CASE(testSuite, PulldownMenuRendererTest, TestFaultOptionRenderer);
	ADD_CASE(testSuite, PulldownMenuRendererTest, TestOptionRendererOld);
	ADD_CASE(testSuite, PulldownMenuRendererTest, TestFaultOptionRendererOld);

	return testSuite;

}

/*===============================================================*/
/*     Check where all is correctly defined                      */
/*===============================================================*/
void PulldownMenuRendererTest::TestCase0()
{
	fCurrentTestMethod = "PulldownMenu-TestCase0";
	this->TestField0();
	assertCharPtrEqual( _QUOTE_(<SELECT NAME="fld_the name of field" SIZE='1' the options of field></SELECT>), fReply.str() );
}

void PulldownMenuRendererTest::TestCase1()
{
	fCurrentTestMethod = "PulldownMenu-TestCase1";
	this->TestField1();
	assertCharPtrEqual( _QUOTE_(<SELECT NAME="fld_the name of field" SIZE='1' 0="the option nr. 0 of field" 1="the option nr. 1 of field" 2="the option nr. 2 of field"></SELECT>), fReply.str() );
}

void PulldownMenuRendererTest::TestCase2()
{
	fCurrentTestMethod = "PulldownMenu-TestCase2";
	this->TestField2();
	assertCharPtrEqual( _QUOTE_(<SELECT NAME="fld_the name of field" SIZE='1' 0="the option nr. 0 of field" the option nr. 1 of field2="the option nr. 2 of field"></SELECT>), fReply.str() );
}

void PulldownMenuRendererTest::TestCase3()
{
	fCurrentTestMethod = "PulldownMenu-TestCase3";
	this->TestField3();
	assertCharPtrEqual( _QUOTE_(<SELECT NAME="fld_the name of field" SIZE='1' 0="the option nr. 0 of field" the option nr. 1 of field2="the option nr. 2 of field"></SELECT>), fReply.str() );
}

/*===============================================================*/
/*     Check where not all is correctly defined                  */
/*===============================================================*/
void PulldownMenuRendererTest::TestCaseEmptyConf()
{
	fCurrentTestMethod = "PulldownMenu-TestCaseEmptyConf";
	this->TestEmptyConf();
	assertCharPtrEqual( "", fReply.str() );
}

void PulldownMenuRendererTest::TestCaseWithoutName()
{
	fCurrentTestMethod = "PulldownMenu-TestCaseWithoutName";
	this->TestFieldWithoutName();
	assertCharPtrEqual( "", fReply.str() );
}

void PulldownMenuRendererTest::TestCaseWithoutLabel()
{
	fCurrentTestMethod = "PulldownMenu-TestCaseWithoutLabel";
	this->TestFieldWithoutLabel();
	assertCharPtrEqual( _QUOTE_(<SELECT NAME="fld_the name of field" SIZE='1' the options of field></SELECT>), fReply.str() );
}

void PulldownMenuRendererTest::TestCaseWithoutValue()
{
	fCurrentTestMethod = "PulldownMenu-TestFieldWithoutValue";
	this->TestFieldWithoutValue();
	assertCharPtrEqual( _QUOTE_(<SELECT NAME="fld_the name of field" SIZE='1' the options of field></SELECT>), fReply.str() );
}

void PulldownMenuRendererTest::TestCaseWithoutSource()
{
	fCurrentTestMethod = "PulldownMenu-TestFieldWithoutSource";
	this->TestFieldWithoutSource();
	assertCharPtrEqual( _QUOTE_(<SELECT NAME="fld_the name of field" SIZE='1' the options of field></SELECT>), fReply.str() );
}

void PulldownMenuRendererTest::TestCaseWithoutMultiple()
{
	fCurrentTestMethod = "PulldownMenu-TestFieldWithoutMultiple";
	this->TestFieldWithoutMultiple();
	assertCharPtrEqual( _QUOTE_(<SELECT NAME="fld_the name of field" SIZE='1' the options of field></SELECT>), fReply.str() );
}

void PulldownMenuRendererTest::TestCaseWithoutChecked()
{
	fCurrentTestMethod = "PulldownMenu-TestFieldWithoutChecked";
	this->TestFieldWithoutChecked();
	assertCharPtrEqual( _QUOTE_(<SELECT NAME="fld_the name of field" SIZE='1' the options of field></SELECT>), fReply.str() );
}

void PulldownMenuRendererTest::TestCaseWithoutOptions()
{
	fCurrentTestMethod = "PulldownMenu-TestFieldWithoutOptions";
	this->TestFieldWithoutOptions();
	assertCharPtrEqual( _QUOTE_(<SELECT NAME="fld_the name of field" SIZE='1'></SELECT>), fReply.str() );
}

/*===============================================================*/
/*     Check where there is a fault fConfig                      */
/*===============================================================*/
void PulldownMenuRendererTest::TestCaseWrong()
{
	fCurrentTestMethod = "PulldownMenu-TestCaseWrong";
	this->TestFieldWithoutName();
	assertCharPtrEqual( "", fReply.str() );
}

/*===============================================================*/
/*     Check List and LookupList                                 */
/*===============================================================*/
void PulldownMenuRendererTest::TestCaseWithList()
{
	String str("<SELECT NAME=\"fld_the name of field\" SIZE='1' 0=\"the option nr. 0 of field\" 1=\"the option nr. 1 of field\" 2=\"the option nr. 2 of field\">\n<OPTION VALUE=\"the value from list[0]\" SELECTED>the text from list[0]\n<OPTION VALUE=\"the value from list[1]\" SELECTED>\n<OPTION VALUE=\"the value from list[2]\">the text from list[2]\n<OPTION SELECTED>the text from list[3]\n<OPTION VALUE=\"the value from list[4]\">\n<OPTION SELECTED>\n<OPTION>the text from list[6]\n<OPTION>\n</SELECT>\n");

	fCurrentTestMethod = "PulldownMenu-TestCaseWithList";
	this->TestFieldWithList();
	assertCharPtrEqual( (const char *)str, fReply.str() );
}

void PulldownMenuRendererTest::TestCaseWithLookupList()
{
	String str("<SELECT NAME=\"fld_the name of field\" SIZE='1' 0=\"the option nr. 0 of field\" 1=\"the option nr. 1 of field\" 2=\"the option nr. 2 of field\">\n<OPTION VALUE=\"the value from list[0]\" SELECTED>the text from list[0]\n<OPTION VALUE=\"the value from list[1]\" SELECTED>\n<OPTION VALUE=\"the value from list[2]\">the text from list[2]\n<OPTION SELECTED>the text from list[3]\n<OPTION VALUE=\"the value from list[4]\">\n<OPTION SELECTED>\n<OPTION>the text from list[6]\n<OPTION>\n</SELECT>\n");

	fCurrentTestMethod = "PulldownMenu-TestCaseWithLookupList";
	this->TestFieldWithLookupList();
	assertCharPtrEqual( (const char *)str, fReply.str() );
}

void PulldownMenuRendererTest::TestCaseWithListAndLookupList()
{
	String str("<SELECT NAME=\"fld_the name of field\" SIZE='1' 0=\"the option nr. 0 of field\" 1=\"the option nr. 1 of field\" 2=\"the option nr. 2 of field\">\n<OPTION VALUE=\"the value from LookupList[0]\" SELECTED>the text from LookupList[0]\n</SELECT>\n");

	fCurrentTestMethod = "PulldownMenu-TestCaseWithListAndLookupList";
	this->TestFieldWithListAndLookupList();
	assertCharPtrEqual( (const char *)str, fReply.str() );
}

void PulldownMenuRendererTest::TestCaseWithListAndLookupListSkipped()
{
	String str("<SELECT NAME=\"fld_the name of field\" SIZE='1' 0=\"the option nr. 0 of field\" 1=\"the option nr. 1 of field\" 2=\"the option nr. 2 of field\">\n</SELECT>\n");

	fCurrentTestMethod = "PulldownMenu-TestCaseWithListAndLookupListSkipped";
	this->TestFieldWithListAndLookupListSkipped();
	assertCharPtrEqual( (const char *)str, fReply.str() );
}

void PulldownMenuRendererTest::TestCaseWithUndefList()
{
	String str("<SELECT NAME=\"fld_the name of field\" SIZE='1' 0=\"the option nr. 0 of field\" 1=\"the option nr. 1 of field\" 2=\"the option nr. 2 of field\">\n<OPTION>\n</SELECT>\n");

	fCurrentTestMethod = "PulldownMenu-TestCaseWithUndefList";
	this->TestFieldWithUndefList();
	assertCharPtrEqual( (const char *)str, fReply.str() );
}

void PulldownMenuRendererTest::TestCaseWithListAndUndefLookupList()
{
	String str("<SELECT NAME=\"fld_the name of field\" SIZE='1' 0=\"the option nr. 0 of field\" 1=\"the option nr. 1 of field\" 2=\"the option nr. 2 of field\">\n<OPTION>\n</SELECT>\n");

	fCurrentTestMethod = "PulldownMenu-TestCaseWithListAndUndefLookupList";
	this->TestFieldWithListAndUndefLookupList();
	assertCharPtrEqual( (const char *)str, fReply.str() );
}

void PulldownMenuRendererTest::TestCaseAllAttributes()
{
	String str("<SELECT NAME=\"fld_EinName\" SIZE='1'>\n<OPTION VALUE=\"11\" SELECTED>Das ist der Text1.\n<OPTION VALUE=\"22\" SELECTED>Das ist der Text2.\n<OPTION VALUE=\"33\" SELECTED>Das ist der Text3.\n</SELECT>\n");

	fCurrentTestMethod = "PulldownMenu-TestCaseAllAttributes";
	this->TestAllAttributes();
	assertCharPtrEqual( (const char *)str, fReply.str() );
}

void PulldownMenuRendererTest::TestCaseNameAndMultiple()
{
	String str("<SELECT NAME=\"fld_EinName\" SIZE='1'>\n</SELECT>\n");

	fCurrentTestMethod = "PulldownMenu-TestCaseNameAndMultiple";
	this->TestNameAndMultiple();
	assertCharPtrEqual( (const char *)str, fReply.str() );
}

void PulldownMenuRendererTest::TestCaseListWithUndefOptions()
{
	String str("<SELECT NAME=\"fld_EinName\" SIZE='1'>\n<OPTION>\n</SELECT>\n");

	fCurrentTestMethod = "PulldownMenu-TestCaseListWithUndefOptions";
	this->TestListWithUndefOptions();
	assertCharPtrEqual( (const char *)str, fReply.str() );
}

/*===============================================================*/
/*     Check the indirection                                     */
/*===============================================================*/
void PulldownMenuRendererTest::TestOptionRendererOld()
{
	fCurrentTestMethod = "PulldownMenuRendererTest-TestOptionRendererOld";
	this->TestFieldOptionRendererOld();
	assertCharPtrEqual( _QUOTE_(<SELECT NAME="fld_the name of field" SIZE='1' 0="the option nr. 0 of field" 1="07.08.97" 2="the option nr. 2 of field"></SELECT>), fReply.str() );
}

void PulldownMenuRendererTest::TestFaultOptionRendererOld()
{
	fCurrentTestMethod = "PulldownMenuRendererTest-TestFaultOptionRendererOld";
	this->TestFieldFaultOptionRendererOld();
	assertCharPtrEqual( _QUOTE_(<SELECT NAME="fld_the name of field" SIZE='1' 0="the option nr. 0 of field" 1="" 2="the option nr. 2 of field"></SELECT>), fReply.str() );
}

void PulldownMenuRendererTest::TestOptionRenderer()
{
	fCurrentTestMethod = "PulldownMenuRendererTest-TestOptionRenderer";
	this->TestFieldOptionRenderer();
	assertCharPtrEqual( _QUOTE_(<SELECT NAME="fld_the name of field" SIZE='1' 0="the option nr. 0 of field" 1="07.08.97" 2="the option nr. 2 of field"></SELECT>), fReply.str() );
}

void PulldownMenuRendererTest::TestFaultOptionRenderer()
{
	fCurrentTestMethod = "PulldownMenuRendererTest-TestFaultOptionRenderer";
	this->TestFieldFaultOptionRenderer();
	assertCharPtrEqual( _QUOTE_(<SELECT NAME="fld_the name of field" SIZE='1' 0="the option nr. 0 of field" 1="%d.%m.%y870912000" 2="the option nr. 2 of field"></SELECT>), fReply.str() );
}

