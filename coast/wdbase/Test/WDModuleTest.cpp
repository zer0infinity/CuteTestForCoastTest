/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#include "WDModuleTest.h"
#include "WDModule.h"
#include "Registry.h"
#include "Policy.h"
#include "TestSuite.h"

class TestModuleTrue: public WDModule
{
public:
	TestModuleTrue() : WDModule("TestModuleTrue") {}
	virtual bool Init(const ROAnything config) {
		return true;
	}
	virtual bool Finis() {
		return true;
	}
	virtual bool ResetFinis(const ROAnything config) {
		return true;
	}
	virtual bool ResetInit(const ROAnything config) {
		return true;
	}
};

class TestModuleFalse: public WDModule
{

public:
	TestModuleFalse() : WDModule("TestModuleFalse") {}
	virtual bool Init(const ROAnything config) {
		return false;
	}
	virtual bool Finis() {
		return false;
	}
	virtual bool ResetFinis(const ROAnything config) {
		return false;
	}
	virtual bool ResetInit(const ROAnything config) {
		return false;
	}
};

//---- WDModuleTest ----------------------------------------------------------------
WDModuleTest::WDModuleTest(TString tname) : TestCaseType(tname)
{
}

WDModuleTest::~WDModuleTest()
{
}

void WDModuleTest::setUp ()
{
	// safe it for later restore
	fOrigWDModuleRegistry = Registry::RemoveRegistry("WDModule");
	Registry::MakeRegistry("WDModule");
	WDModule::ResetCache(true);
	WDModule::FindWDModule("dummy");
	WDModule::ResetCache(false);
}

void WDModuleTest::tearDown ()
{
	// reset the correct WDModule registry
	Anything regTable = Registry::GetRegTable();

	// safe it for later restore
	Registry *wdmoduleTestRegistry = Registry::RemoveRegistry("WDModule");
	regTable["WDModule"] = Anything(fOrigWDModuleRegistry);
	AliasTerminator at("WDModule");
	wdmoduleTestRegistry->Terminate(&at);
	delete wdmoduleTestRegistry;
	WDModule::ResetCache(true);
	WDModule::FindWDModule("dummy");
	WDModule::ResetCache(false);

}

void WDModuleTest::InstallTest()
{
	// set up the correct WDModule registry
	Registry *wdmoduleTestRegistry = Registry::GetRegistry("WDModule");
	t_assert(wdmoduleTestRegistry != 0);
	wdmoduleTestRegistry->RegisterRegisterableObject("TestModuleTrue", new (Coast::Storage::Global()) TestModuleTrue);
	t_assert(WDModule::Install(Anything()) == 0);

	wdmoduleTestRegistry->RegisterRegisterableObject("TestModuleFalse", new (Coast::Storage::Global()) TestModuleFalse);
	t_assert(WDModule::Install(Anything()) == 0); // since it is not mandatory

	// simple configuration
	Anything config;
	config["Modules"]["TestModuleTrue"] = "TestModuleTrue";
	config["Modules"]["TestModuleFalse"] = "TestModuleFalse";

	t_assertm(WDModule::Install(config) == 0, "expected installation to succeed, since it is not mandatory");

	config["Modules"]["TestModuleFalse"]["Mandatory"] = true;
	t_assertm(WDModule::Install(config) == -1, "expected installation to fail, since it is mandatory");
}

void WDModuleTest::Install2Test()
{
	// set up the correct WDModule registry
	Registry *wdmoduleTestRegistry = Registry::GetRegistry("WDModule");
	t_assert(wdmoduleTestRegistry != 0);
	wdmoduleTestRegistry->RegisterRegisterableObject("TestModuleTrue", new (Coast::Storage::Global()) TestModuleTrue);
	t_assert(WDModule::Install(Anything()) == 0);

	wdmoduleTestRegistry->RegisterRegisterableObject("TestModuleFalse", new (Coast::Storage::Global()) TestModuleFalse);
	t_assert(WDModule::Install(Anything()) == 0); // since it is not mandatory

	// simple configuration
	Anything config;
	config["Modules"]["TestModuleTrue"] = "TestModuleTrue";
	config["Modules"]["TestNotExistingModule"] = "TestNotExistingModule";
	config["Modules"]["TestModuleFalse"] = "TestModuleFalse";

	t_assertm(WDModule::Install(config) == -1, "expected installation to fail, since it has a non existing modules");

}

void WDModuleTest::TerminateTest()
{
	// set up the correct WDModule registry
	Registry *wdmoduleTestRegistry = Registry::GetRegistry("WDModule");
	t_assert(wdmoduleTestRegistry != 0);
	wdmoduleTestRegistry->RegisterRegisterableObject("TestModuleTrue", new (Coast::Storage::Global()) TestModuleTrue);
	t_assert(WDModule::Terminate(Anything()) == 0);
	t_assert(!wdmoduleTestRegistry->Find("TestModuleTrue"));

	wdmoduleTestRegistry->RegisterRegisterableObject("TestModuleFalse", new (Coast::Storage::Global()) TestModuleFalse);
	t_assert(WDModule::Terminate(Anything()) == 0); // since it is not mandatory
	t_assert(!wdmoduleTestRegistry->Find("TestModuleFalse"));

	// simple configuration
	Anything config;
	config["Modules"]["TestModuleTrue"] = "TestModuleTrue";
	config["Modules"]["TestModuleFalse"] = "TestModuleFalse";
	wdmoduleTestRegistry->RegisterRegisterableObject("TestModuleTrue", new (Coast::Storage::Global()) TestModuleTrue);
	wdmoduleTestRegistry->RegisterRegisterableObject("TestModuleFalse", new (Coast::Storage::Global()) TestModuleFalse);
	t_assertm(WDModule::Terminate(config) == 0, "expected to succeed, since it is not mandatory"); //

	config["Modules"]["TestModuleFalse"]["Mandatory"] = true;
	wdmoduleTestRegistry->RegisterRegisterableObject("TestModuleTrue", new (Coast::Storage::Global()) TestModuleTrue);
	wdmoduleTestRegistry->RegisterRegisterableObject("TestModuleFalse", new (Coast::Storage::Global()) TestModuleFalse);
	t_assertm(WDModule::Terminate(config) == -1, "expected to fail, since it is mandatory"); //
}

void WDModuleTest::ResetTest()
{
	// set up the correct WDModule registry
	Registry *wdmoduleTestRegistry = Registry::GetRegistry("WDModule");
	wdmoduleTestRegistry->RegisterRegisterableObject("TestModuleTrue", new (Coast::Storage::Global()) TestModuleTrue);
	t_assert(WDModule::Reset(Anything(), Anything()) == 0);
	t_assert(wdmoduleTestRegistry->Find("TestModuleTrue") == 0);

	wdmoduleTestRegistry->RegisterRegisterableObject("TestModuleFalse", new (Coast::Storage::Global()) TestModuleFalse);
	t_assert(WDModule::Reset(Anything(), Anything()) == 0); // since it is not mandatory
	t_assert(wdmoduleTestRegistry->Find("TestModuleFalse") == 0);

	Anything config;
	config["Modules"]["TestModuleFalse"] = "TestModuleFalse";
	config["Modules"]["TestModuleFalse"]["Mandatory"] = true;
	t_assertm(WDModule::Reset(config, config) == -1, "expected reset to fail, since it is mandatory");

}

void WDModuleTest::ResetWithDiffConfigsTest()
{
	// set up the correct WDModule registry
	Registry *wdmoduleTestRegistry = Registry::GetRegistry("WDModule");

	WDModule *testmodulold = new (Coast::Storage::Global()) TestModuleTrue;
	WDModule *testmodulnew = new (Coast::Storage::Global()) TestModuleTrue;
	testmodulold->MarkStatic(); // so it is not deleted in reset
	testmodulnew->MarkStatic(); // so it is not deleted in reset

	wdmoduleTestRegistry->RegisterRegisterableObject("TestModuleTrueOld", testmodulold);
	wdmoduleTestRegistry->RegisterRegisterableObject("TestModuleTrueNew", testmodulnew);

	Anything oldconfig;
	oldconfig["Modules"]["TestModuleTrueOld"] = "TestModuleTrueOld";
	Anything newconfig;
	newconfig["Modules"]["TestModuleTrueNew"] = "TestModuleTrueNew";

	t_assert(wdmoduleTestRegistry->Find("TestModuleTrueOld") != 0);
	t_assert(wdmoduleTestRegistry->Find("TestModuleTrueNew") != 0);

	t_assert(WDModule::Reset(oldconfig, newconfig) == 0);
	t_assert(wdmoduleTestRegistry->Find("TestModuleTrueOld") != 0);
	t_assert(wdmoduleTestRegistry->Find("TestModuleTrueNew") != 0);

	testmodulold->fStaticallyInitialized = false;
	testmodulnew->fStaticallyInitialized = false; // so it is deleted in terminate
}

Test *WDModuleTest::suite ()
{
	TestSuite *testSuite = new TestSuite;

	ADD_CASE(testSuite, WDModuleTest, InstallTest);
	ADD_CASE(testSuite, WDModuleTest, Install2Test);
	ADD_CASE(testSuite, WDModuleTest, TerminateTest);
	ADD_CASE(testSuite, WDModuleTest, ResetTest);
	ADD_CASE(testSuite, WDModuleTest, ResetWithDiffConfigsTest);

	return testSuite;

}
