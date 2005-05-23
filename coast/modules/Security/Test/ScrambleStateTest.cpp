/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

//--- interface include --------------------------------------------------------
#include "ScrambleStateTest.h"
//--- test modules used --------------------------------------------------------
#include "TestSuite.h"

//--- module under test --------------------------------------------------------
#include "SecurityModule.h"

//--- standard modules used ----------------------------------------------------
#include "System.h"
#include "Dbg.h"

//--- c-library modules used ---------------------------------------------------

//---- ScrambleStateTest ----------------------------------------------------------------
Test *ScrambleStateTest::suite ()
{
	TestSuite *testSuite = new TestSuite;

	ADD_CASE(testSuite, ScrambleStateTest, ScrambleUnscrambleTest);

	return testSuite;
}
ScrambleStateTest::ScrambleStateTest(TString tstrName) : TestCase(tstrName)
{
}

ScrambleStateTest::~ScrambleStateTest()
{
}

void ScrambleStateTest::setUp ()
{
	iostream *Ios = System::OpenStream("ScrambleStateTest", "any");
	if ( Ios ) {
		fStdContextAny.Import((*Ios));
		delete Ios;
	}
}

void ScrambleStateTest::ScrambleUnscrambleTest()
{
	Anything config;

	for (long i = 1; i < fStdContextAny.GetSize(); i++) {
		DoScrambleTest(fStdContextAny.SlotName(i));
	}

	config = fStdContextAny["Config4"];
	DoReferenceUncsramble(config);
}

void ScrambleStateTest::DoScrambleTest(const char *thename)
{
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
void ScrambleStateTest::DoReferenceUncsramble(const Anything &config)
{
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
			scrambled = "Base64:Qmxvd2Zpc2hTY3JhbWJsZXIwOnX96LrMgzOhTH-XCHibkAO5a11imS2vmrf9tpA0cj5UDtwg2-MxkkN5Y3Zl1y"
						"-izgLfLeaUZsKQQv9sX2N0KvDsfIhUg2jrS9MM9E$gGR1SS4hzofNBUaZlYDe$9NMYY$n3vOVm5QfkR2Oxq5ldvBKFMLxMU1mLU"
						"ri7T5IVi5ctwUuQc746rQv3x1AE5IZ5Zma-hywm6QVq";
			Anything unscrambledState;
			secm->Init(config);
			SecurityModule::UnscrambleState(unscrambledState, scrambled);
			assertAnyEqual(expectedClearText, unscrambledState);
		}
		{
			String scrambled;
			scrambled = "Base64-Qmxvd2Zpc2hTY3JhbWJsZXIwOnX96LrMgzOhTH-XCHibkAO5a11imS2vmrf9tpA0cj5UDtwg2-MxkkN5Y3Zl1y"
						"-izgLfLeaUZsKQQv9sX2N0KvDsfIhUg2jrS9MM9E$gGR1SS4hzofNBUaZlYDe$9NMYY$n3vOVm5QfkR2Oxq5ldvBKFMLxMU1mLU"
						"ri7T5IVi5ctwUuQc746rQv3x1AE5IZ5Zma-hywm6QVq";
			Anything unscrambledState;
			secm->Init(config);
			SecurityModule::UnscrambleState(unscrambledState, scrambled);
			assertAnyEqual(expectedClearText, unscrambledState);
		}
	}
}
