/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _HTTPMimeHeaderMapperTest_H
#define _HTTPMimeHeaderMapperTest_H

//---- baseclass include -------------------------------------------------
#include "ConfiguredTestCase.h"

//---- HTTPMimeHeaderMapperTest ----------------------------------------------------------
//! <B>single line description of the class</B>
/*!
further explanation of the purpose of the class
this may contain <B>HTML-Tags</B>
*/
class HTTPMimeHeaderMapperTest : public ConfiguredTestCase
{
public:
	//--- constructors

	//! ConfiguredTestCase constructor
	//! \param name name of the test
	HTTPMimeHeaderMapperTest(TString tstrName);

	//! destroys the test case
	~HTTPMimeHeaderMapperTest();

	//--- public api

	//! builds up a suite of ConfiguredTestCases for this test
	static Test *suite ();

	//! sets the environment for this test
	void setUp ();

	//! deletes the environment for this test
	void tearDown ();

	//! take a simple http response and parse it
	void SimpleHeader();
	//! check the fixing of wrongly parsed Date fields regarding RFC 2068
	void CorrectedDateHeadersTest();
	//! test suppress config of HTTPMimeHeaderMapper
	void SuppressedHeadersTest();
	//! test suppress with literal config of HTTPMimeHeaderMapper
	void LiteralSuppressedHeadersTest();
	//! test add with config of HTTPMimeHeaderMapper
	void AddHeadersTest();
	//! test suppress and add with config of HTTPMimeHeaderMapper
	void SuppressAndAddHeadersTest();
	//! test fields which occur more then once
	void FieldsOccursMoreThanOnceHeaderTest();
	//! show effect of DoNotSplitHeaderFields set off (do split header fields, default)
	void FieldsOccursMoreThanOnceHeaderTestDoSplit();
};

#endif
