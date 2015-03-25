/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#include "CallRendererTest.h"
#include "CallRenderer.h"
#include "TestSuite.h"

void CallRendererTest::EmptyCallTest() {
	StartTrace(CallRendererTest.EmptyCallTest);
	Anything myConfig;
	myConfig["CallRenderer"] = Anything();
	Renderer::Render(fReply, fContext, myConfig);
	assertCharPtrEqual("", fReply.str());
}
void CallRendererTest::LookupCallTest() {
	StartTrace(CallRendererTest.LookupCallTest);
	Anything myConfig;
	myConfig["CallRenderer"]["Renderer"] = "MyLookup";
	myConfig["CallRenderer"]["Parameters"]["myparam"] = "Peter was here";
	myConfig["CallRenderer"]["Parameters"]["myparam2"] = "Peter was here too";
	Anything mylookup = Anything(Anything::ArrayMarker());
	mylookup.Append("A Test ");
	Anything spec = Anything(Anything::ArrayMarker());
	spec.Append("myparam");
	mylookup["ContextLookupRenderer"] = spec;
	TraceAny(mylookup, "mylookup");
	fContext.GetTmpStore()["MyLookup"] = mylookup;
	fContext.GetTmpStore()["myparam"] = "Peter got lost on the way";
	Renderer::Render(fReply, fContext, myConfig);
	assertCharPtrEqual("A Test Peter was here", fReply.str());
	ROAnything dummy;
	t_assert(!fContext.Lookup("myparam2", dummy)); // params are popped again
}

// builds up a suite of testcases, add a line for each testmethod
Test *CallRendererTest::suite() {
	StartTrace(CallRendererTest.suite);
	TestSuite *testSuite = new TestSuite;
	ADD_CASE(testSuite, CallRendererTest, EmptyCallTest);
	ADD_CASE(testSuite, CallRendererTest, LookupCallTest);
	return testSuite;
}
