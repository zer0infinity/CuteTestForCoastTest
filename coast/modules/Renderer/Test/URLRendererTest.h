/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _URLRendererTest_h_
#define _URLRendererTest_h_

//-*-Mode: C++;-*-

#include "RendererTest.h"

//---- URLRendererTest -----------------------------------------------------------

class URLRendererTest : public RendererTest
{
public:
	URLRendererTest(TString name);
	virtual ~URLRendererTest();

	static Test *suite ();
	void setUp ();
protected:
	void Standard();
	void BaseURL();
	void BaseR3SSL();
	void IntraPage();
};

#endif
