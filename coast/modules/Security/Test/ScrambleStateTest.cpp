/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#include "ScrambleStateTest.h"
#include "TestSuite.h"
#include "FoundationTestTypes.h"
#include "SecurityModule.h"
#include "SystemFile.h"
#include "Tracer.h"

Test *ScrambleStateTest::suite() {
	TestSuite *testSuite = new TestSuite;

	ADD_CASE(testSuite, ScrambleStateTest, ScrambleUnscrambleTest);

	return testSuite;
}

void ScrambleStateTest::setUp() {
	std::iostream *Ios = Coast::System::OpenStream("ScrambleStateTest", "any");
	if (Ios) {
		fStdContextAny.Import((*Ios));
		delete Ios;
	}
}

void ScrambleStateTest::ScrambleUnscrambleTest() {
	Anything config;

	for (long i = 1; i < fStdContextAny.GetSize(); i++) {
		DoScrambleTest(fStdContextAny.SlotName(i));
	}

	config = fStdContextAny["Config4"];
	DoReferenceUncsramble(config);
}

void ScrambleStateTest::DoScrambleTest(const char *thename) {
	StartTrace(ScrambleStateTest.DoScrambleTest);
	Trace("configuring module: " << thename);
	SecurityModule secm(thename);
	Anything state(fStdContextAny["State1"]);
	String scrambled;
	Anything unscrambledState;

	t_assertm(secm.Init(fStdContextAny), NotNull(thename));

	SecurityModule::ScrambleState(scrambled, state);
	Trace("scrambled state:<" << scrambled << ">");
	t_assertm(SecurityModule::UnscrambleState(unscrambledState, scrambled), TString("expected unscrambling to be possible for config") << thename);

	assertAnyEqual(state, unscrambledState );
	//        secm.ResetFinis(fStdContextAny);
}
void ScrambleStateTest::DoReferenceUncsramble(const Anything &config) {
	WDModule *secm = WDModule::FindWDModule("SecurityModule");
	Anything expectedClearText;
	expectedClearText["role"] = "AdminLocalAdmin";
	expectedClearText["sessionId"] = "979555608_6648308115913975";
	expectedClearText["page"] = "AdminMainPage";
	expectedClearText["action"] = "GoReloadConfig";

	t_assert(secm != NULL);
	if (secm) {
		{
			String scrambled;
			scrambled = "Base64:Qmxvd2Zpc2hTY3JhbWJsZXItD2tpkCGRNDHj8cOapE29gBvuvdeXIO-6ZezbTzjfw3niedqXjJKK31MJSA1oCM"
				"TwMbSWfmK5mjIX0juyVXmqLpS1qi$H335EkLmzeWVkTsN-JNwM8E4q3Ha71EdGMLnfwFDPXB88PVJksah67Y1ksHavgZoVjuJ7C"
				"Mwm9GiC9RGXtZ5O-nQcRj14Mih2w0gw";
			Anything unscrambledState;
			secm->Init(config);
			SecurityModule::UnscrambleState(unscrambledState, scrambled);
			assertAnyEqual(expectedClearText, unscrambledState);
		}
		{
			String scrambled;
			scrambled = "Base64-Qmxvd2Zpc2hTY3JhbWJsZXItD2tpkCGRNDHj8cOapE29gBvuvdeXIO-6ZezbTzjfw3niedqXjJKK31MJSA1oCM"
				"TwMbSWfmK5mjIX0juyVXmqLpS1qi$H335EkLmzeWVkTsN-JNwM8E4q3Ha71EdGMLnfwFDPXB88PVJksah67Y1ksHavgZoVjuJ7C"
				"Mwm9GiC9RGXtZ5O-nQcRj14Mih2w0gw";
			Anything unscrambledState;
			secm->Init(config);
			SecurityModule::UnscrambleState(unscrambledState, scrambled);
			assertAnyEqual(expectedClearText, unscrambledState);
		}
	}
}
