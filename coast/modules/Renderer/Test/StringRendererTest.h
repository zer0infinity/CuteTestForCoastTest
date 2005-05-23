/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _StringRendererTest_h_
#define _StringRendererTest_h_

//-*-Mode: C++;-*-

#include "RendererTest.h"
#include "LocalizedRenderers.h"

//---- StringRendererTest -----------------------------------------------------------

class StringRendererTest : public RendererTest
{
public:
	StringRendererTest(TString tstrName);
	virtual ~StringRendererTest();

	static Test *suite ();
	void setUp ();
protected:
	void langGE();
	void langGEWithoutText();
	void langGEDefaultWithoutText();
	void langGEalone();

	void langFR();
	void langFRWithoutText();
	void langFRDefaultWithoutText();
	void langFRalone();

	void langIT();
	void langITWithoutText();
	void langITDefaultWithoutText();
	void langITalone();

	void langEN();
	void langENWithoutText();
	void langENDefaultWithoutText();
	void langENalone();

	void langDefault();
	void langDefaultWithoutText();
	void langDefaultalone();

	void noText();

	StringRenderer fStringRenderer;
};

#endif
