/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

//--- test modules used --------------------------------------------------------
#include "TestSuite.h"

//--- module under test --------------------------------------------------------
#include "BasicTableRenderer.h"

//--- interface include --------------------------------------------------------
#include "BasicTableRendererTest.h"

//--- standard modules used ----------------------------------------------------
#include "System.h"

//--- c-library modules used ---------------------------------------------------

BasicTableRendererTest::BasicTableRendererTest (TString tname) : RendererTest(tname) {}

BasicTableRendererTest::~BasicTableRendererTest() {}

/*===============================================================*/
/*     Init                                                      */
/*===============================================================*/

Context *BasicTableRendererTest::CreateContext(const char *demodata)
{
	iostream *is = System::OpenStream(demodata, "any");

	if (is) {
		Anything testData;
		testData.Import(*is);
		delete is;

		// setup Context
		Context *c = new Context(testData["Env"], testData["Query"], 0, 0, 0, 0);

		Anything tmp = c->GetTmpStore();
		Anything newTmp = testData["TmpStore"];
		for (long i = 0; i < newTmp.GetSize(); i++) {
			tmp[newTmp.SlotName(i)] = newTmp[i];
		}

		c->SetLanguage("D");		// set a default language explicitely

		return c;
	} else {
		String msg("open file: ");
		msg << demodata << ".any";
		assertEqual((const char *)msg, "file not found");
		return 0L;
	}
}

void BasicTableRendererTest::setUp ()
// setup config for all the renderers in this TestCase
{
	RendererTest::setUp();
} // setUp

Test *BasicTableRendererTest::suite ()
// collect all test cases for the URLRenderer
{
	TestSuite *testSuite = new TestSuite;

	testSuite->addTest (NEW_CASE(BasicTableRendererTest, CanUseInvertedHeaders));
	testSuite->addTest (NEW_CASE(BasicTableRendererTest, FullFledged));

	return testSuite;

} // suite

/*===============================================================*/
/*     Check found cases where all is correctly defined          */
/*===============================================================*/

void BasicTableRendererTest::CanUseInvertedHeaders()
{
	// not a netscape v2 browser
	BasicTableRenderer r("BasicTableRenderer");
	Context *c = CreateContext("BasicTableRendererTestData");
	if (c) {
		t_assert(r.CanUseInvertedHeaders(*c) ==  1);
		delete c;
	} else {
		t_assert(c != 0);
	}
	// netscape v2 browser
	c = CreateContext("BasicTableRendererTestDataNetscape2");

	if (c) {
		t_assert(r.CanUseInvertedHeaders(*c) ==  0);
		delete c;
	} else {
		t_assert(c != 0);
	}
}

void BasicTableRendererTest::FullFledged()
{
	// compare preserved Renderer output (non Netscape2) with current Renderer
	// output (using same configuration)

	Context *c = CreateContext("BasicTableRendererTestData");

	if (c) {
		OStringStream reply;
		// create Renderer output
		iostream *is1 = System::OpenStream("BasicTableRendererConfig", "any");
		if (is1) {

			Anything config;
			config.Import(*is1);
			delete(is1);

			BasicTableRenderer r("TestURLRenderer");
			reply << "<HTML><HEAD></HEAD><BODY>\n";
			r.RenderAll(reply, *c, ROAnything(config));
			reply << "</BODY></HTML>\n";
		} else {
			t_assert(is1 != 0);
		}
		// to create the referenceTabelOutput File
//	iostream *my = System::OpenOStream("ReferenceTableOutput", "html");
//	(*my) << reply.str();
//	delete( my );

		// get reference output
		iostream *is2 = System::OpenStream("ReferenceTableOutput", "html");
		String ref;
		if (is2) {
			int ch;
			while ((ch = is2->get()) != EOF) {
				ref << (char)ch;
			}

			delete(is2);
		} else {
			t_assert(is2 != 0);
		}
		delete c;

		// poor bastard that needs to debug this :(

		assertEqual(reply.str(), ref);
	} else {
		t_assert(c != 0);
	}
}
