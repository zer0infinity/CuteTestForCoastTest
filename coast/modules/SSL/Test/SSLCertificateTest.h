/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _SSLCertificateTest_H
#define _SSLCertificateTest_H

//---- baseclass include -------------------------------------------------
#include "ConfiguredTestCase.h"

//---- SSLCertificateTest ----------------------------------------------------------
//! <B>single line description of the class</B>
/*!
further explanation of the purpose of the class
this may contain <B>HTML-Tags</B>
*/
class SSLCertificateTest : public ConfiguredTestCase
{
public:
	//--- constructors

	//! ConfiguredTestCase constructor
	//! \param name name of the test
	SSLCertificateTest(TString name);

	//! destroys the test case
	~SSLCertificateTest();

	//--- public api

	//! builds up a suite of ConfiguredTestCases for this test
	static Test *suite ();

	//! sets the environment for this test
	void setUp ();

	//! deletes the environment for this test
	void tearDown ();

	//! describe this Test
	void test();
	void clientCertificateTest();
	void checkServerCertificateTest();
	void convertDNTest();
};

#endif
