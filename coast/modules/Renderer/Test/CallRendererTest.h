/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _CallRendererTest_H
#define _CallRendererTest_H

#include "Anything.h"
#include "StringStream.h"
#include "System.h"
#include "Context.h"

//---- baseclass include -------------------------------------------------
#include "RendererTest.h"

//---- CallRendererTest ----------------------------------------------------------
//! <B>test the new Call (aka Lambda) Renderer</B>
/*!
Testcase to create test-first the proposed CallRenderer.
This renderer should allow the reuse of complex renderer specifications
by passing parameters to the renderer via the context, thus local
throughout a page
this may contain <B>HTML-Tags</B>
*/
class CallRendererTest : public RendererTest
{
public:
	//--- constructors

	//! TestCase constructor
	//! \param name name of the test
	CallRendererTest(TString name);

	//! destroys the test case
	~CallRendererTest();

	//--- public api

	//! builds up a suite of testcases for this test
	static Test *suite ();

	//! sets the environment for this test
	void setUp ();

	//! deletes the environment for this test
	void tearDown ();

	//! Call a CallRenderer with an empty config
	void EmptyCallTest();
	//! Call a CallRenderer with an simple lookup config
	void LookupCallTest();
};

#endif
