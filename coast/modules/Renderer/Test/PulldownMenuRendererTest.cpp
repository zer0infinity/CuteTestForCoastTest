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
	ADD_CASE(testSuite, PulldownMenuRendererTest, TestCaseWithListAndLookupListSkipped);
	ADD_CASE(testSuite, PulldownMenuRendererTest, TestCaseWithUndefList);
	ADD_CASE(testSuite, PulldownMenuRendererTest, TestCaseWithListAndUndefLookupList);
	ADD_CASE(testSuite, PulldownMenuRendererTest, TestCaseNameAndMultiple);
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
	assertCharPtrEqual( _QUOTE_(<select name="fld_the name of field" size='1' the options of field>\n</select>\n), fReply.str() );
}

void PulldownMenuRendererTest::TestCase1()
{
	fCurrentTestMethod = "PulldownMenu-TestCase1";
	this->TestField1();
	assertCharPtrEqual( _QUOTE_(<select name="fld_the name of field" size='1' 0="the option nr. 0 of field" 1="the option nr. 1 of field" 2="the option nr. 2 of field">\n</select>\n), fReply.str() );
}

void PulldownMenuRendererTest::TestCase2()
{
	fCurrentTestMethod = "PulldownMenu-TestCase2";
	this->TestField2();
	assertCharPtrEqual( _QUOTE_(<select name="fld_the name of field" size='1' 0="the option nr. 0 of field" the option nr. 1 of field 2="the option nr. 2 of field">\n</select>\n), fReply.str() );
}

void PulldownMenuRendererTest::TestCase3()
{
	fCurrentTestMethod = "PulldownMenu-TestCase3";
	this->TestField3();
	assertCharPtrEqual( _QUOTE_(<select name="fld_the name of field" size='1' 0="the option nr. 0 of field" the option nr. 1 of field 2="the option nr. 2 of field">\n</select>\n), fReply.str() );
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
	assertCharPtrEqual( _QUOTE_(<select name="fld_the name of field" size='1' the options of field>\n</select>\n), fReply.str() );
}

void PulldownMenuRendererTest::TestCaseWithoutValue()
{
	fCurrentTestMethod = "PulldownMenu-TestFieldWithoutValue";
	this->TestFieldWithoutValue();
	assertCharPtrEqual( _QUOTE_(<select name="fld_the name of field" size='1' the options of field>\n</select>\n), fReply.str() );
}

void PulldownMenuRendererTest::TestCaseWithoutSource()
{
	fCurrentTestMethod = "PulldownMenu-TestFieldWithoutSource";
	this->TestFieldWithoutSource();
	assertCharPtrEqual( _QUOTE_(<select name="fld_the name of field" size='1' the options of field>\n</select>\n), fReply.str() );
}

void PulldownMenuRendererTest::TestCaseWithoutMultiple()
{
	fCurrentTestMethod = "PulldownMenu-TestFieldWithoutMultiple";
	this->TestFieldWithoutMultiple();
	assertCharPtrEqual( _QUOTE_(<select name="fld_the name of field" size='1' the options of field>\n</select>\n), fReply.str() );
}

void PulldownMenuRendererTest::TestCaseWithoutChecked()
{
	fCurrentTestMethod = "PulldownMenu-TestFieldWithoutChecked";
	this->TestFieldWithoutChecked();
	assertCharPtrEqual( _QUOTE_(<select name="fld_the name of field" size='1' the options of field>\n</select>\n), fReply.str() );
}

void PulldownMenuRendererTest::TestCaseWithoutOptions()
{
	fCurrentTestMethod = "PulldownMenu-TestFieldWithoutOptions";
	this->TestFieldWithoutOptions();
	assertCharPtrEqual( _QUOTE_(<select name="fld_the name of field" size='1'>\n</select>\n), fReply.str() );
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
/*     Check List and ListName                                 */
/*===============================================================*/
void PulldownMenuRendererTest::TestCaseWithListAndLookupListSkipped()
{
	String str("<select name=\"fld_the name of field\" size='1' 0=\"the option nr. 0 of field\" 1=\"the option nr. 1 of field\" 2=\"the option nr. 2 of field\">\n</select>\n");

	fCurrentTestMethod = "PulldownMenu-TestCaseWithListAndLookupListSkipped";
	this->TestFieldWithListAndLookupListSkipped();
	assertCharPtrEqual( (const char *)str, fReply.str() );
}

void PulldownMenuRendererTest::TestCaseWithUndefList()
{
	String str("<select name=\"fld_the name of field\" size='1' 0=\"the option nr. 0 of field\" 1=\"the option nr. 1 of field\" 2=\"the option nr. 2 of field\">\n<option></option>\n</select>\n");

	fCurrentTestMethod = "PulldownMenu-TestCaseWithUndefList";
	this->TestFieldWithUndefList();
	assertCharPtrEqual( (const char *)str, fReply.str() );
}

void PulldownMenuRendererTest::TestCaseWithListAndUndefLookupList()
{
	String str("<select name=\"fld_the name of field\" size='1' 0=\"the option nr. 0 of field\" 1=\"the option nr. 1 of field\" 2=\"the option nr. 2 of field\">\n<option></option>\n</select>\n");

	fCurrentTestMethod = "PulldownMenu-TestCaseWithListAndUndefLookupList";
	this->TestFieldWithListAndUndefLookupList();
	assertCharPtrEqual( (const char *)str, fReply.str() );
}

void PulldownMenuRendererTest::TestCaseNameAndMultiple()
{
	String str("<select name=\"fld_EinName\" size='1'>\n</select>\n");

	fCurrentTestMethod = "PulldownMenu-TestCaseNameAndMultiple";
	this->TestNameAndMultiple();
	assertCharPtrEqual( (const char *)str, fReply.str() );
}

/*===============================================================*/
/*     Check the indirection                                     */
/*===============================================================*/
void PulldownMenuRendererTest::TestOptionRendererOld()
{
	fCurrentTestMethod = "PulldownMenuRendererTest-TestOptionRendererOld";
	this->TestFieldOptionRendererOld();
	assertCharPtrEqual( _QUOTE_(<select name="fld_the name of field" size='1' 0="the option nr. 0 of field" 1="07.08.97" 2="the option nr. 2 of field">\n</select>\n), fReply.str() );
}

void PulldownMenuRendererTest::TestFaultOptionRendererOld()
{
	fCurrentTestMethod = "PulldownMenuRendererTest-TestFaultOptionRendererOld";
	this->TestFieldFaultOptionRendererOld();
	assertCharPtrEqual( _QUOTE_(<select name="fld_the name of field" size='1' 0="the option nr. 0 of field" 1="" 2="the option nr. 2 of field">\n</select>\n), fReply.str() );
}

void PulldownMenuRendererTest::TestOptionRenderer()
{
	fCurrentTestMethod = "PulldownMenuRendererTest-TestOptionRenderer";
	this->TestFieldOptionRenderer();
	assertCharPtrEqual( _QUOTE_(<select name="fld_the name of field" size='1' 0="the option nr. 0 of field" 1="07.08.97" 2="the option nr. 2 of field">\n</select>\n), fReply.str() );
}

void PulldownMenuRendererTest::TestFaultOptionRenderer()
{
	fCurrentTestMethod = "PulldownMenuRendererTest-TestFaultOptionRenderer";
	this->TestFieldFaultOptionRenderer();
	assertCharPtrEqual( _QUOTE_(<select name="fld_the name of field" size='1' 0="the option nr. 0 of field" 1="%d.%m.%y870912000" 2="the option nr. 2 of field">\n</select>\n), fReply.str() );
}

