/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _GenericXMLParserTest_H
#define _GenericXMLParserTest_H

//---- baseclass include -------------------------------------------------
#include "TestCase.h"
#include "Anything.h"

//---- GenericXMLParserTest ----------------------------------------------------------
//! <B>really brief class description</B>
/*!
further explanation of the purpose of the class
this may contain <B>HTML-Tags</B>
*/
class GenericXMLParserTest : public TestCase
{
public:
	//--- constructors

	//! TestCase constructor
	//! \param name name of the test
	GenericXMLParserTest(TString name);

	//! destroys the test case
	~GenericXMLParserTest();

	//--- public api

	//! builds up a suite of testcases for this test
	static Test *suite ();

	//! sets the environment for this test
	void setUp ();

	//! deletes the environment for this test
	void tearDown ();

	//! describe this testcase
	void simpleEmptyTag();
	void simpleAttributeTag();
	void simpleBodyTag();
	void simpleAttributeBodyTag();
	void simpleTagWithComment();
	void simpleNestedTags();
	void simpleExampleXML();
	void simpleDTDExampleXML();
	void simpleXMLError();
	void simpleParsePrint();
	void configuredTests();

private:
	Anything fConfig;
};

#endif
