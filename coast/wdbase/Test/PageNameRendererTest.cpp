/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

//--- interface include --------------------------------------------------------
#include "PageNameRendererTest.h"

//--- test modules used --------------------------------------------------------
#include "TestSuite.h"
#include "Page.h"

//--- module under test --------------------------------------------------------
#include "PageNameRenderer.h"

//--- standard modules used ----------------------------------------------------
#include "Dbg.h"

//---- PageNameRendererTest ----------------------------------------------------------------
PageNameRendererTest::PageNameRendererTest(TString tname)
	: TestCaseType(tname)
{
	StartTrace(PageNameRendererTest.Ctor);
}

PageNameRendererTest::~PageNameRendererTest()
{
	StartTrace(PageNameRendererTest.Dtor);
}

void PageNameRendererTest::RenderTest()
{
	StartTrace(PageNameRendererTest.RenderTest);

	Context ctx;

	// First without a page
	PageNameRenderer pnr("TestMe");
	ROAnything config;
	{
		String result;
		OStringStream os(result);
		pnr.RenderAll(os, ctx, config);
		assertEqual("", result);
	}

	Page p("TestPage for PageNameRendererTest");
	// now with a page set
	ctx.SetPage(&p);
	{
		String result;
		OStringStream os(result);
		pnr.RenderAll(os, ctx, config);
		assertEqual("TestPage for PageNameRendererTest", result);
	}
}

// builds up a suite of testcases, add a line for each testmethod
Test *PageNameRendererTest::suite ()
{
	StartTrace(PageNameRendererTest.suite);
	TestSuite *testSuite = new TestSuite;

	ADD_CASE(testSuite, PageNameRendererTest, RenderTest);

	return testSuite;

}
