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

int SimpleTestApp::GlobalInit(int argc, char *argv[], const ROAnything config)
{
	StartTrace(SimpleTestApp.GlobalInit);
	MethodTracker track("GlobalInit", fTest);
	return Application::GlobalInit(argc, argv, config);
}

int SimpleTestApp::GlobalRun()
{
	StartTrace(SimpleTestApp.GlobalRun);
	MethodTracker track("GlobalRun", fTest);
	return Application::GlobalRun();
}

int SimpleTestApp::GlobalTerminate(int val)
{
	StartTrace(SimpleTestApp.GlobalTerminate);
	MethodTracker track("GlobalTerminate", fTest);
	return Application::GlobalTerminate(val);
}

int SimpleTestApp::Init()
{
	StartTrace(SimpleTestApp.Init);
	MethodTracker track("Init", fTest);
	return Application::Init();
}

int SimpleTestApp::Run()
{
	StartTrace(SimpleTestApp.Run);
	MethodTracker track("Run", fTest);

	return Application::Run();
}

int SimpleTestApp::Terminate(int val)
{
	StartTrace(Application.Terminate);
	MethodTracker track("Terminate", fTest);
	return Application::Terminate(val);
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

int SimpleTestServer::GlobalInit(int argc, char *argv[], const ROAnything config)
{
	StartTrace(SimpleTestServer.GlobalInit);
	MethodTracker track("GlobalInit", fTest);
	return Server::GlobalInit(argc, argv, config);
}

int SimpleTestServer::GlobalRun()
{
	StartTrace(SimpleTestServer.GlobalRun);
	MethodTracker track("GlobalRun", fTest);
	return Application::GlobalRun();
}

int SimpleTestServer::GlobalTerminate(int val)
{
	StartTrace(SimpleTestServer.GlobalTerminate);
	MethodTracker track("GlobalTerminate", fTest);
	return Server::GlobalTerminate(val);
}

int SimpleTestServer::Init()
{
	StartTrace(SimpleTestServer.Init);
	MethodTracker track("Init", fTest);
	return Server::Init();
}

int SimpleTestServer::Run()
{
	StartTrace(SimpleTestServer.Run);
	MethodTracker track("Run", fTest);
	return Application::Run();
}

int SimpleTestServer::Terminate(int val)
{
	StartTrace(Application.Terminate);
	MethodTracker track("Terminate", fTest);
	return Server::Terminate(val);
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
