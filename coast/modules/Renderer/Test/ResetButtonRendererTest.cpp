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
#include "ResetButtonRendererTest.h"

ResetButtonRendererTest::ResetButtonRendererTest (TString tname) : FieldRendererTest(tname)
{
	fFieldRenderer = new ResetButtonRenderer("ResetButtonRenderer");
};

ResetButtonRendererTest::~ResetButtonRendererTest()
{
	if (fFieldRenderer) {
		delete fFieldRenderer;
	}
	fFieldRenderer = 0;
};

/*===============================================================*/
/*     Init                                                      */
/*===============================================================*/
void ResetButtonRendererTest::setUp ()
// setup config for all the renderers in this TestCase
{
	FieldRendererTest::setUp();
} // setUp

Test *ResetButtonRendererTest::suite ()
// collect all test cases for the ResetButtonRenderer
{
	TestSuite *testSuite = new TestSuite;

	testSuite->addTest (NEW_CASE(ResetButtonRendererTest, TestCaseEmptyConf));
	testSuite->addTest (NEW_CASE(ResetButtonRendererTest, TestCase0));
	testSuite->addTest (NEW_CASE(ResetButtonRendererTest, TestCase1));
	testSuite->addTest (NEW_CASE(ResetButtonRendererTest, TestCase2));
	testSuite->addTest (NEW_CASE(ResetButtonRendererTest, TestCase3));
	testSuite->addTest (NEW_CASE(ResetButtonRendererTest, TestCaseWithoutName));
	testSuite->addTest (NEW_CASE(ResetButtonRendererTest, TestCaseWithoutLabel));
	testSuite->addTest (NEW_CASE(ResetButtonRendererTest, TestCaseWithoutValue));
	testSuite->addTest (NEW_CASE(ResetButtonRendererTest, TestCaseWithoutSource));
	testSuite->addTest (NEW_CASE(ResetButtonRendererTest, TestCaseWithoutMultiple));
	testSuite->addTest (NEW_CASE(ResetButtonRendererTest, TestCaseWithoutChecked));
	testSuite->addTest (NEW_CASE(ResetButtonRendererTest, TestCaseWithoutOptions));
	testSuite->addTest (NEW_CASE(ResetButtonRendererTest, TestCaseWrong ));
	testSuite->addTest (NEW_CASE(ResetButtonRendererTest, TestOptionRenderer ));
	testSuite->addTest (NEW_CASE(ResetButtonRendererTest, TestOptionRendererOld ));
	testSuite->addTest (NEW_CASE(ResetButtonRendererTest, TestFaultOptionRenderer ));
	testSuite->addTest (NEW_CASE(ResetButtonRendererTest, TestFaultOptionRendererOld ));

	return testSuite;

} // suite

/*===============================================================*/
/*     Check where all is correctly defined                      */
/*===============================================================*/
void ResetButtonRendererTest::TestCase0()
{
	fCurrentTestMethod = "ResetButton-TestCase0";
	this->TestField0();
	assertCharPtrEqual( _QUOTE_(<INPUT TYPE="RESET" NAME="b_the name of field" VALUE="the label of field" the options of field>), fReply.str() );
}

void ResetButtonRendererTest::TestCase1()
{
	fCurrentTestMethod = "ResetButton-TestCase1";
	this->TestField1();
	assertCharPtrEqual( _QUOTE_(<INPUT TYPE="RESET" NAME="b_the name of field" VALUE="the label of field" 0="the option nr. 0 of field" 1="the option nr. 1 of field" 2="the option nr. 2 of field">), fReply.str() );
}

void ResetButtonRendererTest::TestCase2()
{
	fCurrentTestMethod = "ResetButton-TestCase2";
	this->TestField2();
	assertCharPtrEqual( _QUOTE_(<INPUT TYPE="RESET" NAME="b_the name of field" VALUE="the label of field" 0="the option nr. 0 of field" the option nr. 1 of field 2="the option nr. 2 of field">), fReply.str() );
}

void ResetButtonRendererTest::TestCase3()
{
	fCurrentTestMethod = "ResetButton-TestCase3";
	this->TestField3();
	assertCharPtrEqual( _QUOTE_(<INPUT TYPE="RESET" NAME="b_the name of field" VALUE="the label of field" 0="the option nr. 0 of field" the option nr. 1 of field 2="the option nr. 2 of field">), fReply.str() );
}

/*===============================================================*/
/*     Check where not all is correctly defined                  */
/*===============================================================*/
void ResetButtonRendererTest::TestCaseEmptyConf()
{
	fCurrentTestMethod = "ResetButton-TestCaseEmptyConf";
	this->TestEmptyConf();
	assertCharPtrEqual( "", fReply.str() );
}

void ResetButtonRendererTest::TestCaseWithoutName()
{
	fCurrentTestMethod = "ResetButton-TestCaseWithoutName";
	this->TestFieldWithoutName();
	assertCharPtrEqual( "", fReply.str() );
}

void ResetButtonRendererTest::TestCaseWithoutLabel()
{
	fCurrentTestMethod = "ResetButton-TestCaseWithoutLabel";
	this->TestFieldWithoutLabel();
	assertCharPtrEqual( _QUOTE_(<INPUT TYPE="RESET" NAME="b_the name of field" the options of field>), fReply.str() );
}

void ResetButtonRendererTest::TestCaseWithoutValue()
{
	fCurrentTestMethod = "ResetButton-TestFieldWithoutValue";
	this->TestFieldWithoutValue();
	assertCharPtrEqual( _QUOTE_(<INPUT TYPE="RESET" NAME="b_the name of field" VALUE="the label of field" the options of field>), fReply.str() );
}

void ResetButtonRendererTest::TestCaseWithoutSource()
{
	fCurrentTestMethod = "ResetButton-TestFieldWithoutSource";
	this->TestFieldWithoutSource();
	assertCharPtrEqual( _QUOTE_(<INPUT TYPE="RESET" NAME="b_the name of field" VALUE="the label of field" the options of field>), fReply.str() );
}

void ResetButtonRendererTest::TestCaseWithoutMultiple()
{
	fCurrentTestMethod = "ResetButton-TestCaseWithoutMultiple";
	this->TestFieldWithoutMultiple();
	assertCharPtrEqual( _QUOTE_(<INPUT TYPE="RESET" NAME="b_the name of field" VALUE="the label of field" the options of field>), fReply.str() );
}

void ResetButtonRendererTest::TestCaseWithoutChecked()
{
	fCurrentTestMethod = "ResetButton-TestCaseWithoutChecked";
	this->TestFieldWithoutChecked();
	assertCharPtrEqual( _QUOTE_(<INPUT TYPE="RESET" NAME="b_the name of field" VALUE="the label of field" the options of field>), fReply.str() );
}

void ResetButtonRendererTest::TestCaseWithoutOptions()
{
	fCurrentTestMethod = "ResetButton-TestFieldWithoutOptions";
	this->TestFieldWithoutOptions();
	assertCharPtrEqual( _QUOTE_(<INPUT TYPE="RESET" NAME="b_the name of field" VALUE="the label of field">), fReply.str() );
}

/*===============================================================*/
/*     Check where there is a fault fConfig                      */
/*===============================================================*/
void ResetButtonRendererTest::TestCaseWrong()
{
	fCurrentTestMethod = "ResetButton-TestFieldWithoutOptions";
	this->TestFieldWithoutName();
	assertCharPtrEqual( "", fReply.str() );
}

/*===============================================================*/
/*     Check the indirection                                     */
/*===============================================================*/
void ResetButtonRendererTest::TestOptionRendererOld()
{
	fCurrentTestMethod = "ResetButtonRendererTest-TestOptionRendererOld";
	this->TestFieldOptionRendererOld();
	assertCharPtrEqual( _QUOTE_(<INPUT TYPE="RESET" NAME="b_the name of field" VALUE="the label of field" 0="the option nr. 0 of field" 1="07.08.97" 2="the option nr. 2 of field">), fReply.str() );
}

void ResetButtonRendererTest::TestFaultOptionRendererOld()
{
	fCurrentTestMethod = "ResetButtonRendererTest-TestFaultOptionRendererOld";
	this->TestFieldFaultOptionRendererOld();
	assertCharPtrEqual( _QUOTE_(<INPUT TYPE="RESET" NAME="b_the name of field" VALUE="the label of field" 0="the option nr. 0 of field" 1="" 2="the option nr. 2 of field">), fReply.str() );
}

void ResetButtonRendererTest::TestOptionRenderer()
{
	fCurrentTestMethod = "ResetButtonRendererTest-TestOptionRenderer";
	this->TestFieldOptionRenderer();
	assertCharPtrEqual( _QUOTE_(<INPUT TYPE="RESET" NAME="b_the name of field" VALUE="the label of field" 0="the option nr. 0 of field" 1="07.08.97" 2="the option nr. 2 of field">), fReply.str() );
}

void ResetButtonRendererTest::TestFaultOptionRenderer()
{
	fCurrentTestMethod = "ResetButtonRendererTest-TestFaultOptionRenderer";
	this->TestFieldFaultOptionRenderer();
	assertCharPtrEqual( _QUOTE_(<INPUT TYPE="RESET" NAME="b_the name of field" VALUE="the label of field" 0="the option nr. 0 of field" 1="%d.%m.%y870912000" 2="the option nr. 2 of field">), fReply.str() );
}
