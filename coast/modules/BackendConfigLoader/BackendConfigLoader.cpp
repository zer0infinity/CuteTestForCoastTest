/*
 * Copyright (c) 2008, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

//--- interface include --------------------------------------------------------
#include "BackendConfigLoader.h"

//--- project modules used -----------------------------------------------------
#include "Server.h"
#include "Renderer.h"

//--- standard modules used ----------------------------------------------------
#include "SystemLog.h"
#include "System.h"
#include "Dbg.h"

//--- c-library modules used ---------------------------------------------------
#if defined(WIN32)
#include <stdio.h>
#endif

//---- BackendConfigLoaderModule -----------------------------------------------------------
RegisterModule(BackendConfigLoaderModule);

BackendConfigLoaderModule *BackendConfigLoaderModule::fgBackendConfigLoaderModule = 0;
Anything BackendConfigLoaderModule::backendConfigurations = InitBackendConfigurations();

BackendConfigLoaderModule::BackendConfigLoaderModule(const char *name)
	: WDModule(name)

{
}

BackendConfigLoaderModule::~BackendConfigLoaderModule()
{
}

Anything BackendConfigLoaderModule::InitBackendConfigurations()
{
	Anything init;
	return init;
}

bool BackendConfigLoaderModule::Init(const Anything &config)
{
	StartTrace(BackendConfigLoaderModule.Init);
	bool retCode = true;
	Anything BackendConfigLoaderConfig;

	if ( config.LookupPath(BackendConfigLoaderConfig, "BackendConfigLoaderModule") && BackendConfigLoaderConfig.IsDefined("BackendConfigDir") && BackendConfigLoaderConfig["BackendConfigDir"].GetSize() ) {
		TraceAny(BackendConfigLoaderConfig, "BackendConfigLoaderConfig:");
		SysLog::WriteToStderr("\tReading Backend Configuration Files");
		String path = BackendConfigLoaderConfig["BackendConfigDir"].AsCharPtr();
		if (!System::IsAbsolutePath(path)) {
			String cwd;
			cwd = System::GetRootDir();
			path = cwd << "/" << path;
			System::ResolvePath(path);
		}
		Anything dirlist = System::DirFileList(path, "any");
		for (int i = 0 ; i < dirlist.GetSize(); i++) {
			SysLog::WriteToStderr(".");
			String backendName = dirlist[i].AsString();
			Anything backendConfig;
			if (!System::LoadConfigFile(backendConfig, backendName)) {
				retCode = false;
			}
			backendConfigurations[backendName] = backendConfig.DeepClone();
		}
		TraceAny(backendConfigurations, "Backend Configurations:")
	} else {
		retCode = false;
	}

	if (retCode) {
		fgBackendConfigLoaderModule = this;
		SysLog::WriteToStderr(" done\n");
	} else {
		fgBackendConfigLoaderModule = 0L;
		SysLog::WriteToStderr(" failed\n");
	}

	return retCode;
}

bool BackendConfigLoaderModule::Finis()
{
	StartTrace(BackendConfigLoaderModule.Finis);

	SysLog::WriteToStderr("\tTerminating BackendConfigLoader Module");
	fgBackendConfigLoaderModule = 0L;
	SysLog::WriteToStderr(" done\n");

	return true;
}

Anything BackendConfigLoaderModule::GetBackendConfig(String backendName)
{
	StartTrace(BackendConfigLoaderModule.GetBackendConfig);
	TraceAny(backendConfigurations, "Backend Configurations:")
	return backendConfigurations[backendName].DeepClone();
}

Anything BackendConfigLoaderModule::GetBackendConfig()
{
	return backendConfigurations.DeepClone();
}

Anything BackendConfigLoaderModule::GetBackendList()
{
	StartTrace(BackendConfigLoaderModule.GetBackendList);
	Anything backendList;
	for (int i = 0; i < backendConfigurations.GetSize(); i++) {
		backendList.Append(backendConfigurations.SlotName(i));
	}
	TraceAny(backendList, "List of all Backends:");
	return backendList.DeepClone();
}

void BackendConfigLoaderModule::GetBackendConfig(Anything &any, String backendName)
{
	StartTrace(BackendConfigLoaderModule.GetBackendConfig);
	TraceAny(backendConfigurations, "Backend Configurations:")
	any = backendConfigurations[backendName].DeepClone();
}

