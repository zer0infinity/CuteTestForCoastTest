/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

//--- interface include --------------------------------------------------------
#include "NewRendererTest.h"

//--- test modules used --------------------------------------------------------
#include "TestSuite.h"

//--- standard modules used ----------------------------------------------------
#include "Renderer.h"
#include "Server.h"
#include "Session.h"
#include "Page.h"
#include "Role.h"
#include "Dbg.h"

//---- NewRendererTest ----------------------------------------------------------------
NewRendererTest::NewRendererTest(TString tname)
	: ConfiguredTestCase(tname, "NewRendererTestConfig")
{
}

NewRendererTest::~NewRendererTest()
{
}

void NewRendererTest::setUp()
{
	ConfiguredTestCase::setUp();
	t_assert(fConfig.IsDefined("TestCases"));
	fGlobalConfig = LoadConfigFile("Config");
	t_assert(fGlobalConfig.IsDefined("Modules"));
	Application::InitializeGlobalConfig(fGlobalConfig);
	WDModule::Install(fGlobalConfig);
}

void NewRendererTest::tearDown()
{
	t_assert(fGlobalConfig.IsDefined("Modules"));
	WDModule::Terminate(fGlobalConfig);
	Application::InitializeGlobalConfig(Anything());
	ConfiguredTestCase::tearDown();
}

void NewRendererTest::TestRenderers()
{
	StartTrace(NewRendererTest.TestRenderers);

	Server *theServer = Server::FindServer("Server");
	t_assert(theServer != NULL);
	Context ctx;
	Session theSession("TestSession", ctx);
	Page *thePage = Page::FindPage("TestPage");
	t_assert(thePage != NULL);
	Role *theRole = Role::FindRole("TestRole");
	t_assert(theRole != NULL);
	theSession.Init("TestSession", ctx);

	Anything testCases = fConfig["TestCases"];
	long sz = testCases.GetSize();

	for (long i = 0 ; i < sz; i++ ) {
		TString slotToCheck = testCases.SlotName(i);
		TString message = "NewRendererTestConfig.any:0 at TestCases";
		if (slotToCheck.Length()) {
			message << "." << slotToCheck;
		} else {
			message << ":" << i;
		}

		Context c(fConfig["EnvForAllCases"], testCases[i]["Env"], theServer, &theSession, theRole, thePage);
		PutInStore(testCases[i]["TmpStore"], c.GetTmpStore());
		PutInStore(testCases[i]["SessionStore"], c.GetSessionStore());
		String result("[");
		String expected("[");
		expected << testCases[i]["Expected"].AsCharPtr("") << "]";
		{
			OStringStream reply(result);
			Renderer::Render(reply, c, testCases[i]["Renderer"]);
			reply.flush();
		}
		result.Append(']');
		assertEqualm(expected, result, message);
	}
}

Test *NewRendererTest::suite ()
// collect all test cases for the SocketStream
{
	TestSuite *testSuite = new TestSuite;

	testSuite->addTest (NEW_CASE(NewRendererTest, TestRenderers));

	return testSuite;
} // suite
