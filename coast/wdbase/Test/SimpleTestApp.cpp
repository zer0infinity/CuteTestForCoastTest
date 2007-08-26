/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

//--- c-library modules used ---------------------------------------------------
#include <stdlib.h>

//--- standard modules used ----------------------------------------------------
#include "Anything.h"
#include "ServerUtils.h"
#include "Dbg.h"

//--- test modules used --------------------------------------------------------
#include "TString.h"
#include "TestLocation.h"
#include "TestLocList.h"
#include "TestResult.h"

//--- module under test --------------------------------------------------------
#include "AppBooterTest.h"

//--- interface include --------------------------------------------------------
#include "SimpleTestApp.h"

class MethodTracker
{
public:
	MethodTracker(const String &methodName, AppBooterTest *test);
	~MethodTracker();

	void Use();
protected:
	String fMethodName;
	AppBooterTest *fTest;
};

RegisterApplication(SimpleTestApp);

SimpleTestApp::SimpleTestApp()
	:	Application("SimpleTestApp"),
		fTest(0)
{
	StartTrace(SimpleTestApp.Ctor);
}

SimpleTestApp::SimpleTestApp(const char *AppName)
	:	Application(AppName),
		fTest(0)
{
	StartTrace(SimpleTestApp.Ctor);
}

SimpleTestApp::~SimpleTestApp()
{
	StartTrace(SimpleTestApp.Dtor);
}

int SimpleTestApp::DoGlobalInit(int argc, const char *argv[], const ROAnything config)
{
	StartTrace(SimpleTestApp.DoGlobalInit);
	MethodTracker track("DoGlobalInit", fTest);
	return Application::DoGlobalInit(argc, argv, config);
}

int SimpleTestApp::DoGlobalRun()
{
	StartTrace(SimpleTestApp.DoGlobalRun);
	MethodTracker track("DoGlobalRun", fTest);
	return Application::DoGlobalRun();
}

int SimpleTestApp::DoGlobalTerminate(int val)
{
	StartTrace(SimpleTestApp.DoGlobalTerminate);
	MethodTracker track("DoGlobalTerminate", fTest);
	return Application::DoGlobalTerminate(val);
}

int SimpleTestApp::DoInit()
{
	StartTrace(SimpleTestApp.DoInit);
	MethodTracker track("DoInit", fTest);
	return Application::DoInit();
}

int SimpleTestApp::DoRun()
{
	StartTrace(SimpleTestApp.DoRun);
	MethodTracker track("DoRun", fTest);
	return Application::DoRun();
}

int SimpleTestApp::DoTerminate(int val)
{
	StartTrace(SimpleTestApp.DoTerminate);
	MethodTracker track("DoTerminate", fTest);
	return Application::DoTerminate(val);
}

void SimpleTestApp::SetTest(AppBooterTest *test)
{
	StartTrace(Application.SetTest);
	fTest = test;
}

RegisterApplication(SimpleTestServer);

SimpleTestServer::SimpleTestServer()
	:	Server("SimpleTestServer"),
		fTest(0)
{
	StartTrace(SimpleTestServer.Ctor);
}

SimpleTestServer::SimpleTestServer(const char *AppName)
	:	Server(AppName),
		fTest(0)
{
	StartTrace(SimpleTestServer.Ctor);
}

SimpleTestServer::~SimpleTestServer()
{
	StartTrace(SimpleTestServer.Dtor);
}

int SimpleTestServer::DoGlobalInit(int argc, const char *argv[], const ROAnything config)
{
	StartTrace(SimpleTestServer.DoGlobalInit);
	MethodTracker track("DoGlobalInit", fTest);
	return Server::DoGlobalInit(argc, argv, config);
}

int SimpleTestServer::DoGlobalRun()
{
	StartTrace(SimpleTestServer.DoGlobalRun);
	MethodTracker track("DoGlobalRun", fTest);
	return Application::DoGlobalRun();
}

int SimpleTestServer::DoGlobalTerminate(int val)
{
	StartTrace(SimpleTestServer.DoGlobalTerminate);
	MethodTracker track("DoGlobalTerminate", fTest);
	return Server::DoGlobalTerminate(val);
}

int SimpleTestServer::DoInit()
{
	StartTrace(SimpleTestServer.DoInit);
	MethodTracker track("DoInit", fTest);
	return Server::DoInit();
}

int SimpleTestServer::DoRun()
{
	StartTrace(SimpleTestServer.DoRun);
	MethodTracker track("DoRun", fTest);
	return Application::DoRun();
}

int SimpleTestServer::DoTerminate(int val)
{
	StartTrace(SimpleTestServer.DoTerminate);
	MethodTracker track("DoTerminate", fTest);
	return Server::DoTerminate(val);
}

void SimpleTestServer::SetTest(AppBooterTest *test)
{
	StartTrace(Application.SetTest);
	fTest = test;
}

MethodTracker::MethodTracker(const String &methodName, AppBooterTest *test)
	:	fMethodName(methodName),
		fTest(test)
{
	String msg(fMethodName);
	msg << " entered";
	fTest->Method(msg);
}

MethodTracker::~MethodTracker()
{
	String msg(fMethodName);
	msg << " left";
	fTest->Method(msg);
}

void MethodTracker::Use()
{

}
