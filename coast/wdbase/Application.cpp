/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

//--- interface include --------------------------------------------------------
#include "Application.h"

//--- standard modules used ----------------------------------------------------
#include "AppBooter.h"
#include "Registry.h"
#include "SysLog.h"
#include "Dbg.h"

//--- c-library modules used ---------------------------------------------------
#if !defined(WIN32)
// used for dynamic load of libraries
#include <dlfcn.h>
#endif

// initializing static variables
Anything Application::fgConfig(Storage::Global());
Mutex Application::fgConfigMutex("ApplicationConfig");

RegisterApplication(Application);

Application::Application(const char *appName)
	: HierarchConfNamed(appName)
{
	StartTrace1(Application.Ctor, "Name:<" << NotNull(appName) << ">");
}

Application::~Application()
{
	StartTrace(Application.Dtor);
}

// intialization - loads configuration based on instance's name
int Application::Init()
{
	StartTrace(Application.Init);

	CheckConfig("Application", true);
	TraceAny(fConfig, "Application config");

	return 0;
}

// doing the work
int Application::Run()
{
	StartTrace(Application.Run);
	SysLog::WriteToStderr("That all I'm doing ;-)\n");
	return 0;
}

// termination
int Application::Terminate(int val)
{
	StartTrace(Application.Terminate);

	SysLog::WriteToStderr(String("Terminating: <") << fName << ">\n");
	// Check if last instance has be terminated

	return val;
}

// Looks up key in the instance's store then config and finally in the global config
bool Application::DoLookup(const char *key, ROAnything &result, char delim, char indexdelim) const
{
	StartTrace(Application.Lookup);
	// lookup the application store
//	{
//		MutexEntry me((Mutex&)fStoreMutex);
//		ROAnything store(fStore);
//		if ( store.LookupPath(result, key, delim, indexdelim) )
//		{
//			return true;
//		}
//	}

	// have a lookup in the application config
	if ( HierarchConfNamed::DoLookup(key, result, delim, indexdelim) ) {
		return true;
	}

	// lookup the main process config
	ROAnything config(fgConfig);
	return config.LookupPath(result, key, delim, indexdelim);
}

void Application::InitializeGlobalConfig(Anything theConfiguration)
{
	StartTrace(Application.InitializeGlobalConfig);
	MutexEntry me(fgConfigMutex);
	me.Use();
	fgConfig = theConfiguration; // SOP: will deepclone if needed
}

ROAnything Application::GetConfig()
{
	StartTrace(Application.GetConfig);

	if ( fgConfig.IsNull() ) {
		MutexEntry me(fgConfigMutex);
		me.Use();
		static bool bootonce = false;
		if ( !bootonce && fgConfig.IsNull() ) {
			SysLog::Info("implicit Application booting");

			bootonce = true;
			AppBooter booter;
			Anything dummyargs;
			if (!booter.Boot(dummyargs)) {
				SysLog::Error("Application booting failed");
			}
		}
	}
	return fgConfig;
}

Application *Application::GetGlobalApplication(String &applicationName)
{
	StartTrace(Application.GetGlobalApplication);
	Application *application = 0;
	if (!fgConfig.IsNull()) {
		Anything applicationConf;

		if (fgConfig.LookupPath(applicationConf, "Application") ) {
			for (long i = 0, sz = applicationConf.GetSize() && !application; i < sz; ++i) {
				// iterate over the applicationname list
				applicationName = applicationConf[i].AsCharPtr(0);
				if ( applicationName.Length() > 0 ) {
					// return the first application object found by name
					application = Application::FindApplication(applicationName);
					break;
				}
			}
		} else {
			// if no application object is configured in the config any
			// return the first in the list
			RegistryIterator ri(Registry::GetRegistry("Application"), false);
			for ( ; ri.HasMore() && !application ; application = SafeCast(ri.Next(applicationName), Application));
		}
	}
	return application;
}

// GlobalInit: installs ressources shared among all instances
int Application::GlobalInit(int argc, char *argv[], const Anything &config)
{
	StartTrace(Application.GlobalInit);

	return Init();	// Call instance init
}

int Application::GlobalRun()
{
	StartTrace(Application.GlobalRun);
	return Run();
}

// GlobalTerminate: frees ressources shared among all instances
int Application::GlobalTerminate(int val)
{
	StartTrace(Application.GlobalTerminate);

	return Terminate(val);
}
//
//void Application::PutInStore(const char *key, const Anything &a)
//{
//	MutexEntry me(fStoreMutex);me.Use();
//	fStore[key]= a;
//}
//
//void Application::RemoveFromStore(const char *key)
//{
//	MutexEntry me(fStoreMutex);me.Use();
//	fStore.Remove(key);
//}
//
//---- registry api
RegCacheImpl(Application);	// FindApplication()
