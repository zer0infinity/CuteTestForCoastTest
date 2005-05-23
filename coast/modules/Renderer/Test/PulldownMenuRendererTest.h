/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _PulldownMenuRendererTest_H
#define _PulldownMenuRendererTest_H

//-*-Mode: C++;-*-
#ifndef _PulldownMenuRenderer_h_
#define _PulldownMenuRenderer_h_

#include "RendererTest.h"
#include "FieldRendererTest.h"

//---- PulldownMenuRenderer -----------------------------------------------------------

class PulldownMenuRendererTest : public FieldRendererTest
{
public:
	PulldownMenuRendererTest(TString tstrName);
	virtual ~PulldownMenuRendererTest();

	static Test *suite ();
	void setUp ();

protected:
	void TestCaseEmptyConf();
	void TestCase0();
	void TestCase1();
	void TestCase2();
	void TestCase3();
	void TestCaseWithoutName();
	void TestCaseWithoutLabel();
	void TestCaseWithoutValue();
	void TestCaseWithoutSource();
	void TestCaseWithoutMultiple();
	void TestCaseWithoutChecked();
	void TestCaseWithoutOptions();
	void TestCaseWrong();
	void TestCaseWithList();
	void TestCaseWithLookupList();
	void TestCaseWithListAndLookupList();
	void TestCaseWithListAndLookupListSkipped();
	void TestCaseWithUndefList();
	void TestCaseWithListAndUndefLookupList();
	void TestCaseAllAttributes();
	void TestCaseNameAndMultiple();
	void TestCaseListWithUndefOptions();
	void TestOptionRenderer();
	void TestFaultOptionRenderer();
	void TestOptionRendererOld();
	void TestFaultOptionRendererOld();
};

#endif
#endif		//not defined _PulldownMenuRendererTest_H
