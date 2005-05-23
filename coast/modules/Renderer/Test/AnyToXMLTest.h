/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _AnyToXMLTest_h_
#define _AnyToXMLTest_h_

#include "TestCase.h"
#include "AnyToXMLRenderer.h"

class Anything;
class String;

//---- AnyToXMLTest -----------------------------------------------------------
class AnyToXMLTest : public TestCase
{
public:
	AnyToXMLTest(TString tstrName);
	virtual ~AnyToXMLTest();

	void setUp ();
	static Test *suite ();

	// Produce an XML out of an Anything
	void AnythingToXMLTest();

	// Production of XML fails in theses cases
	void WrongInputTests();

protected:
	// Call RenderAll on fAnyToXMLRenderer with fConfig[RendererConfig] and compares the result
	// with fConfig[ExpectedOutput].AsString();
	void DoTest(String RendererConfig, String ExpectedOutput);

	Anything fConfig;
	AnyToXMLRenderer fAnyToXMLRenderer;
};

#endif
