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
#include "CheckBoxRendererTest.h"

CheckBoxRendererTest::CheckBoxRendererTest (TString tname) : FieldRendererTest(tname)
{
	fFieldRenderer = new CheckBoxRenderer("CheckBoxRendererTest");
};

CheckBoxRendererTest::~CheckBoxRendererTest()
{
	if (fFieldRenderer) {
		delete fFieldRenderer;
	}
	fFieldRenderer = 0;
};

/*===============================================================*/
/*     Init                                                      */
/*===============================================================*/
void CheckBoxRendererTest::setUp ()
// setup config for all the renderers in this TestCase
{
	FieldRendererTest::setUp();
} // setUp

Test *CheckBoxRendererTest::suite ()
// collect all test cases for the CheckBoxRenderer
{
	TestSuite *testSuite = new TestSuite;

	testSuite->addTest (NEW_CASE(CheckBoxRendererTest, TestCaseEmptyConf));
	testSuite->addTest (NEW_CASE(CheckBoxRendererTest, TestCase0));
	testSuite->addTest (NEW_CASE(CheckBoxRendererTest, TestCase1));
	testSuite->addTest (NEW_CASE(CheckBoxRendererTest, TestCase2));
	testSuite->addTest (NEW_CASE(CheckBoxRendererTest, TestCase3));
	testSuite->addTest (NEW_CASE(CheckBoxRendererTest, TestCaseWithoutName));
	testSuite->addTest (NEW_CASE(CheckBoxRendererTest, TestCaseWithoutLabel));
	testSuite->addTest (NEW_CASE(CheckBoxRendererTest, TestCaseWithoutValue));
	testSuite->addTest (NEW_CASE(CheckBoxRendererTest, TestCaseWithoutSource));
	testSuite->addTest (NEW_CASE(CheckBoxRendererTest, TestCaseWithoutMultiple));
	testSuite->addTest (NEW_CASE(CheckBoxRendererTest, TestCaseWithoutChecked ));
	testSuite->addTest (NEW_CASE(CheckBoxRendererTest, TestCaseWithoutOptions));
	testSuite->addTest (NEW_CASE(CheckBoxRendererTest, TestCaseWrong ));
	testSuite->addTest (NEW_CASE(CheckBoxRendererTest, TestOptionRenderer ));
	testSuite->addTest (NEW_CASE(CheckBoxRendererTest, TestFaultOptionRenderer ));
	testSuite->addTest (NEW_CASE(CheckBoxRendererTest, TestOptionRendererOld ));
	testSuite->addTest (NEW_CASE(CheckBoxRendererTest, TestFaultOptionRendererOld ));

	return testSuite;

} // suite

/*===============================================================*/
/*     Check where all is correctly defined                      */
/*===============================================================*/
void CheckBoxRendererTest::TestCase0()
{
	fCurrentTestMethod = "CheckBox-TestCase0";
	this->TestField0();
	assertCharPtrEqual( _QUOTE_(<INPUT TYPE="CHECKBOX" NAME="fld_the name of field" the options of field>), fReply.str() );
}

void CheckBoxRendererTest::TestCase1()
{
	fCurrentTestMethod = "CheckBox-TestCase1";
	this->TestField1();
	assertCharPtrEqual( _QUOTE_(<INPUT TYPE="CHECKBOX" NAME="fld_the name of field" 0="the option nr. 0 of field" 1="the option nr. 1 of field" 2="the option nr. 2 of field">), fReply.str() );
}

void CheckBoxRendererTest::TestCase2()
{
	fCurrentTestMethod = "CheckBox-TestCase2";
	this->TestField2();
	assertCharPtrEqual( _QUOTE_(<INPUT TYPE="CHECKBOX" NAME="fld_the name of field" 0="the option nr. 0 of field" the option nr. 1 of field 2="the option nr. 2 of field">), fReply.str() );
}

void CheckBoxRendererTest::TestCase3()
{
	fCurrentTestMethod = "CheckBox-TestCase3";
	this->TestField3();
	assertCharPtrEqual( _QUOTE_(<INPUT TYPE="CHECKBOX" NAME="fld_the name of field" 0="the option nr. 0 of field" the option nr. 1 of field 2="the option nr. 2 of field">), fReply.str() );
}

/*===============================================================*/
/*     Check where not all is correctly defined                  */
/*===============================================================*/
void CheckBoxRendererTest::TestCaseEmptyConf()
{
	fCurrentTestMethod = "CheckBox-TestCaseEmptyConf";
	this->TestEmptyConf();
	assertCharPtrEqual( "", fReply.str() );
}

void CheckBoxRendererTest::TestCaseWithoutName()
{
	fCurrentTestMethod = "CheckBox-TestCaseWithoutName";
	this->TestFieldWithoutName();
	assertCharPtrEqual( "", fReply.str() );
}

void CheckBoxRendererTest::TestCaseWithoutLabel()
{
	fCurrentTestMethod = "CheckBox-TestCaseWithoutLabel";
	this->TestFieldWithoutLabel();
	assertCharPtrEqual( _QUOTE_(<INPUT TYPE="CHECKBOX" NAME="fld_the name of field" the options of field>), fReply.str() );
}

void CheckBoxRendererTest::TestCaseWithoutValue()
{
	fCurrentTestMethod = "CheckBox-TestFieldWithoutValue";
	this->TestFieldWithoutValue();
	assertCharPtrEqual( _QUOTE_(<INPUT TYPE="CHECKBOX" NAME="fld_the name of field" the options of field>), fReply.str() );
}

void CheckBoxRendererTest::TestCaseWithoutSource()
{
	fCurrentTestMethod = "CheckBox-TestFieldWithoutSource";
	this->TestFieldWithoutValue();
	assertCharPtrEqual( _QUOTE_(<INPUT TYPE="CHECKBOX" NAME="fld_the name of field" the options of field>), fReply.str() );
}

void CheckBoxRendererTest::TestCaseWithoutMultiple()
{
	fCurrentTestMethod = "CheckBox-TestFieldWithoutMultiple";
	this->TestFieldWithoutMultiple();
	assertCharPtrEqual( _QUOTE_(<INPUT TYPE="CHECKBOX" NAME="fld_the name of field" the options of field>), fReply.str() );
}

void CheckBoxRendererTest::TestCaseWithoutChecked()
{
	fCurrentTestMethod = "CheckBox-TestFieldWithoutChecked";
	this->TestFieldWithoutChecked();
	assertCharPtrEqual( _QUOTE_(<INPUT TYPE="CHECKBOX" NAME="fld_the name of field" the options of field>), fReply.str() );
}

void CheckBoxRendererTest::TestCaseWithoutOptions()
{
	fCurrentTestMethod = "CheckBox-TestFieldWithoutOptions";
	this->TestFieldWithoutOptions();
	assertCharPtrEqual( _QUOTE_(<INPUT TYPE="CHECKBOX" NAME="fld_the name of field">), fReply.str() );
}

/*===============================================================*/
/*     Check where there is a fault fConfig                      */
/*===============================================================*/
void CheckBoxRendererTest::TestCaseWrong()
{
	fCurrentTestMethod = "CheckBox-TestCaseWrong";
	this->TestFieldWithoutName();
	assertCharPtrEqual( "", fReply.str() );
}

/*===============================================================*/
/*     Check the indirection                                     */
/*===============================================================*/
void CheckBoxRendererTest::TestOptionRendererOld()
{
	fCurrentTestMethod = "CheckBoxRendererTest-TestOptionRendererOld";
	this->TestFieldOptionRendererOld();
	assertCharPtrEqual( _QUOTE_(<INPUT TYPE="CHECKBOX" NAME="fld_the name of field" 0="the option nr. 0 of field" 1="07.08.97" 2="the option nr. 2 of field">), fReply.str() );
}

void CheckBoxRendererTest::TestFaultOptionRendererOld()
{
	fCurrentTestMethod = "CheckBoxRendererTest-TestFaultOptionRendererOld";
	this->TestFieldFaultOptionRendererOld();
	assertCharPtrEqual( _QUOTE_(<INPUT TYPE="CHECKBOX" NAME="fld_the name of field" 0="the option nr. 0 of field" 1="" 2="the option nr. 2 of field">), fReply.str() );
}

void CheckBoxRendererTest::TestOptionRenderer()
{
	fCurrentTestMethod = "CheckBoxRendererTest-TestOptionRenderer";
	this->TestFieldOptionRenderer();
	assertCharPtrEqual( _QUOTE_(<INPUT TYPE="CHECKBOX" NAME="fld_the name of field" 0="the option nr. 0 of field" 1="07.08.97" 2="the option nr. 2 of field">), fReply.str() );
}

void CheckBoxRendererTest::TestFaultOptionRenderer()
{
	fCurrentTestMethod = "CheckBoxRendererTest-TestFaultOptionRenderer";
	this->TestFieldFaultOptionRenderer();
	assertCharPtrEqual( _QUOTE_(<INPUT TYPE="CHECKBOX" NAME="fld_the name of field" 0="the option nr. 0 of field" 1="%d.%m.%y870912000" 2="the option nr. 2 of field">), fReply.str() );
}
