/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

//--- c-library modules used ---------------------------------------------------

//--- standard modules used ----------------------------------------------------
#include "Anything.h"
#include "StringStream.h"
#include "Context.h"
#include "Page.h"

//--- test modules used --------------------------------------------------------
#include "TestSuite.h"

//--- module under test --------------------------------------------------------
#include "PageRelatedLookupRenderer.h"

//--- interface include --------------------------------------------------------
#include "PageRelatedRendererTest.h"

//---- PageRelatedRendererTest ----------------------------------------------------------------
PageRelatedRendererTest::PageRelatedRendererTest(TString tname) : TestCaseType(tname)
{
}

PageRelatedRendererTest::~PageRelatedRendererTest()
{
}

void PageRelatedRendererTest::trivialTest()
{
	// setup Context 1
	Anything q1;
	q1["Test"] = "ContextValueP2";
	q1["Page1"]["Test"] = "PagedContextValueP2";
	q1["query"]["Page1"]["Test"] = "hurray";
	Page p1("Page1");
	Context c1(q1);
	c1.SetPage(&p1);

	PageRelatedLookupRenderer r("PageRelatedLookupRenderer");

	Anything config;
	config["LookupName"] = "Test";

	// use context 1 for rendering
	String s;
	{
		OStringStream reply(s);
		r.RenderAll(reply, c1, ROAnything(config));
	}
	assertEqual("hurray", s);

	// setup Context 2
	Anything q2;
	q2["Test"] = "ContextValueP2";
	Page p2("Page2");
	Context c2(q2);
	c2.SetPage(&p2);

	// use context 2 for rendering: within same
	// thread/without explicit re-initialization
	// the cached values are used rather than the
	// current config
	String s2;
	{
		OStringStream reply(s2);
		r.RenderAll(reply, c2, ROAnything(config));
	}
	assertEqual("ContextValueP2", s2);

	q2["Page2"]["Test"] = "PagedContextValueP2";
	// and re-run 2nd request
	String s3;
	{
		OStringStream reply(s3);
		r.RenderAll(reply, c2, ROAnything(config));
	}
	assertEqual("PagedContextValueP2", s3);

	q2["query"]["Page2"]["Test"] = "hurray";
	// and re-run 2nd request
	String s4;
	{
		OStringStream reply(s4);
		r.RenderAll(reply, c2, ROAnything(config));
	}
	assertEqual("hurray", s4);
}

Test *PageRelatedRendererTest::suite()
{
	TestSuite *testSuite = new TestSuite;
	ADD_CASE(testSuite, PageRelatedRendererTest, trivialTest);
	return testSuite;
}
