/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _SwitchRendererTest_h_
#define _SwitchRendererTest_h_

//-*-Mode: C++;-*-

#include "RendererTest.h"
#include "ConditionalRenderers.h"

//---- SwitchRendererTest -----------------------------------------------------------

class SwitchRendererTest : public RendererTest
{
public:
	SwitchRendererTest(TString name);
	virtual ~SwitchRendererTest();

	static Test *suite ();
	void setUp ();
protected:
	void ConfigureCases( const char *cases );
	void TestCaseA();
	void TestCaseB();
	void TestCaseC();
	void TestCaseNotFoundWithDefault();
	void TestCaseNotFoundWithoutDefault();

	void TestCaseEmptyKeyWithDefault();
	void TestCaseEmptyKeyWithIsEmpty();

	void TestCaseAWithoutCases();
	void TestCaseBWithoutCases();
	void TestCaseCWithoutCases();

	void TestCaseAWithoutContextLookupName();
	void TestCaseBWithoutContextLookupName();
	void TestCaseCWithoutContextLookupName();
	void TestCaseNotFoundWithoutContextLookupName();
	void TestCaseLookupEmpty();

	SwitchRenderer fSwitchRenderer;
};

#endif
