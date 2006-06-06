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
#include "TextAreaRendererTest.h"

TextAreaRendererTest::TextAreaRendererTest (TString tname) : FieldRendererTest(tname)
{
	fFieldRenderer = new TextAreaRenderer("TextAreaRenderer");
}
TextAreaRendererTest::~TextAreaRendererTest()
{
	if (fFieldRenderer) {
		delete fFieldRenderer;
	}
	fFieldRenderer = 0;
}

/*===============================================================*/
/*     Init                                                      */
/*===============================================================*/
void TextAreaRendererTest::setUp ()
{
	FieldRendererTest::setUp();
}

Test *TextAreaRendererTest::suite ()
{
	TestSuite *testSuite = new TestSuite;

	// Generic test done for each type of FieldRenderer
	ADD_CASE(testSuite, TextAreaRendererTest, TestCaseEmptyConf);
	ADD_CASE(testSuite, TextAreaRendererTest, TestCase0);
	ADD_CASE(testSuite, TextAreaRendererTest, TestCase1);
	ADD_CASE(testSuite, TextAreaRendererTest, TestCase2);
	ADD_CASE(testSuite, TextAreaRendererTest, TestCase3);
	ADD_CASE(testSuite, TextAreaRendererTest, TestCaseWithoutName);
	ADD_CASE(testSuite, TextAreaRendererTest, TestCaseWithoutLabel);
	ADD_CASE(testSuite, TextAreaRendererTest, TestCaseWithoutValue);
	ADD_CASE(testSuite, TextAreaRendererTest, TestCaseWithoutSource);
	ADD_CASE(testSuite, TextAreaRendererTest, TestCaseWithoutMultiple);
	ADD_CASE(testSuite, TextAreaRendererTest, TestCaseWithoutChecked);
	ADD_CASE(testSuite, TextAreaRendererTest, TestCaseWithoutOptions);
	ADD_CASE(testSuite, TextAreaRendererTest, TestCaseWrong);
	ADD_CASE(testSuite, TextAreaRendererTest, TestOptionRenderer);
	ADD_CASE(testSuite, TextAreaRendererTest, TestOptionRendererOld);
	ADD_CASE(testSuite, TextAreaRendererTest, TestFaultOptionRenderer);
	ADD_CASE(testSuite, TextAreaRendererTest, TestFaultOptionRendererOld);

	// Specific test ONLY for TextAreaRenderer
	ADD_CASE(testSuite, TextAreaRendererTest, AllAttributesTextArea);
	ADD_CASE(testSuite, TextAreaRendererTest, WidthTextArea);
	ADD_CASE(testSuite, TextAreaRendererTest, HeightTextArea);
	ADD_CASE(testSuite, TextAreaRendererTest, WrapTextArea);
	ADD_CASE(testSuite, TextAreaRendererTest, Options4TextArea);
	ADD_CASE(testSuite, TextAreaRendererTest, Options3TextArea);
	ADD_CASE(testSuite, TextAreaRendererTest, Options2TextArea);
	ADD_CASE(testSuite, TextAreaRendererTest, Option1TextArea);
	ADD_CASE(testSuite, TextAreaRendererTest, OptionsNoSlotName2TextArea);
	ADD_CASE(testSuite, TextAreaRendererTest, OptionNoSlotName1TextArea);
	ADD_CASE(testSuite, TextAreaRendererTest, OptionsMixedTextArea);
	ADD_CASE(testSuite, TextAreaRendererTest, NoOptionsTextArea);
	ADD_CASE(testSuite, TextAreaRendererTest, TextTextArea);

	return testSuite;

}

/*=================================================================================*/
/* Generic tests done for each type of FieldRendererTest                           */
/*=================================================================================*/
/*===============================================================*/
/*     Check where all is correctly defined                      */
/*===============================================================*/
void TextAreaRendererTest::TestCase0()
{
	fCurrentTestMethod = "TextAreaRendererTest-TestCase0";
	this->TestField0();
	assertCharPtrEqual( _QUOTE_(<TEXTAREA NAME="fld_the name of field" COLS=30 ROWS=20 WRAP=10 the options of field>the value of field</TEXTAREA>), fReply.str() );
}

void TextAreaRendererTest::TestCase1()
{
	fCurrentTestMethod = "TextAreaRendererTest-TestCase1";
	this->TestField1();
	assertCharPtrEqual( _QUOTE_(<TEXTAREA NAME="fld_the name of field" COLS=30 ROWS=20 WRAP=10 0="the option nr. 0 of field" 1="the option nr. 1 of field" 2="the option nr. 2 of field">the value of field</TEXTAREA>), fReply.str() );
}

void TextAreaRendererTest::TestCase2()
{
	fCurrentTestMethod = "TextAreaRendererTest-TestCase2";
	this->TestField2();
	assertCharPtrEqual( _QUOTE_(<TEXTAREA NAME="fld_the name of field" 0="the option nr. 0 of field" the option nr. 1 of field 2="the option nr. 2 of field">the value of field</TEXTAREA>), fReply.str() );
}

void TextAreaRendererTest::TestCase3()
{
	fCurrentTestMethod = "TextAreaRendererTest-TestCase3";
	this->TestField3();
	assertCharPtrEqual( _QUOTE_(<TEXTAREA NAME="fld_the name of field" 0="the option nr. 0 of field" the option nr. 1 of field 2="the option nr. 2 of field">the value of field</TEXTAREA>), fReply.str() );
}

/*===============================================================*/
/*     Check where not all is correctly defined                  */
/*===============================================================*/
void TextAreaRendererTest::TestCaseEmptyConf()
{
	fCurrentTestMethod = "TextAreaRendererTest-TestCaseEmptyConf";
	this->TestEmptyConf();
	assertCharPtrEqual( "", fReply.str() );
}

void TextAreaRendererTest::TestCaseWithoutName()
{
	fCurrentTestMethod = "TextAreaRendererTest-TestCaseWithoutName";
	this->TestFieldWithoutName();
	assertCharPtrEqual( "", fReply.str() );
}

void TextAreaRendererTest::TestCaseWithoutLabel()
{
	fCurrentTestMethod = "TextAreaRendererTest-TestCaseWithoutLabel";
	this->TestFieldWithoutLabel();
	assertCharPtrEqual( _QUOTE_(<TEXTAREA NAME="fld_the name of field" COLS=30 ROWS=20 WRAP=10 the options of field>the value of field</TEXTAREA>), fReply.str() );
}

void TextAreaRendererTest::TestCaseWithoutValue()
{
	fCurrentTestMethod = "TextAreaRendererTest-TestFieldWithoutValue";
	this->TestFieldWithoutValue();
	assertCharPtrEqual( _QUOTE_(<TEXTAREA NAME="fld_the name of field" COLS=30 ROWS=20 WRAP=10 the options of field></TEXTAREA>), fReply.str() );
}

void TextAreaRendererTest::TestCaseWithoutSource()
{
	fCurrentTestMethod = "TextAreaRendererTest-TestFieldWithoutSource";
	this->TestFieldWithoutSource();
	assertCharPtrEqual( _QUOTE_(<TEXTAREA NAME="fld_the name of field" COLS=30 ROWS=20 WRAP=10 the options of field></TEXTAREA>), fReply.str() );
}

void TextAreaRendererTest::TestCaseWithoutMultiple()
{
	fCurrentTestMethod = "TextAreaRendererTest-TestFieldWithoutMultiple";
	this->TestFieldWithoutMultiple();
	assertCharPtrEqual( _QUOTE_(<TEXTAREA NAME="fld_the name of field" COLS=30 ROWS=20 WRAP=10 the options of field>the value of field</TEXTAREA>), fReply.str() );
}

void TextAreaRendererTest::TestCaseWithoutChecked()
{
	fCurrentTestMethod = "TextAreaRendererTest-TestFieldWithoutChecked";
	this->TestFieldWithoutChecked();
	assertCharPtrEqual( _QUOTE_(<TEXTAREA NAME="fld_the name of field" COLS=30 ROWS=20 WRAP=10 the options of field>the value of field</TEXTAREA>), fReply.str() );
}

void TextAreaRendererTest::TestCaseWithoutOptions()
{
	fCurrentTestMethod = "TextAreaRendererTest-TestFieldWithoutOptions";
	this->TestFieldWithoutOptions();
	assertCharPtrEqual( _QUOTE_(<TEXTAREA NAME="fld_the name of field" COLS=30 ROWS=20 WRAP=10>the value of field</TEXTAREA>), fReply.str() );
}

/*===============================================================*/
/*     Check where there is a fault fConfig                      */
/*===============================================================*/
void TextAreaRendererTest::TestCaseWrong()
{
	fCurrentTestMethod = "TextAreaRendererTest-TestFieldWithoutOptions";
	this->TestFieldWithoutName();
	assertCharPtrEqual( "", fReply.str() );
}

/*=================================================================================*/
/* Special testcases ONLY for TextAreaRenderer                                     */
/*=================================================================================*/
void TextAreaRendererTest::AllAttributesTextArea()
// test the date renderer with simple formatting strings
{

	fCurrentTestMethod = "TextAreaRendererTest-AllAttributesTextArea";
	this->TestField0();
	assertCharPtrEqual( _QUOTE_(<TEXTAREA NAME="fld_the name of field" COLS=30 ROWS=20 WRAP=10 the options of field>the value of field</TEXTAREA>), fReply.str() );
}

void TextAreaRendererTest::WidthTextArea()
// test the date renderer with simple formatting strings
{

	fCurrentTestMethod = "TextAreaRendererTest-TestFieldWidth";
	this->TestFieldWidth();
	assertEqual( _QUOTE_(<TEXTAREA NAME="fld_the name of field" COLS=30></TEXTAREA>), fReply.str());

}
void TextAreaRendererTest::HeightTextArea()
// test the date renderer with simple formatting strings
{

	fCurrentTestMethod = "TextAreaRendererTest-TestFieldHeight";
	this->TestFieldHeight();
	assertEqual( _QUOTE_(<TEXTAREA NAME="fld_the name of field" ROWS=20></TEXTAREA>), fReply.str());
}

void TextAreaRendererTest::WrapTextArea()
// test the date renderer with simple formatting strings
{

	fCurrentTestMethod = "TextAreaRendererTest-TestFieldWrap";
	this->TestFieldWrap();
	assertEqual( _QUOTE_(<TEXTAREA NAME="fld_the name of field" WRAP=10></TEXTAREA>), fReply.str());

}

void TextAreaRendererTest::Options4TextArea()
// test the date renderer with simple formatting strings
{

	fCurrentTestMethod = "TextAreaRendererTest-Test4Opt";
	this->TestField4Opt();
	assertEqual( _QUOTE_(<TEXTAREA NAME="fld_the name of field" 0="the option nr. 0 of field" 1="the option nr. 1 of field" 2="the option nr. 2 of field" 3="the option nr. 3 of field"></TEXTAREA>), fReply.str());

}

void TextAreaRendererTest::Options3TextArea()
// test the date renderer with simple formatting strings
{

	fCurrentTestMethod = "TextAreaRendererTest-Test3Opt";
	this->TestField3Opt();
	assertEqual( _QUOTE_(<TEXTAREA NAME="fld_the name of field" 0="the option nr. 0 of field" 1="the option nr. 1 of field" 2="the option nr. 2 of field"></TEXTAREA>), fReply.str());

}

void TextAreaRendererTest::Options2TextArea()
// test the date renderer with simple formatting strings
{

	fCurrentTestMethod = "TextAreaRendererTest-Test2Opt";
	this->TestField2Opt();
	assertEqual( _QUOTE_(<TEXTAREA NAME="fld_the name of field" 0="the option nr. 0 of field" 1="the option nr. 1 of field"></TEXTAREA>), fReply.str());

}

void TextAreaRendererTest::Option1TextArea()
// test the date renderer with simple formatting strings
{

	fCurrentTestMethod = "TextAreaRendererTest-Test1Opt";
	this->TestField1Opt();
	assertEqual( _QUOTE_(<TEXTAREA NAME="fld_the name of field" 0="the option nr. 0 of field"></TEXTAREA>), fReply.str());

}

void TextAreaRendererTest::OptionsNoSlotName2TextArea()
// test the date renderer with simple formatting strings
{

	fCurrentTestMethod = "TextAreaRendererTest-Test2OptNoSlotName";
	this->TestField2OptNoSlotName();
	assertEqual( _QUOTE_(<TEXTAREA NAME="fld_the name of field" the option nr. 0 of field the option nr. 1 of field></TEXTAREA>), fReply.str());

}

void TextAreaRendererTest::OptionNoSlotName1TextArea()
// test the date renderer with simple formatting strings
{

	fCurrentTestMethod = "TextAreaRendererTest-Test1OptNoSlotName";
	this->TestField1OptNoSlotName();
	assertEqual( _QUOTE_(<TEXTAREA NAME="fld_the name of field" the option nr. 0 of field></TEXTAREA>), fReply.str());

}

void TextAreaRendererTest::OptionsMixedTextArea()
// test the date renderer with simple formatting strings
{

	fCurrentTestMethod = "TextAreaRendererTest-Test3OptMixed";
	this->TestField3OptMixed();
	assertEqual( _QUOTE_(<TEXTAREA NAME="fld_the name of field" the option nr. 0 of field 1="the option nr. 1 of field" the option nr. 2 of field></TEXTAREA>), fReply.str());

}

void TextAreaRendererTest::NoOptionsTextArea()
// test the date renderer with simple formatting strings
{

	fCurrentTestMethod = "TextAreaRendererTest-TestOptEmpty";
	this->TestFieldOptEmpty();
	assertEqual( _QUOTE_(<TEXTAREA NAME="fld_the name of field"></TEXTAREA>), fReply.str());

}

void TextAreaRendererTest::TextTextArea()
// test the date renderer with simple formatting strings
{

	fCurrentTestMethod = "TextAreaRendererTest-Text";
	this->TestFieldValue();
	assertEqual( _QUOTE_(<TEXTAREA NAME="fld_the name of field">the value of field</TEXTAREA>), fReply.str());

}

/*===============================================================*/
/*     Check the indirection                                     */
/*===============================================================*/
void TextAreaRendererTest::TestOptionRendererOld()
{
	fCurrentTestMethod = "TextAreaRendererTest-TestOptionRendererOld";
	this->TestFieldOptionRendererOld();
	assertCharPtrEqual( _QUOTE_(<TEXTAREA NAME="fld_the name of field" COLS=30 ROWS=20 WRAP=10 0="the option nr. 0 of field" 1="07.08.97" 2="the option nr. 2 of field">the value of field</TEXTAREA>), fReply.str() );

}

void TextAreaRendererTest::TestFaultOptionRendererOld()
{
	fCurrentTestMethod = "TextAreaRendererTest-TestFaultOptionRendererOld";
	this->TestFieldFaultOptionRendererOld();
	assertCharPtrEqual( _QUOTE_(<TEXTAREA NAME="fld_the name of field" COLS=30 ROWS=20 WRAP=10 0="the option nr. 0 of field" 1="" 2="the option nr. 2 of field">the value of field</TEXTAREA>), fReply.str() );
}

void TextAreaRendererTest::TestOptionRenderer()
{
	fCurrentTestMethod = "TextAreaRendererTest-TestOptionRenderer";
	this->TestFieldOptionRenderer();
	assertCharPtrEqual( _QUOTE_(<TEXTAREA NAME="fld_the name of field" COLS=30 ROWS=20 WRAP=10 0="the option nr. 0 of field" 1="07.08.97" 2="the option nr. 2 of field">the value of field</TEXTAREA>), fReply.str() );
}

void TextAreaRendererTest::TestFaultOptionRenderer()
{
	fCurrentTestMethod = "TextAreaRendererTest-TestFaultOptionRenderer";
	this->TestFieldFaultOptionRenderer();
	assertCharPtrEqual( _QUOTE_(<TEXTAREA NAME="fld_the name of field" COLS=30 ROWS=20 WRAP=10 0="the option nr. 0 of field" 1="%d.%m.%y870912000" 2="the option nr. 2 of field">the value of field</TEXTAREA>), fReply.str() );
}
