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

//--- test modules used --------------------------------------------------------
#include "TestSuite.h"

//--- module under test --------------------------------------------------------
#include "FormRenderer.h"

//--- interface include --------------------------------------------------------
#include "FileBrowseRendererTest.h"

FileBrowseRendererTest::FileBrowseRendererTest(TString tname) : TextFieldRendererTest(tname)
{

	if (fFieldRenderer) {
		delete fFieldRenderer;    // base class already initialized this !
	}
	fFieldRenderer = new FileBrowseRenderer("FileBrowseRenderer");
}

FileBrowseRendererTest::~FileBrowseRendererTest()
{
	if (fFieldRenderer) {
		delete fFieldRenderer;
	}
	fFieldRenderer = 0;
};

void FileBrowseRendererTest::setUp ()
// what: Setup the variables for all test cases in this class
{
	// TestCase::setUp();
};

Test *FileBrowseRendererTest::suite ()
/* what: return the whole suite of tests for FileBrowseRendererTest, add all top level
		 test functions here.
*/
{
	TestSuite *testSuite = new TestSuite;
	ADD_CASE(testSuite, FileBrowseRendererTest, TestCase0);

	return testSuite;
} // suite

void FileBrowseRendererTest::TestCase0()
{
	fCurrentTestMethod = "TextField-TestCase0";
	this->TestField0();
	assertCharPtrEqual( _QUOTE_(<INPUT TYPE="FILE" NAME="fld_the name of field" VALUE="the value of field" SIZE="50" MAXLENGTH="40" the options of field>), fReply.str() );

}
