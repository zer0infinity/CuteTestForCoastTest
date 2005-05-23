/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _SybCTThreadTest_H
#define _SybCTThreadTest_H

//---- baseclass include -------------------------------------------------
#include "ConfiguredTestCase.h"

//---- SybCTThreadTest ----------------------------------------------------------
//! <B>single line description of the class</B>
/*!
further explanation of the purpose of the class
this may contain <B>HTML-Tags</B>
*/
class SybCTThreadTest : public ConfiguredTestCase
{
public:
	//--- constructors

	//! ConfiguredTestCase constructor
	//! \param name name of the test
	SybCTThreadTest(TString tstrName);

	//! destroys the test case
	~SybCTThreadTest();

	//--- public api

	//! builds up a suite of ConfiguredTestCases for this test
	static Test *suite ();

	//! sets the environment for this test
	void setUp ();

	//! deletes the environment for this test
	void tearDown ();

	//! describe this Test
	void SybCTDAImplTest();
	void SybCTPoolDAImplTest();
	void SybCTnewDAImplTest();

	void DoTest(const char *goodDAName, const char *failDAName);
	void Run(long id, const char *goodDAName, const char *failDAName);

	bool fbWasInitialized;
};

#endif
