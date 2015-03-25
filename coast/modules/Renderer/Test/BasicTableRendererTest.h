/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _BasicTableRendererTest_h_
#define _BasicTableRendererTest_h_

//-*-Mode: C++;-*-

#include "RendererTest.h"

//---- BasicTableRendererTest -----------------------------------------------------------

class BasicTableRendererTest : public RendererTest
{
public:
	BasicTableRendererTest(TString tstrName);
	virtual ~BasicTableRendererTest();

	static Test *suite ();
	void setUp ();
protected:
	Context *CreateContext(const char *demodata);
	void CanUseInvertedHeaders();
	void FullFledged();
};

#endif
