/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

//--- standard modules used ----------------------------------------------------
#include "Anything.h"
#include "StringStream.h"
#include "Context.h"
#include "Renderer.h"

//--- test modules used --------------------------------------------------------
#include "TestSuite.h"

//--- module under test --------------------------------------------------------
#include "URLRenderers.h"

//--- interface include --------------------------------------------------------
#include "URLRendererTest.h"

URLRendererTest::URLRendererTest (TString tname) : RendererTest(tname) {};
URLRendererTest::~URLRendererTest() {};

/*===============================================================*/
/*     Init                                                      */
/*===============================================================*/
void URLRendererTest::setUp ()
// setup config for all the renderers in this TestCase
{
	RendererTest::setUp();

	fConfig["Action"] = "StandardAction";
	fConfig["Params"]["p0"] = "Par0";
	fConfig["Params"]["p1"] = "Par1";
	fConfig["Params"]["p2"] = "Par2";
	fConfig["Params"]["p3"] = "Par3";

} // setUp

Test *URLRendererTest::suite ()
// collect all test cases for the URLRenderer
{
	TestSuite *testSuite = new TestSuite;

	ADD_CASE(testSuite, URLRendererTest, Standard);
	ADD_CASE(testSuite, URLRendererTest, BaseURL);
	ADD_CASE(testSuite, URLRendererTest, BaseR3SSL);
	ADD_CASE(testSuite, URLRendererTest, IntraPage);

	return testSuite;

} // suite

/*===============================================================*/
/*     Check found cases where all is correctly defined          */
/*===============================================================*/

void URLRendererTest::Standard()
{
	URLRenderer urlRenderer("TestURLRenderer");

	// render the configuration
	ROAnything roConfig = fConfig;
	urlRenderer.RenderAll(fReply, fContext, roConfig);

	// assert the result

	assertCharPtrEqual(_QUOTE_(wdgateway?X=Encoder-Scrambler-Signer-Compressor-{/action "StandardAction"/p0 "Par0"/p1 "Par1"/p2 "Par2"/p3 "Par3"}), fReply.str());
}

void URLRendererTest::BaseURL()
{
	URLRenderer urlRenderer("TestURLRenderer");

	fContext.GetTmpStore()["UseBaseURL"] = 1;

	// render the configuration
	ROAnything roConfig = fConfig;
	urlRenderer.RenderAll(fReply, fContext, roConfig);

	// assert the result
	assertCharPtrEqual(_QUOTE_(X2=Encoder-Scrambler-Signer-Compressor-{/action "StandardAction"/p0 "Par0"/p1 "Par1"/p2 "Par2"/p3 "Par3"}), fReply.str());
}

void URLRendererTest::BaseR3SSL()
{
	BaseURLRenderer baseUrlRenderer("TestBaseURLRenderer");
	Anything env, dummy;

	env["HTTP_USER_AGENT"] = "R3SSLV2.0";		// need this in env
	Context context(env, dummy, 0, 0, 0, 0);
	// therefore I generate my own context

	Anything tmpStore = context.GetTmpStore();
	tmpStore["UseBaseURL"] = 1;
	tmpStore["R3BaseAddress"] = "test.r3.base.address/";

	Anything config = fConfig.DeepClone();

	config["BaseAddr"]["ContextLookupRenderer"] = "R3BaseAddress";
	baseUrlRenderer.RenderAll(fReply, context, config);

	assertEqual(_QUOTE_(<base href="test.r3.base.address/wdgateway/X1=Encoder-Scrambler-Signer-Compressor-*/"/>\n), fReply.str());
} // R3SSL

void URLRendererTest::IntraPage()
{
	Anything env, dummy;

	env["HTTP_USER_AGENT"] = "R3SSLV2.0";		// obsoleted
	Context context(env, dummy, 0, 0, 0, 0);
	// therefore I generate my own context

	Anything tmpStore = context.GetTmpStore();
	tmpStore["UseBaseURL"] = 1;
	tmpStore["BaseAddress"] = "test.r3.base.address/";

	String result = URLRenderer::CreateIntraPageLink(context, "TestIntra");
	assertEqual("test.r3.base.address//wdgateway#TestIntra", result);
	// well, actually I doubt that this is still of any use, but that it is

} // IntraPage
