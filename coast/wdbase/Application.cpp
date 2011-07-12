/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#include "Application.h"
#include "AppBooter.h"
#include "Registry.h"
#include "SystemLog.h"
#include "Tracer.h"
#if !defined(WIN32)
// used for dynamic load of libraries
#include <dlfcn.h>
#endif

// initializing static variables
Anything Application::fgConfig(Coast::Storage::Global());
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

int Application::Init()
{
	StartTrace(Application.Init);
	return DoInit();
}

// intialization - loads configuration based on instance's name
int Application::DoInit()
{
	StartTrace(Application.DoInit);
	return 0;
}

// doing the work
int Application::Run()
{
	StartTrace(Application.Run);
	return DoRun();
}

// doing the work
int Application::DoRun()
{
	StartTrace(Application.DoRun);
	SystemLog::WriteToStderr("That all I'm doing ;-)\n");
	return 0;
}

// termination
int Application::Terminate(int val)
{
	StartTrace(Application.Terminate);

	String m(50);
	m << "\tTerminating <" << fName << ">" << "\n";
	SystemLog::WriteToStderr(m);

	int iRetVal = DoTerminate(val);

	m.Trim(0);
	m << "\tTerminating <" << fName << "> done\n";
	SystemLog::WriteToStderr(m);

	return iRetVal;
}

int Application::DoTerminate(int val)
{
	StartTrace(Application.DoTerminate);
	return val;
}

// Looks up key in the instance's store then config and finally in the global config
bool Application::DoLookup(const char *key, ROAnything &result, char delim, char indexdelim) const
{
	StartTrace(Application.Lookup);

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
	LockUnlockEntry me(fgConfigMutex);
	fgConfig = theConfiguration; // SOP: will deepclone if needed
}

ROAnything Application::GetConfig()
{
	StartTrace(Application.GetConfig);

	if ( fgConfig.IsNull() ) {
		LockUnlockEntry me(fgConfigMutex);
		static bool bootonce = false;
		if ( !bootonce && fgConfig.IsNull() ) {
			SystemLog::Info("implicit Application booting");

			bootonce = true;
			AppBooter booter;
			Anything dummyargs;
			if (!booter.Boot(dummyargs)) {
				SystemLog::Error("Application booting failed");
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
int Application::GlobalInit(int argc, const char *argv[], const ROAnything config)
{
	StartTrace(Application.GlobalInit);
	int ret = DoGlobalInit(argc, argv, config);
	if ( ret == 0 ) {
		SYSINFO("Global init: succeeded");
	} else {
		SYSERROR("Global init: failed");
	}
	return (ret);
}

int Application::DoGlobalInit(int argc, const char *argv[], const ROAnything config)
{
	StartTrace(Application.DoGlobalInit);
	return Init();	// Call instance init
}

int Application::GlobalRun()
{
	StartTrace(Application.GlobalRun);
	return DoGlobalRun();
}

int Application::DoGlobalRun()
{
	StartTrace(Application.DoGlobalRun);
	SystemLog::WriteToStderr("=> Have fun :-)\n");
	return Run();
}

// GlobalTerminate: frees ressources shared among all instances
int Application::GlobalTerminate(int val)
{
	StartTrace(Application.GlobalTerminate);
	return DoGlobalTerminate(val);
}

int Application::DoGlobalTerminate(int val)
{
	StartTrace(Application.GlobalTerminate);
	return Terminate(val);
}

//---- registry api
RegCacheImpl(Application);	// FindApplication()
