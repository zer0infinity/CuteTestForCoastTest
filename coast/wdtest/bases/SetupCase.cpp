/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

//--- interface include --------------------------------------------------------
#include "SetupCase.h"

//--- test modules used --------------------------------------------------------
#include "TestSuite.h"

//--- standard modules used ----------------------------------------------------
#include "System.h"
#include "SysLog.h"
#include "AppBooter.h"
#include "Dbg.h"
#include "Server.h"

//--- c-library modules used ---------------------------------------------------

//---- SetupCase ----------------------------------------------------------------
SetupCase::SetupCase(TString tname) : ConfiguredTestCase(tname, "SetupTestConfig"), fServer(0)
{
	StartTrace(SetupCase.Ctor);

	// Config of application
	fMainConfig["WD_BOOTFILE"] = "Config";
	istream *ifp = System::OpenStream("Config", "any");
	if (ifp) {
		// Config file can be loaded
		delete ifp;
		ifp = NULL;
		if (!AppBooter().Boot(fMainConfig)) {
			SysLog::Error("configuration error in Config.any! exiting...");
			// force program abortion, using throw because there is no other way to exit from
			// a ctor
			throw;
		}
	}
	// additional configuration
	fTestConfig["WD_BOOTFILE"] = "TestConfig";
	ifp = System::OpenStream("TestConfig", "any");
	if (ifp) {
		// TestConfig file can be loaded
		delete ifp;
		ifp = NULL;
		if (!AppBooter().Boot(fTestConfig)) {
			SysLog::Error("configuration error in TestConfig.any! exiting...");
			// force program abortion, using throw because there is no other way to exit from
			// a ctor
			throw;
		}
	}
}

SetupCase::~SetupCase()
{
	StartTrace(SetupCase.Dtor);

	if (fServer) {
		fServer->Terminate(0);
		fServer = 0;
	}

	if (!fTestConfig.IsNull()) {
		WDModule::Terminate(fTestConfig);
	}

	if (!fMainConfig.IsNull()) {
		WDModule::Terminate(fMainConfig);
	}
}

void SetupCase::dummy ()
{
	if (fTestConfig.IsDefined("InitServer")) {
		String serverName = fTestConfig["InitServer"].AsString();
		fServer = Server::FindServer(serverName);
		t_assert ( fServer != 0 );
		if (fServer) {
			t_assert(fServer->Init() == 0);
		}
	}
}
// builds up a suite of testcases, add a line for each testmethod
Test *SetupCase::suite ()
{
	StartTrace(SetupCase.suite);
	TestSuite *testSuite = new TestSuite;

	ADD_CASE(testSuite, SetupCase, dummy);

	return testSuite;
} // suite
