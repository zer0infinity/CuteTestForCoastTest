/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _DateRendererTest_h_
#define _DateRendererTest_h_

//-*-Mode: C++;-*-

#include "RendererTest.h"

//---- DateRendererTest -----------------------------------------------------------

class DateRendererTest : public RendererTest
{
public:
	DateRendererTest(TString name);
	virtual ~DateRendererTest();

	static Test *suite ();
	void setUp ();
protected:
	void simpleFormat();
	void DifferentFormats1();
	void DifferentFormats2();
	void DifferentFormats3();
	void RelativeTimeFormat();
	void AbsolutTimeFormat();
	void GMTTime();
	void emptyConfig1();
	void emptyConfig2();

private:
	void CompareHelper(const char *format, long offset = 0L);
};

#endif
