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
#include "HiddenFieldRendererTest.h"

HiddenFieldRendererTest::HiddenFieldRendererTest (TString tname) : FieldRendererTest(tname)
{
	fFieldRenderer = new HiddenFieldRenderer("HiddenFieldRendererTest");
};

HiddenFieldRendererTest::~HiddenFieldRendererTest()
{
	if (fFieldRenderer) {
		delete fFieldRenderer;
	}
	fFieldRenderer = 0;
};

/*===============================================================*/
/*     Init                                                      */
/*===============================================================*/
void HiddenFieldRendererTest::setUp ()
// setup config for all the renderers in this TestCase
{
	FieldRendererTest::setUp();
} // setUp

Test *HiddenFieldRendererTest::suite ()
// collect all test cases for the HiddenFieldRenderer
{
	TestSuite *testSuite = new TestSuite;

	testSuite->addTest (NEW_CASE(HiddenFieldRendererTest, TestCaseEmptyConf));
	testSuite->addTest (NEW_CASE(HiddenFieldRendererTest, TestCase0));
	testSuite->addTest (NEW_CASE(HiddenFieldRendererTest, TestCase1));
	testSuite->addTest (NEW_CASE(HiddenFieldRendererTest, TestCase2));
	testSuite->addTest (NEW_CASE(HiddenFieldRendererTest, TestCase3));
	testSuite->addTest (NEW_CASE(HiddenFieldRendererTest, TestCaseWithoutName));
	testSuite->addTest (NEW_CASE(HiddenFieldRendererTest, TestCaseWithoutLabel));
	testSuite->addTest (NEW_CASE(HiddenFieldRendererTest, TestCaseWithoutValue));
	testSuite->addTest (NEW_CASE(HiddenFieldRendererTest, TestCaseWithoutSource));
	testSuite->addTest (NEW_CASE(HiddenFieldRendererTest, TestCaseWithoutMultiple));
	testSuite->addTest (NEW_CASE(HiddenFieldRendererTest, TestCaseWithoutChecked));
	testSuite->addTest (NEW_CASE(HiddenFieldRendererTest, TestCaseWithoutOptions));
	testSuite->addTest (NEW_CASE(HiddenFieldRendererTest, TestCaseWrong ));
	testSuite->addTest (NEW_CASE(HiddenFieldRendererTest, TestOptionRenderer));
	testSuite->addTest (NEW_CASE(HiddenFieldRendererTest, TestFaultOptionRenderer));
	testSuite->addTest (NEW_CASE(HiddenFieldRendererTest, TestOptionRendererOld));
	testSuite->addTest (NEW_CASE(HiddenFieldRendererTest, TestFaultOptionRendererOld ));

	return testSuite;

} // suite

/*===============================================================*/
/*     Check where all is correctly defined                      */
/*===============================================================*/
void HiddenFieldRendererTest::TestCase0()
{
	fCurrentTestMethod = "HiddenField-TestCase0";
	this->TestField0();
	assertCharPtrEqual( _QUOTE_(<INPUT TYPE="HIDDEN" NAME="fld_the name of field" VALUE="the value of field" the options of field>), fReply.str() );
}

void HiddenFieldRendererTest::TestCase1()
{
	fCurrentTestMethod = "HiddenField-TestCase1";
	this->TestField1();
	assertCharPtrEqual( _QUOTE_(<INPUT TYPE="HIDDEN" NAME="fld_the name of field" VALUE="the value of field" 0="the option nr. 0 of field" 1="the option nr. 1 of field" 2="the option nr. 2 of field">), fReply.str() );
}

void HiddenFieldRendererTest::TestCase2()
{
	fCurrentTestMethod = "HiddenField-TestCase2";
	this->TestField2();
	assertCharPtrEqual( _QUOTE_(<INPUT TYPE="HIDDEN" NAME="fld_the name of field" VALUE="the value of field" 0="the option nr. 0 of field" the option nr. 1 of field 2="the option nr. 2 of field">), fReply.str() );
}

void HiddenFieldRendererTest::TestCase3()
{
	fCurrentTestMethod = "HiddenField-TestCase3";
	this->TestField3();
	assertCharPtrEqual( _QUOTE_(<INPUT TYPE="HIDDEN" NAME="fld_the name of field" VALUE="the value of field" 0="the option nr. 0 of field" the option nr. 1 of field 2="the option nr. 2 of field">), fReply.str() );
}

/*===============================================================*/
/*     Check where not all is correctly defined                  */
/*===============================================================*/
void HiddenFieldRendererTest::TestCaseEmptyConf()
{
	fCurrentTestMethod = "HiddenField-TestCaseEmptyConf";
	this->TestEmptyConf();
	assertCharPtrEqual( "", fReply.str() );
}

void HiddenFieldRendererTest::TestCaseWithoutName()
{
	fCurrentTestMethod = "HiddenField-TestCaseWithoutName";
	this->TestFieldWithoutName();
	assertCharPtrEqual( "", fReply.str() );
}

void HiddenFieldRendererTest::TestCaseWithoutLabel()
{
	fCurrentTestMethod = "HiddenField-TestCaseWithoutLabel";
	this->TestFieldWithoutLabel();
	assertCharPtrEqual( _QUOTE_(<INPUT TYPE="HIDDEN" NAME="fld_the name of field" VALUE="the value of field" the options of field>), fReply.str() );
}

void HiddenFieldRendererTest::TestCaseWithoutValue()
{
	fCurrentTestMethod = "HiddenField-TestFieldWithoutValue";
	this->TestFieldWithoutValue();
	assertCharPtrEqual( _QUOTE_(<INPUT TYPE="HIDDEN" NAME="fld_the name of field" the options of field>), fReply.str() );
}

void HiddenFieldRendererTest::TestCaseWithoutSource()
{
	fCurrentTestMethod = "HiddenField-TestFieldWithoutSource";
	this->TestFieldWithoutSource();
	assertCharPtrEqual( _QUOTE_(<INPUT TYPE="HIDDEN" NAME="fld_the name of field" the options of field>), fReply.str() );
}

void HiddenFieldRendererTest::TestCaseWithoutMultiple()
{
	fCurrentTestMethod = "HiddenField-TestFieldWithoutMultiple";
	this->TestFieldWithoutMultiple();
	assertCharPtrEqual( _QUOTE_(<INPUT TYPE="HIDDEN" NAME="fld_the name of field" VALUE="the value of field" the options of field>), fReply.str() );
}

void HiddenFieldRendererTest::TestCaseWithoutChecked()
{
	fCurrentTestMethod = "HiddenField-TestFieldWithoutChecked";
	this->TestFieldWithoutChecked();
	assertCharPtrEqual( _QUOTE_(<INPUT TYPE="HIDDEN" NAME="fld_the name of field" VALUE="the value of field" the options of field>), fReply.str() );
}

void HiddenFieldRendererTest::TestCaseWithoutOptions()
{
	fCurrentTestMethod = "HiddenField-TestFieldWithoutOptions";
	this->TestFieldWithoutOptions();
	assertCharPtrEqual( _QUOTE_(<INPUT TYPE="HIDDEN" NAME="fld_the name of field" VALUE="the value of field">), fReply.str() );
}

/*===============================================================*/
/*     Check where there is a fault fConfig                      */
/*===============================================================*/
void HiddenFieldRendererTest::TestCaseWrong()
{
	fCurrentTestMethod = "HiddenField-TestCaseWrong";
	this->TestFieldWithoutName();
	assertCharPtrEqual( "", fReply.str() );
}

/*===============================================================*/
/*     Check the indirection                                     */
/*===============================================================*/
void HiddenFieldRendererTest::TestOptionRendererOld()
{
	fCurrentTestMethod = "HiddenFieldRendererTest-TestOptionRendererOld";
	this->TestFieldOptionRendererOld();
	assertCharPtrEqual( _QUOTE_(<INPUT TYPE="HIDDEN" NAME="fld_the name of field" VALUE="the value of field" 0="the option nr. 0 of field" 1="07.08.97" 2="the option nr. 2 of field">), fReply.str() );
}

void HiddenFieldRendererTest::TestFaultOptionRendererOld()
{
	fCurrentTestMethod = "HiddenFieldRendererTest-TestFaultOptionRendererOld";
	this->TestFieldFaultOptionRendererOld();
	assertCharPtrEqual( _QUOTE_(<INPUT TYPE="HIDDEN" NAME="fld_the name of field" VALUE="the value of field" 0="the option nr. 0 of field" 1="" 2="the option nr. 2 of field">), fReply.str() );
}

void HiddenFieldRendererTest::TestOptionRenderer()
{
	fCurrentTestMethod = "HiddenFieldRendererTest-TestOptionRenderer";
	this->TestFieldOptionRenderer();
	assertCharPtrEqual( _QUOTE_(<INPUT TYPE="HIDDEN" NAME="fld_the name of field" VALUE="the value of field" 0="the option nr. 0 of field" 1="07.08.97" 2="the option nr. 2 of field">), fReply.str() );
}

void HiddenFieldRendererTest::TestFaultOptionRenderer()
{
	fCurrentTestMethod = "HiddenFieldRendererTest-TestFaultOptionRenderer";
	this->TestFieldFaultOptionRenderer();
	assertCharPtrEqual( _QUOTE_(<INPUT TYPE="HIDDEN" NAME="fld_the name of field" VALUE="the value of field" 0="the option nr. 0 of field" 1="%d.%m.%y870912000" 2="the option nr. 2 of field">), fReply.str() );
}
