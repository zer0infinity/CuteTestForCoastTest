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
#include "ButtonRendererTest.h"

ButtonRendererTest::ButtonRendererTest (TString tname) : FieldRendererTest(tname)
{
	fFieldRenderer = new ButtonRenderer("ButtonRenderer");
};

ButtonRendererTest::~ButtonRendererTest()
{
	if (fFieldRenderer) {
		delete fFieldRenderer;
	}
	fFieldRenderer = 0;
};

/*===============================================================*/
/*     Init                                                      */
/*===============================================================*/
void ButtonRendererTest::setUp ()
// setup config for all the renderers in this TestCase
{
	FieldRendererTest::setUp();
} // setUp

Test *ButtonRendererTest::suite ()
// collect all test cases for the ButtonRenderer
{
	TestSuite *testSuite = new TestSuite;

	testSuite->addTest (NEW_CASE(ButtonRendererTest, TestCaseEmptyConf));
	testSuite->addTest (NEW_CASE(ButtonRendererTest, TestCase0));
	testSuite->addTest (NEW_CASE(ButtonRendererTest, TestCase1));
	testSuite->addTest (NEW_CASE(ButtonRendererTest, TestCase2));
	testSuite->addTest (NEW_CASE(ButtonRendererTest, TestCase3));
	testSuite->addTest (NEW_CASE(ButtonRendererTest, TestCaseWithoutName));
	testSuite->addTest (NEW_CASE(ButtonRendererTest, TestCaseWithoutLabel));
	testSuite->addTest (NEW_CASE(ButtonRendererTest, TestCaseWithoutValue));
	testSuite->addTest (NEW_CASE(ButtonRendererTest, TestCaseWithoutSource));
	testSuite->addTest (NEW_CASE(ButtonRendererTest, TestCaseWithoutMultiple));
	testSuite->addTest (NEW_CASE(ButtonRendererTest, TestCaseWithoutChecked ));
	testSuite->addTest (NEW_CASE(ButtonRendererTest, TestCaseWithoutOptions));
	testSuite->addTest (NEW_CASE(ButtonRendererTest, TestCaseWrong ));
	testSuite->addTest (NEW_CASE(ButtonRendererTest, TestOptionRenderer ));
	testSuite->addTest (NEW_CASE(ButtonRendererTest, TestOptionRendererOld ));
	testSuite->addTest (NEW_CASE(ButtonRendererTest, TestFaultOptionRenderer ));
	testSuite->addTest (NEW_CASE(ButtonRendererTest, TestFaultOptionRendererOld ));

	return testSuite;
} // suite

/*===============================================================*/
/*     Check where all is correctly defined                      */
/*===============================================================*/
void ButtonRendererTest::TestCase0()
{
	fCurrentTestMethod = "ButtonRendererTest-TestCase0";
	this->TestField0();
	assertCharPtrEqual( _QUOTE_(<INPUT TYPE="SUBMIT" NAME="b_the name of field" VALUE="the label of field" the options of field>), fReply.str() );
}

void ButtonRendererTest::TestCase1()
{
	fCurrentTestMethod = "ButtonRendererTest-TestCase1";
	this->TestField1();
	assertCharPtrEqual( _QUOTE_(<INPUT TYPE="SUBMIT" NAME="b_the name of field" VALUE="the label of field" 0="the option nr. 0 of field" 1="the option nr. 1 of field" 2="the option nr. 2 of field">), fReply.str() );
}

void ButtonRendererTest::TestCase2()
{
	fCurrentTestMethod = "ButtonRendererTest-TestCase2";
	this->TestField2();
	assertCharPtrEqual( _QUOTE_(<INPUT TYPE="SUBMIT" NAME="b_the name of field" VALUE="the label of field" 0="the option nr. 0 of field" the option nr. 1 of field 2="the option nr. 2 of field">), fReply.str() );
}

void ButtonRendererTest::TestCase3()
{
	fCurrentTestMethod = "ButtonRendererTest-TestCase3";
	this->TestField3();
	assertCharPtrEqual( _QUOTE_(<INPUT TYPE="SUBMIT" NAME="b_the name of field" VALUE="the label of field" 0="the option nr. 0 of field" the option nr. 1 of field 2="the option nr. 2 of field">), fReply.str() );
}

/*===============================================================*/
/*     Check where not all is correctly defined                  */
/*===============================================================*/
void ButtonRendererTest::TestCaseEmptyConf()
{
	fCurrentTestMethod = "ButtonRendererTest-TestCaseEmptyConf";
	this->TestEmptyConf();
	assertCharPtrEqual( "", fReply.str() );
}

void ButtonRendererTest::TestCaseWithoutName()
{
	fCurrentTestMethod = "ButtonRendererTest-TestCaseWithoutName";
	this->TestFieldWithoutName();
	assertCharPtrEqual( "", fReply.str() );
}

void ButtonRendererTest::TestCaseWithoutLabel()
{
	fCurrentTestMethod = "ButtonRendererTest-TestCaseWithoutLabel";
	this->TestFieldWithoutLabel();
	assertCharPtrEqual( _QUOTE_(<INPUT TYPE="SUBMIT" NAME="b_the name of field" the options of field>), fReply.str() );
}

void ButtonRendererTest::TestCaseWithoutValue()
{
	fCurrentTestMethod = "ButtonRendererTest-TestFieldWithoutValue";
	this->TestFieldWithoutValue();
	assertCharPtrEqual( _QUOTE_(<INPUT TYPE="SUBMIT" NAME="b_the name of field" VALUE="the label of field" the options of field>), fReply.str() );
}

void ButtonRendererTest::TestCaseWithoutSource()
{
	fCurrentTestMethod = "ButtonRendererTest-TestFieldWithoutSource";
	this->TestFieldWithoutSource();
	assertCharPtrEqual( _QUOTE_(<INPUT TYPE="SUBMIT" NAME="b_the name of field" VALUE="the label of field" the options of field>), fReply.str() );
}

void ButtonRendererTest::TestCaseWithoutMultiple()
{
	fCurrentTestMethod = "ButtonRendererTest-TestFieldWithoutMultiple";
	this->TestFieldWithoutMultiple();
	assertCharPtrEqual( _QUOTE_(<INPUT TYPE="SUBMIT" NAME="b_the name of field" VALUE="the label of field" the options of field>), fReply.str() );
}

void ButtonRendererTest::TestCaseWithoutChecked()
{
	fCurrentTestMethod = "ButtonRendererTest-TestFieldWithoutChecked";
	this->TestFieldWithoutChecked();
	assertCharPtrEqual( _QUOTE_(<INPUT TYPE="SUBMIT" NAME="b_the name of field" VALUE="the label of field" the options of field>), fReply.str() );
}

void ButtonRendererTest::TestCaseWithoutOptions()
{
	fCurrentTestMethod = "ButtonRendererTest-TestFieldWithoutOptions";
	this->TestFieldWithoutOptions();
	assertCharPtrEqual( _QUOTE_(<INPUT TYPE="SUBMIT" NAME="b_the name of field" VALUE="the label of field">), fReply.str() );
}

/*===============================================================*/
/*     Check where there is a fault fConfig                      */
/*===============================================================*/
void ButtonRendererTest::TestCaseWrong()
{
	fCurrentTestMethod = "ButtonRendererTest-TestFieldWithoutOptions";
	this->TestFieldWithoutName();
	assertCharPtrEqual( "", fReply.str() );
}

/*===============================================================*/
/*     Check the indirection                                     */
/*===============================================================*/
void ButtonRendererTest::TestOptionRendererOld()
{
	fCurrentTestMethod = "ButtonRendererTest-TestOptionRendererOld";
	this->TestFieldOptionRendererOld();
	assertCharPtrEqual( _QUOTE_(<INPUT TYPE="SUBMIT" NAME="b_the name of field" VALUE="the label of field" 0="the option nr. 0 of field" 1="07.08.97" 2="the option nr. 2 of field">), fReply.str() );
}

void ButtonRendererTest::TestFaultOptionRendererOld()
{
	fCurrentTestMethod = "ButtonRendererTest-TestFaultOptionRendererOld";
	this->TestFieldFaultOptionRendererOld();
	assertCharPtrEqual( _QUOTE_(<INPUT TYPE="SUBMIT" NAME="b_the name of field" VALUE="the label of field" 0="the option nr. 0 of field" 1="" 2="the option nr. 2 of field">), fReply.str() );
}

void ButtonRendererTest::TestOptionRenderer()
{
	fCurrentTestMethod = "ButtonRendererTest-TestOptionRenderer";
	this->TestFieldOptionRenderer();
	assertCharPtrEqual( _QUOTE_(<INPUT TYPE="SUBMIT" NAME="b_the name of field" VALUE="the label of field" 0="the option nr. 0 of field" 1="07.08.97" 2="the option nr. 2 of field">), fReply.str() );
}

void ButtonRendererTest::TestFaultOptionRenderer()
{
	fCurrentTestMethod = "ButtonRendererTest-TestFaultOptionRenderer";
	this->TestFieldFaultOptionRenderer();
	assertCharPtrEqual( _QUOTE_(<INPUT TYPE="SUBMIT" NAME="b_the name of field" VALUE="the label of field" 0="the option nr. 0 of field" 1="%d.%m.%y870912000" 2="the option nr. 2 of field">), fReply.str() );
}
