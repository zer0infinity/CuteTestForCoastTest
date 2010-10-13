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
#include "TextFieldRendererTest.h"

TextFieldRendererTest::TextFieldRendererTest (TString tname) : FieldRendererTest(tname)
{
	fFieldRenderer = new (Storage::Global()) TextFieldRenderer("TextFieldRenderer");
};

TextFieldRendererTest::~TextFieldRendererTest()
{
	if (fFieldRenderer) {
		delete fFieldRenderer;
	}
	fFieldRenderer = 0;
};

void TextFieldRendererTest::setUp ()
{

}

void TextFieldRendererTest::TestCase0()
{
	fCurrentTestMethod = "TextField-TestCase0";
	this->TestField0();
	assertCharPtrEqual( _QUOTE_(<INPUT TYPE="PASSWORD" NAME="fld_the name of field" VALUE="the value of field" SIZE="50" MAXLENGTH="40" the options of field>), fReply.str() );

}

void TextFieldRendererTest::TestCase1()
{
	fCurrentTestMethod = "TextField-TestCase1";
	this->TestField1();
	assertCharPtrEqual( _QUOTE_(<INPUT TYPE="PASSWORD" NAME="fld_the name of field" VALUE="the value of field" SIZE="50" MAXLENGTH="40" 0="the option nr. 0 of field" 1="the option nr. 1 of field" 2="the option nr. 2 of field">), fReply.str() );

}

void TextFieldRendererTest::TestOptionRenderer()
{
	fCurrentTestMethod = "TextField-OptionRendere";
	this->TestFieldOptionRenderer();
	assertCharPtrEqual( _QUOTE_(<INPUT TYPE="PASSWORD" NAME="fld_the name of field" VALUE="the value of field" SIZE="50" MAXLENGTH="40" 0="the option nr. 0 of field" 1="07.08.97" 2="the option nr. 2 of field">), fReply.str() );

}

void TextFieldRendererTest::TestOptionRendererOld()
{
	fCurrentTestMethod = "TextField-OptionRendere";
	this->TestFieldOptionRendererOld();
	assertCharPtrEqual( _QUOTE_(<INPUT TYPE="PASSWORD" NAME="fld_the name of field" VALUE="the value of field" SIZE="50" MAXLENGTH="40" 0="the option nr. 0 of field" 1="07.08.97" 2="the option nr. 2 of field">), fReply.str() );

}

void TextFieldRendererTest::TestValue()
{
	fCurrentTestMethod = "TextField-TestCase1";
	this->TestFieldValue();
	assertCharPtrEqual( _QUOTE_(<INPUT TYPE="TEXT" NAME="fld_the name of field" VALUE="the value of field">), fReply.str() );

}

void TextFieldRendererTest::TestUnreadable()
{
	fCurrentTestMethod = "TextField-Unreadable";
	this->TestFieldUnreadable();
	assertCharPtrEqual( _QUOTE_(<INPUT TYPE="PASSWORD" NAME="fld_the name of field">), fReply.str() );

}

void TextFieldRendererTest::TestSize()
{
	fCurrentTestMethod = "TextField-Size";
	this->TestFieldSize();
	assertCharPtrEqual( _QUOTE_(<INPUT TYPE="TEXT" NAME="fld_the name of field" SIZE="50">), fReply.str() );

}

void TextFieldRendererTest::TestMaxlength()
{
	fCurrentTestMethod = "TextField-Maxlength";
	this->TestFieldMaxlength();
	assertCharPtrEqual( _QUOTE_(<INPUT TYPE="TEXT" NAME="fld_the name of field" MAXLENGTH="40">), fReply.str() );

}

void TextFieldRendererTest::Options4TextArea()
// test the date renderer with simple formatting strings
{
	fCurrentTestMethod = "TextField-Test4Opt";
	this->TestField4Opt();
	assertEqual( _QUOTE_(<INPUT TYPE="TEXT" NAME="fld_the name of field" 0="the option nr. 0 of field" 1="the option nr. 1 of field" 2="the option nr. 2 of field" 3="the option nr. 3 of field">), fReply.str());

}

void TextFieldRendererTest::Options3TextArea()
// test the date renderer with simple formatting strings
{
	fCurrentTestMethod = "TextField-Test3Opt";
	this->TestField3Opt();
	assertEqual( _QUOTE_(<INPUT TYPE="TEXT" NAME="fld_the name of field" 0="the option nr. 0 of field" 1="the option nr. 1 of field" 2="the option nr. 2 of field">), fReply.str());

}

void TextFieldRendererTest::Options2TextArea()
// test the date renderer with simple formatting strings
{
	fCurrentTestMethod = "TextField-Test2Opt";
	this->TestField2Opt();
	assertEqual( _QUOTE_(<INPUT TYPE="TEXT" NAME="fld_the name of field" 0="the option nr. 0 of field" 1="the option nr. 1 of field">), fReply.str());

}

void TextFieldRendererTest::Option1TextArea()
// test the date renderer with simple formatting strings
{
	fCurrentTestMethod = "TextField-Test1Opt";
	this->TestField1Opt();
	assertEqual( _QUOTE_(<INPUT TYPE="TEXT" NAME="fld_the name of field" 0="the option nr. 0 of field">), fReply.str());

}

void TextFieldRendererTest::OptionsNoSlotName2TextArea()
// test the date renderer with simple formatting strings
{
	fCurrentTestMethod = "TextField-Test2OptNoSlotName";
	this->TestField2OptNoSlotName();
	assertEqual( _QUOTE_(<INPUT TYPE="TEXT" NAME="fld_the name of field" the option nr. 0 of field the option nr. 1 of field>), fReply.str());

}

void TextFieldRendererTest::OptionNoSlotName1TextArea()
// test the date renderer with simple formatting strings
{
	fCurrentTestMethod = "TextField-Test1OptNoSlotName";
	this->TestField1OptNoSlotName();
	assertEqual( _QUOTE_(<INPUT TYPE="TEXT" NAME="fld_the name of field" the option nr. 0 of field>), fReply.str());

}

void TextFieldRendererTest::OptionsMixedTextArea()
// test the date renderer with simple formatting strings
{
	fCurrentTestMethod = "TextField-Test3OptMixed";
	this->TestField3OptMixed();
	assertEqual( _QUOTE_(<INPUT TYPE="TEXT" NAME="fld_the name of field" the option nr. 0 of field 1="the option nr. 1 of field" the option nr. 2 of field>), fReply.str());

}

void TextFieldRendererTest::NoOptionsTextArea()
// test the date renderer with simple formatting strings
{
	fCurrentTestMethod = "TextField-TestOptEmpty";
	this->TestFieldOptEmpty();
	assertEqual( _QUOTE_(<INPUT TYPE="TEXT" NAME="fld_the name of field">), fReply.str());

}

Test *TextFieldRendererTest::suite ()
{
	TestSuite *testSuite = new TestSuite;

	ADD_CASE(testSuite, TextFieldRendererTest, TestCase0);
	ADD_CASE(testSuite, TextFieldRendererTest, TestCase1);
	ADD_CASE(testSuite, TextFieldRendererTest, TestValue);
	ADD_CASE(testSuite, TextFieldRendererTest, TestOptionRenderer);
	ADD_CASE(testSuite, TextFieldRendererTest, TestOptionRendererOld);
	ADD_CASE(testSuite, TextFieldRendererTest, TestUnreadable);
	ADD_CASE(testSuite, TextFieldRendererTest, TestSize);
	ADD_CASE(testSuite, TextFieldRendererTest, TestMaxlength);
	ADD_CASE(testSuite, TextFieldRendererTest, Options4TextArea);
	ADD_CASE(testSuite, TextFieldRendererTest, Options3TextArea);
	ADD_CASE(testSuite, TextFieldRendererTest, Options2TextArea);
	ADD_CASE(testSuite, TextFieldRendererTest, Option1TextArea);
	ADD_CASE(testSuite, TextFieldRendererTest, OptionsNoSlotName2TextArea);
	ADD_CASE(testSuite, TextFieldRendererTest, OptionNoSlotName1TextArea);
	ADD_CASE(testSuite, TextFieldRendererTest, OptionsMixedTextArea);
	ADD_CASE(testSuite, TextFieldRendererTest, NoOptionsTextArea);

	return testSuite;
}
