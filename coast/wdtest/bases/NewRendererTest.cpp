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
#include "System.h"
#include "Renderer.h"
#include "Server.h"
#include "Session.h"
#include "Page.h"
#include "Role.h"
#include "Dbg.h"

//---- NewRendererTest ----------------------------------------------------------------
NewRendererTest::NewRendererTest(TString tname)
	: TestCaseType(tname)
{
	StartTrace(NewRendererTest.NewRendererTest);
}

TString NewRendererTest::getConfigFileName()
{
	return "NewRendererTestConfig";
}

NewRendererTest::~NewRendererTest()
{
}

void NewRendererTest::setUp()
{
	t_assert(GetConfig().IsDefined("TestCases"));
	t_assertm( System::LoadConfigFile(fGlobalConfig, "Config", "any"), TString("expected Config.any to be readable!" ));
	t_assert(fGlobalConfig.IsDefined("Modules"));
	Application::InitializeGlobalConfig(fGlobalConfig);
	WDModule::Install(fGlobalConfig);
}

void NewRendererTest::tearDown()
{
	t_assert(fGlobalConfig.IsDefined("Modules"));
	WDModule::Terminate(fGlobalConfig);
	Application::InitializeGlobalConfig(Anything());
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

	Anything testCases = GetConfig()["TestCases"].DeepClone();
	long sz = testCases.GetSize();

	for (long i = 0 ; i < sz; i++ ) {
		TString slotToCheck = testCases.SlotName(i);
		TString message = "NewRendererTestConfig.any:0 at TestCases";
		if (slotToCheck.Length()) {
			message << "." << slotToCheck;
		} else {
			message << ":" << i;
		}

		Context c(GetConfig()["EnvForAllCases"].DeepClone(), testCases[i]["Env"], theServer, &theSession, theRole, thePage);
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
		// message << ":\n" << result;
		assertEqualm(expected, result, message);
	}
}

Test *NewRendererTest::suite ()
{
	TestSuite *testSuite = new TestSuite;
	ADD_CASE(testSuite, NewRendererTest, TestRenderers);
	return testSuite;
}
