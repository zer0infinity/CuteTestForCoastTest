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
#include "ImageButtonRendererTest.h"

ImageButtonRendererTest::ImageButtonRendererTest (TString tname) : FieldRendererTest(tname)
{
	fFieldRenderer = new ImageButtonRenderer("ImageButtonRenderer");
};

ImageButtonRendererTest::~ImageButtonRendererTest()
{
	if (fFieldRenderer) {
		delete fFieldRenderer;
	}
	fFieldRenderer = 0;
};

/*===============================================================*/
/*     Init                                                      */
/*===============================================================*/
void ImageButtonRendererTest::setUp ()
// setup config for all the renderers in this TestCase
{
	FieldRendererTest::setUp();
} // setUp

Test *ImageButtonRendererTest::suite ()
// collect all test cases for the ImageButtonRenderer
{
	TestSuite *testSuite = new TestSuite;

	testSuite->addTest (NEW_CASE(ImageButtonRendererTest, TestCaseEmptyConf));
	testSuite->addTest (NEW_CASE(ImageButtonRendererTest, TestCase0));
	testSuite->addTest (NEW_CASE(ImageButtonRendererTest, TestCase1));
	testSuite->addTest (NEW_CASE(ImageButtonRendererTest, TestCase2));
	testSuite->addTest (NEW_CASE(ImageButtonRendererTest, TestCase3));
	testSuite->addTest (NEW_CASE(ImageButtonRendererTest, TestCaseWithoutName));
	testSuite->addTest (NEW_CASE(ImageButtonRendererTest, TestCaseWithoutLabel));
	testSuite->addTest (NEW_CASE(ImageButtonRendererTest, TestCaseWithoutValue));
	testSuite->addTest (NEW_CASE(ImageButtonRendererTest, TestCaseWithoutSource));
	testSuite->addTest (NEW_CASE(ImageButtonRendererTest, TestCaseWithoutMultiple));
	testSuite->addTest (NEW_CASE(ImageButtonRendererTest, TestCaseWithoutChecked));
	testSuite->addTest (NEW_CASE(ImageButtonRendererTest, TestCaseWithoutOptions));
	testSuite->addTest (NEW_CASE(ImageButtonRendererTest, TestCaseWrong ));
	testSuite->addTest (NEW_CASE(ImageButtonRendererTest, ConfigurationWithRenderer ));
	testSuite->addTest (NEW_CASE(ImageButtonRendererTest, TestOptionRenderer));
	testSuite->addTest (NEW_CASE(ImageButtonRendererTest, TestFaultOptionRenderer));
	testSuite->addTest (NEW_CASE(ImageButtonRendererTest, TestOptionRendererOld ));
	testSuite->addTest (NEW_CASE(ImageButtonRendererTest, TestFaultOptionRendererOld ));

	return testSuite;

} // suite

/*===============================================================*/
/*     Check where all is correctly defined                      */
/*===============================================================*/
void ImageButtonRendererTest::TestCase0()
{
	fCurrentTestMethod = "ImageButton-TestCase0";
	this->TestField0();
	assertCharPtrEqual( _QUOTE_(<INPUT TYPE="IMAGE" NAME="i_the name of field" SRC="the source of field" the options of field>), fReply.str() );
}

void ImageButtonRendererTest::TestCase1()
{
	fCurrentTestMethod = "ImageButton-TestCase1";
	this->TestField1();
	assertCharPtrEqual( _QUOTE_(<INPUT TYPE="IMAGE" NAME="i_the name of field" SRC="the source of field" 0="the option nr. 0 of field" 1="the option nr. 1 of field" 2="the option nr. 2 of field">), fReply.str() );
}

void ImageButtonRendererTest::TestCase2()
{
	fCurrentTestMethod = "ImageButton-TestCase2";
	this->TestField2();
	assertCharPtrEqual( _QUOTE_(<INPUT TYPE="IMAGE" NAME="i_the name of field" SRC="the source of field" 0="the option nr. 0 of field" the option nr. 1 of field 2="the option nr. 2 of field">), fReply.str() );
}

void ImageButtonRendererTest::TestCase3()
{
	fCurrentTestMethod = "ImageButton-TestCase3";
	this->TestField3();
	assertCharPtrEqual( _QUOTE_(<INPUT TYPE="IMAGE" NAME="i_the name of field" SRC="the source of field" 0="the option nr. 0 of field" the option nr. 1 of field 2="the option nr. 2 of field">), fReply.str() );
}

/*===============================================================*/
/*     Check where not all is correctly defined                  */
/*===============================================================*/
void ImageButtonRendererTest::TestCaseEmptyConf()
{
	fCurrentTestMethod = "ImageButton-TestCaseEmptyConf";
	this->TestEmptyConf();
	assertCharPtrEqual( "", fReply.str() );
}

void ImageButtonRendererTest::TestCaseWithoutName()
{
	fCurrentTestMethod = "ImageButton-TestCaseWithoutName";
	this->TestFieldWithoutName();
	assertCharPtrEqual( "", fReply.str() );
}

void ImageButtonRendererTest::TestCaseWithoutLabel()
{
	fCurrentTestMethod = "ImageButton-TestCaseWithoutLabel";
	this->TestFieldWithoutLabel();
	assertCharPtrEqual( _QUOTE_(<INPUT TYPE="IMAGE" NAME="i_the name of field" SRC="the source of field" the options of field>), fReply.str() );
}

void ImageButtonRendererTest::TestCaseWithoutValue()
{
	fCurrentTestMethod = "ImageButton-TestFieldWithoutValue";
	this->TestFieldWithoutValue();
	assertCharPtrEqual( _QUOTE_(<INPUT TYPE="IMAGE" NAME="i_the name of field" SRC="the source of field" the options of field>), fReply.str() );
}

void ImageButtonRendererTest::TestCaseWithoutSource()
{
	fCurrentTestMethod = "ImageButton-TestFieldWithoutSource";
	this->TestFieldWithoutSource();
	assertCharPtrEqual( _QUOTE_(<INPUT TYPE="IMAGE" NAME="i_the name of field" SRC="the source of field" the options of field>), fReply.str() );
}

void ImageButtonRendererTest::TestCaseWithoutMultiple()
{
	fCurrentTestMethod = "ImageButton-TestFieldWithoutMultiple";
	this->TestFieldWithoutMultiple();
	assertCharPtrEqual( _QUOTE_(<INPUT TYPE="IMAGE" NAME="i_the name of field" SRC="the source of field" the options of field>), fReply.str() );
}

void ImageButtonRendererTest::TestCaseWithoutChecked()
{
	fCurrentTestMethod = "ImageButton-TestFieldWithoutChecked";
	this->TestFieldWithoutChecked();
	assertCharPtrEqual( _QUOTE_(<INPUT TYPE="IMAGE" NAME="i_the name of field" SRC="the source of field" the options of field>), fReply.str() );
}

void ImageButtonRendererTest::TestCaseWithoutOptions()
{
	fCurrentTestMethod = "ImageButton-TestFieldWithoutOptions";
	this->TestFieldWithoutOptions();
	assertCharPtrEqual( _QUOTE_(<INPUT TYPE="IMAGE" NAME="i_the name of field" SRC="the source of field">), fReply.str() );
}

/*===============================================================*/
/*     Check where there is a fault fConfig                      */
/*===============================================================*/
void ImageButtonRendererTest::TestCaseWrong()
{
	fCurrentTestMethod = "ImageButton-TestCaseWrong";
	this->TestFieldWithoutName();
	assertCharPtrEqual( "", fReply.str() );
}

/*===============================================================*/
/*     Check the indirection                                     */
/*===============================================================*/
void ImageButtonRendererTest::TestOptionRendererOld()
{
	fCurrentTestMethod = "ImageButtonRendererTest-TestOptionRendererOld";
	this->TestFieldOptionRendererOld();
	assertCharPtrEqual( _QUOTE_(<INPUT TYPE="IMAGE" NAME="i_the name of field" SRC="the source of field" 0="the option nr. 0 of field" 1="07.08.97" 2="the option nr. 2 of field">), fReply.str() );
}

void ImageButtonRendererTest::TestFaultOptionRendererOld()
{
	fCurrentTestMethod = "ImageButtonRendererTest-TestFaultOptionRendererOld";
	this->TestFieldFaultOptionRendererOld();
	assertCharPtrEqual( _QUOTE_(<INPUT TYPE="IMAGE" NAME="i_the name of field" SRC="the source of field" 0="the option nr. 0 of field" 1="" 2="the option nr. 2 of field">), fReply.str() );
}

void ImageButtonRendererTest::TestOptionRenderer()
{
	fCurrentTestMethod = "ImageButtonRendererTest-TestOptionRenderer";
	this->TestFieldOptionRenderer();
	assertCharPtrEqual( _QUOTE_(<INPUT TYPE="IMAGE" NAME="i_the name of field" SRC="the source of field" 0="the option nr. 0 of field" 1="07.08.97" 2="the option nr. 2 of field">), fReply.str() );
}

void ImageButtonRendererTest::TestFaultOptionRenderer()
{
	fCurrentTestMethod = "ImageButtonRendererTest-TestFaultOptionRenderer";
	this->TestFieldFaultOptionRenderer();
	assertCharPtrEqual( _QUOTE_(<INPUT TYPE="IMAGE" NAME="i_the name of field" SRC="the source of field" 0="the option nr. 0 of field" 1="%d.%m.%y870912000" 2="the option nr. 2 of field">), fReply.str() );
}

/*===============================================================*/
/*     Check where there is a fConfig with Renderers             */
/*===============================================================*/
void ImageButtonRendererTest::ConfigurationWithRenderer()
{
	fCurrentTestMethod = "ImageButton-ConfigurationWithRenderer";
	this->TestConfigureFieldWithRenderer();
	assertCharPtrEqual( _QUOTE_(<INPUT TYPE="IMAGE" NAME="i_String rendered Name" SRC="String rendered Source" 0="the option nr. 0 of field" 1="the option nr. 1 of field" 2="String rendered Option">), fReply.str() );
}
