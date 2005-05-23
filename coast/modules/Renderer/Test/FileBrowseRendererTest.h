/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _FileBrowseRendererTest_h_
#define _FileBrowseRendererTest_h_

#include "RendererTest.h"
#include "TextFieldRendererTest.h"

//---- FileBrowseRendererTest -----------------------------------------------------------

class FileBrowseRendererTest : public TextFieldRendererTest
{
public:
	FileBrowseRendererTest(TString tstrName);
	virtual ~FileBrowseRendererTest();

	static Test *suite ();
	void setUp ();
protected:
	// Generic test cases
	void TestCase0();
};

#endif
