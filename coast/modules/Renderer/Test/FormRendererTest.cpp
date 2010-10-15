/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

//--- interface include --------------------------------------------------------
#include "FormRendererTest.h"

//--- test modules used --------------------------------------------------------
#include "TestSuite.h"

//--- standard modules used ----------------------------------------------------
#include <iostream>

//--- module under test --------------------------------------------------------

FormRendererTest::FormRendererTest (TString tname) : TestCaseType(tname),
	fEnvironment(Anything::ArrayMarker()), fRole("FRTestRole"),
	fContext( fEnvironment, Anything(), 0, 0, &fRole, 0 ),
	fFormRenderer("FormRenderer")
{
}

void FormRendererTest::setUp ()
{
	fContext.SetLanguage("D");
}

Test *FormRendererTest::suite ()
{
	TestSuite *testSuite = new TestSuite;
	ADD_CASE(testSuite, FormRendererTest, TestCase0);
	return testSuite;
}

FormRendererTest::~FormRendererTest()
{
}

/*===============================================================*/
/*     Help Method                                               */
/*===============================================================*/
void FormRendererTest::PrintResult()
{
	std::cerr << std::endl << (const char *)fCurrentTestMethod << " :   " << (const char *)fReply.str() << std::endl;
}

void FormRendererTest::ConfigureField6()
{
	Anything config;
	WDModule *wdm = WDModule::FindWDModule("SecurityModule");
	if (wdm) {
		wdm->Init(config);
	}
	config["HTMLTemplateConfig"]["TemplateDir"] = "wd_test";
	Anything langMap;
	langMap["D"] = "Localized_D";
	config["HTMLTemplateConfig"]["LanguageDirMap"] = langMap;

	wdm = WDModule::FindWDModule("TemplatesCacheModule");
	if (wdm) {
		wdm->Init(config);
	}

	fConfig["Method"] 		= "GET";		// Try with POST, too
	fConfig["EncType"] 		= "my-encr";
	fConfig["Target"] 		= "my-target";
//	fConfig["Layout"] 		= "my-layout";
	fConfig["Action"]	 	= "my_action";
	fConfig["Params"]["0"]	= "my-param0";
	fConfig["Params"].Append("my-param1");
	fConfig["Params"]["2"]	= "my-param2";
	fConfig["Options"]["0"]	= "my-option0";
	fConfig["Options"].Append("my-option1");
	fConfig["Options"]["2"] = "my-option2";
	fConfig["TemplateName"]	= "nonExistingTemplate";

	fEnvironment["ACCEPT-LANGUAGE"]		= "language";
	fEnvironment["SCRIPT_NAME"]					= "script";
	fEnvironment["HTTP_USER_AGENT"]				= "agent";
	fEnvironment["HTTP_USER_AGENT"]["R3SSL"]	= "r3ssl";

	Anything tmpStore = fContext.GetTmpStore();
//	tmpStore["UseBaseURL"] = 123;					// es muss eine Zahl sein sonst wird UseBaseURL ignoriert
	tmpStore["LanguageKeyMap"] = "GE";
	tmpStore["ServicePrefix"] = "wd-Gateway";
	tmpStore["R3BaseAddress"] = "r3BaseAddress";
	tmpStore["BaseAddress"] = "baseAddress";
}

void FormRendererTest::TestCase0()
{
	fCurrentTestMethod = "Form-TestCase0";
	ConfigureField6();
	ROAnything	roConfig = fConfig;
	fFormRenderer.RenderAll(fReply, fContext, roConfig);
	// assert the result

	const char *expectedResult = _QUOTE_(<FORM ACTION="wd-Gateway?X=Encoder-Scrambler-Signer-Compressor-%7B%2Frole%20%22FRTestRole%22%2Faction%20%22my_action%22%2F%220%22%20%22my-param0%22%2FP1%20%22my-param1%22%2F%222%22%20%22my-param2%22%7D" METHOD="GET" ENCTYPE="my-encr" TARGET="my-target" 0="my-option0" my-option1 2="my-option2"><INPUT TYPE=HIDDEN NAME="X" VALUE="Encoder-Scrambler-Signer-Compressor-%7B%2Frole%20%22FRTestRole%22%2Faction%20%22my_action%22%2F%220%22%20%22my-param0%22%2FP1%20%22my-param1%22%2F%222%22%20%22my-param2%22%7D"></FORM>);

	assertEqual( expectedResult, fReply.str() );
	assertEqual( strlen(expectedResult), fReply.str().Length());

	for (long i = 0; i < fReply.str().Length(); i++) {
		assertEqual((long)expectedResult[i], (long)fReply.str()[i]);
	} // for
}
