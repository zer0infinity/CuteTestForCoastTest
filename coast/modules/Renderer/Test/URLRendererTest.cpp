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
{
	RendererTest::setUp();

	fConfig["Action"] = "StandardAction";
	fConfig["Params"]["p0"] = "Par0";
	fConfig["Params"]["p1"] = "Par1";
	fConfig["Params"]["p2"] = "Par2";
	fConfig["Params"]["p3"] = "Par3";

}

Test *URLRendererTest::suite ()
{
	TestSuite *testSuite = new TestSuite;

	ADD_CASE(testSuite, URLRendererTest, Standard);
	ADD_CASE(testSuite, URLRendererTest, BaseURL);
	ADD_CASE(testSuite, URLRendererTest, BaseR3SSL);
	ADD_CASE(testSuite, URLRendererTest, IntraPage);

	return testSuite;

}

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

	assertCharPtrEqual("wdgateway?X=Encoder-Scrambler-Signer-Compressor-%7B%2Faction%20%22StandardAction%22%2Fp0%20%22Par0%22%2Fp1%20%22Par1%22%2Fp2%20%22Par2%22%2Fp3%20%22Par3%22%7D", fReply.str());
}

void URLRendererTest::BaseURL()
{
	URLRenderer urlRenderer("TestURLRenderer");

	fContext.GetTmpStore()["UseBaseURL"] = 1;

	// render the configuration
	ROAnything roConfig = fConfig;
	urlRenderer.RenderAll(fReply, fContext, roConfig);

	// assert the result
	assertCharPtrEqual("X2=Encoder-Scrambler-Signer-Compressor-%7B%2Faction%20%22StandardAction%22%2Fp0%20%22Par0%22%2Fp1%20%22Par1%22%2Fp2%20%22Par2%22%2Fp3%20%22Par3%22%7D", fReply.str());
}

void URLRendererTest::BaseR3SSL()
{
	BaseURLRenderer baseUrlRenderer("TestBaseURLRenderer");
	Anything env, dummy;

	env["header"]["USER-AGENT"] = "R3SSLV2.0";		// need this in env
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

	env["header"]["USER-AGENT"] = "R3SSLV2.0";		// obsoleted
	Context context(env, dummy, 0, 0, 0, 0);
	// therefore I generate my own context

	Anything tmpStore = context.GetTmpStore();
	tmpStore["UseBaseURL"] = 1;
	tmpStore["BaseAddress"] = "test.r3.base.address/";

	String result = URLRenderer::CreateIntraPageLink(context, "TestIntra");
	assertEqual("test.r3.base.address//wdgateway#TestIntra", result);
	// well, actually I doubt that this is still of any use, but that it is

} // IntraPage
