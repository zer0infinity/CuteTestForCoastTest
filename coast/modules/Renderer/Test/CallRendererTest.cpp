/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#include "CallRendererTest.h"
#include "Dbg.h"
#include "TestSuite.h"
#include "CallRenderer.h"

//---- CallRendererTest ----------------------------------------------------------------
CallRendererTest::CallRendererTest(TString tstrName) : RendererTest(tstrName)
{
	StartTrace(CallRendererTest.Ctor);
}

CallRendererTest::~CallRendererTest()
{
	StartTrace(CallRendererTest.Dtor);
}

void CallRendererTest::EmptyCallTest()
{
	StartTrace(CallRendererTest.EmptyCallTest);
	CallRenderer cr("EmptyCall");
	fConfig = Anything();
	cr.RenderAll(fReply, fContext, fConfig);
	assertCharPtrEqual("", fReply.str());
}
void CallRendererTest::LookupCallTest()
{
	StartTrace(CallRendererTest.LookupCallTest);
	CallRenderer cr("LookupCall");
	fConfig = Anything();
	fConfig["Renderer"] = "MyLookup";
	fConfig["Parameters"]["myparam"] = "Peter was here";
	fConfig["Parameters"]["myparam2"] = "Peter was here too";
	Anything mylookup = Anything(Anything::ArrayMarker());
	mylookup.Append("A Test ");
	Anything spec = Anything(Anything::ArrayMarker());
	spec.Append("myparam");
	mylookup["ContextLookupRenderer"] = spec;
	TraceAny(mylookup, "mylookup");
	fContext.GetTmpStore()["MyLookup"] = mylookup;
	fContext.GetTmpStore()["myparam"] = "Peter got lost on the way";
	cr.RenderAll(fReply, fContext, fConfig);
	assertCharPtrEqual("A Test Peter was here", fReply.str());
	ROAnything dummy;
	t_assert(!fContext.Lookup("myparam2", dummy)); // params are popped again

}

// builds up a suite of testcases, add a line for each testmethod
Test *CallRendererTest::suite ()
{
	StartTrace(CallRendererTest.suite);
	TestSuite *testSuite = new TestSuite;

	ADD_CASE(testSuite, CallRendererTest, EmptyCallTest);
	ADD_CASE(testSuite, CallRendererTest, LookupCallTest);

	return testSuite;
}
