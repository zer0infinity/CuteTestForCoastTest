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
#include "RadioButtonRendererTest.h"

RadioButtonRendererTest::RadioButtonRendererTest (TString tname) : FieldRendererTest(tname)
{
	fFieldRenderer = new (Storage::Global()) RadioButtonRenderer("RadioButtonRenderer");
};

RadioButtonRendererTest::~RadioButtonRendererTest()
{
	if (fFieldRenderer) {
		delete fFieldRenderer;
	}
	fFieldRenderer = 0;
};

/*===============================================================*/
/*     Init                                                      */
/*===============================================================*/
void RadioButtonRendererTest::setUp ()
{
	FieldRendererTest::setUp();
}

Test *RadioButtonRendererTest::suite ()
{
	TestSuite *testSuite = new TestSuite;

	ADD_CASE(testSuite, RadioButtonRendererTest, TestCaseEmptyConf);
	ADD_CASE(testSuite, RadioButtonRendererTest, TestCase0);
	ADD_CASE(testSuite, RadioButtonRendererTest, TestCase1);
	ADD_CASE(testSuite, RadioButtonRendererTest, TestCase2);
	ADD_CASE(testSuite, RadioButtonRendererTest, TestCase3);
	ADD_CASE(testSuite, RadioButtonRendererTest, TestCaseWithoutName);
	ADD_CASE(testSuite, RadioButtonRendererTest, TestCaseWithoutLabel);
	ADD_CASE(testSuite, RadioButtonRendererTest, TestCaseWithoutValue);
	ADD_CASE(testSuite, RadioButtonRendererTest, TestCaseWithoutSource);
	ADD_CASE(testSuite, RadioButtonRendererTest, TestCaseWithoutMultiple);
	ADD_CASE(testSuite, RadioButtonRendererTest, TestCaseWithoutChecked);
	ADD_CASE(testSuite, RadioButtonRendererTest, TestCaseWithoutOptions);
	ADD_CASE(testSuite, RadioButtonRendererTest, TestCaseWrong);
	ADD_CASE(testSuite, RadioButtonRendererTest, TestOptionRenderer);
	ADD_CASE(testSuite, RadioButtonRendererTest, TestOptionRendererOld);
	ADD_CASE(testSuite, RadioButtonRendererTest, TestFaultOptionRenderer);
	ADD_CASE(testSuite, RadioButtonRendererTest, TestFaultOptionRendererOld);

	return testSuite;
}

/*===============================================================*/
/*     Check where all is correctly defined                      */
/*===============================================================*/
void RadioButtonRendererTest::TestCase0()
{
	fCurrentTestMethod = "RadioButton-TestCase0";
	this->TestField0();
	assertCharPtrEqual( _QUOTE_(<INPUT TYPE="RADIO" NAME="fld_the name of field" VALUE="the value of field" CHECKED the options of field>), fReply.str() );
}

void RadioButtonRendererTest::TestCase1()
{
	fCurrentTestMethod = "RadioButton-TestCase1";
	this->TestField1();
	assertCharPtrEqual( _QUOTE_(<INPUT TYPE="RADIO" NAME="fld_the name of field" VALUE="the value of field" CHECKED 0="the option nr. 0 of field" 1="the option nr. 1 of field" 2="the option nr. 2 of field">), fReply.str() );
}

void RadioButtonRendererTest::TestCase2()
{
	fCurrentTestMethod = "RadioButton-TestCase2";
	this->TestField2();
	assertCharPtrEqual( _QUOTE_(<INPUT TYPE="RADIO" NAME="fld_the name of field" VALUE="the value of field" CHECKED 0="the option nr. 0 of field" the option nr. 1 of field 2="the option nr. 2 of field">), fReply.str() );
}

void RadioButtonRendererTest::TestCase3()
{
	fCurrentTestMethod = "RadioButton-TestCase3";
	this->TestField3();
	assertCharPtrEqual( _QUOTE_(<INPUT TYPE="RADIO" NAME="fld_the name of field" VALUE="the value of field" CHECKED 0="the option nr. 0 of field" the option nr. 1 of field 2="the option nr. 2 of field">), fReply.str() );
}

/*===============================================================*/
/*     Check where not all is correctly defined                  */
/*===============================================================*/
void RadioButtonRendererTest::TestCaseEmptyConf()
{
	fCurrentTestMethod = "RadioButton-TestCaseEmptyConf";
	this->TestFieldWithoutName();
	assertCharPtrEqual( "", fReply.str() );
}

void RadioButtonRendererTest::TestCaseWithoutName()
{
	fCurrentTestMethod = "RadioButton-TestCaseWithoutName";
	this->TestFieldWithoutName();
	assertCharPtrEqual( "", fReply.str() );
}

void RadioButtonRendererTest::TestCaseWithoutLabel()
{
	fCurrentTestMethod = "RadioButton-TestCaseWithoutLabel";
	this->TestFieldWithoutLabel();
	assertCharPtrEqual( _QUOTE_(<INPUT TYPE="RADIO" NAME="fld_the name of field" VALUE="the value of field" CHECKED the options of field>), fReply.str() );
}

void RadioButtonRendererTest::TestCaseWithoutValue()
{
	fCurrentTestMethod = "RadioButton-TestFieldWithoutValue";
	this->TestFieldWithoutValue();
	assertCharPtrEqual( _QUOTE_(<INPUT TYPE="RADIO" NAME="fld_the name of field" CHECKED the options of field>), fReply.str() );
}

void RadioButtonRendererTest::TestCaseWithoutSource()
{
	fCurrentTestMethod = "RadioButton-TestFieldWithoutSource";
	this->TestFieldWithoutSource();
	assertCharPtrEqual( _QUOTE_(<INPUT TYPE="RADIO" NAME="fld_the name of field" CHECKED the options of field>), fReply.str() );
}

void RadioButtonRendererTest::TestCaseWithoutMultiple()
{
	fCurrentTestMethod = "RadioButton-TestFieldWithoutMultiple";
	this->TestFieldWithoutMultiple();
	assertCharPtrEqual( _QUOTE_(<INPUT TYPE="RADIO" NAME="fld_the name of field" VALUE="the value of field" CHECKED the options of field>), fReply.str() );
}

void RadioButtonRendererTest::TestCaseWithoutChecked()
{
	fCurrentTestMethod = "RadioButton-TestFieldWithoutChecked";
	this->TestFieldWithoutChecked();
	assertCharPtrEqual( _QUOTE_(<INPUT TYPE="RADIO" NAME="fld_the name of field" VALUE="the value of field" the options of field>), fReply.str() );
}

void RadioButtonRendererTest::TestCaseWithoutOptions()
{
	fCurrentTestMethod = "RadioButton-TestFieldWithoutOptions";
	this->TestFieldWithoutOptions();
	assertCharPtrEqual( _QUOTE_(<INPUT TYPE="RADIO" NAME="fld_the name of field" VALUE="the value of field" CHECKED>), fReply.str() );
}

/*===============================================================*/
/*     Check where there is a fault fConfig                      */
/*===============================================================*/
void RadioButtonRendererTest::TestCaseWrong()
{
	fCurrentTestMethod = "RadioButton-TestCaseWrong";
	this->TestFieldWithoutName();
	assertCharPtrEqual( "", fReply.str() );
}

/*===============================================================*/
/*     Check the indirection                                     */
/*===============================================================*/
void RadioButtonRendererTest::TestOptionRendererOld()
{
	fCurrentTestMethod = "RadioButtonRendererTest-TestOptionRendererOld";
	this->TestFieldOptionRendererOld();
	assertCharPtrEqual( _QUOTE_(<INPUT TYPE="RADIO" NAME="fld_the name of field" VALUE="the value of field" CHECKED 0="the option nr. 0 of field" 1="07.08.97" 2="the option nr. 2 of field">), fReply.str() );
}

void RadioButtonRendererTest::TestFaultOptionRendererOld()
{
	fCurrentTestMethod = "RadioButtonRendererTest-TestFaultOptionRendererOld";
	this->TestFieldFaultOptionRendererOld();
	assertCharPtrEqual( _QUOTE_(<INPUT TYPE="RADIO" NAME="fld_the name of field" VALUE="the value of field" CHECKED 0="the option nr. 0 of field" 1="" 2="the option nr. 2 of field">), fReply.str() );
}

void RadioButtonRendererTest::TestOptionRenderer()
{
	fCurrentTestMethod = "RadioButtonRendererTest-TestOptionRenderer";
	this->TestFieldOptionRenderer();
	assertCharPtrEqual( _QUOTE_(<INPUT TYPE="RADIO" NAME="fld_the name of field" VALUE="the value of field" CHECKED 0="the option nr. 0 of field" 1="07.08.97" 2="the option nr. 2 of field">), fReply.str() );
}

void RadioButtonRendererTest::TestFaultOptionRenderer()
{
	fCurrentTestMethod = "RadioButtonRendererTest-TestFaultOptionRenderer";
	this->TestFieldFaultOptionRenderer();
	assertCharPtrEqual( _QUOTE_(<INPUT TYPE="RADIO" NAME="fld_the name of field" VALUE="the value of field" CHECKED 0="the option nr. 0 of field" 1="%d.%m.%y870912000" 2="the option nr. 2 of field">), fReply.str() );
}

