/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

//--- interface include --------------------------------------------------------
#include "FirstNonEmptyRendererTest.h"
//--- test modules used --------------------------------------------------------
#include "TestSuite.h"
//--- module under test --------------------------------------------------------
#include "FirstNonEmptyRenderer.h"
//--- standard modules used ----------------------------------------------------
#include "Dbg.h"

//--- project modules used -----------------------------------------------------

//--- c-modules used -----------------------------------------------------------

//---- FirstNonEmptyRendererTest ----------------------------------------------------------------
FirstNonEmptyRendererTest::FirstNonEmptyRendererTest(TString tstrName) : RendererTest(tstrName)
{
	StartTrace(FirstNonEmptyRendererTest.Ctor);
}

FirstNonEmptyRendererTest::~FirstNonEmptyRendererTest()
{
	StartTrace(FirstNonEmptyRendererTest.Dtor);
}

void FirstNonEmptyRendererTest::EmptyList()
{
	StartTrace(FirstNonEmptyRendererTest.EmptyList);
	FirstNonEmptyRenderer r("EmptyList");
	fConfig = Anything();
	r.RenderAll(fReply, fContext, fConfig);
	assertEqual("", fReply.str());
}
void FirstNonEmptyRendererTest::OneElement()
{
	StartTrace(FirstNonEmptyRendererTest.OneElement);
	FirstNonEmptyRenderer r("OneElement");
	fConfig = Anything();
	fConfig.Append(Anything("output"));
	r.RenderAll(fReply, fContext, fConfig);
	assertEqual("output", fReply.str());
}
void FirstNonEmptyRendererTest::MultipleElements()
{
	StartTrace(FirstNonEmptyRendererTest.MultipleElements);
	FirstNonEmptyRenderer r("MultipleElements");
	fConfig = Anything();
	fConfig.Append(Anything(""));
	Anything lookup;
	lookup["ContextLookupRenderer"]["LookupName"] = "notfound";
	fConfig.Append(lookup.DeepClone());
	lookup["ContextLookupRenderer"]["LookupName"] = "iamhere";
	fConfig.Append(lookup);
	fConfig.Append(Anything("some text"));
	fContext.GetTmpStore()["iamhere"] = "Peter";
	r.RenderAll(fReply, fContext, fConfig);
	assertEqual("Peter", fReply.str());
}

// builds up a suite of testcases, add a line for each testmethod
Test *FirstNonEmptyRendererTest::suite ()
{
	StartTrace(FirstNonEmptyRendererTest.suite);
	TestSuite *testSuite = new TestSuite;

	ADD_CASE(testSuite, FirstNonEmptyRendererTest, EmptyList);
	ADD_CASE(testSuite, FirstNonEmptyRendererTest, OneElement);
	ADD_CASE(testSuite, FirstNonEmptyRendererTest, MultipleElements);

	return testSuite;
}
