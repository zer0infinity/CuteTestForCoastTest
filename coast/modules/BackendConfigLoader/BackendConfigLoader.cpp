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
#include "HierarchyInstallerWithConfig.h"
#include "DataAccessImpl.h"
#include "Mapper.h"
#include "ServiceHandler.h"
#include "Server.h"
#include "Renderer.h"
#include "CacheHandler.h"
#include "Registry.h"
#include "Role.h"

//--- standard modules used ----------------------------------------------------
#include "SystemLog.h"
#include "System.h"
#include "Dbg.h"
#include "Policy.h"

//--- c-library modules used ---------------------------------------------------
#if defined(WIN32)
#include <stdio.h>
#endif

//---- BackendConfigLoaderModule -----------------------------------------------------------
RegisterModule(BackendConfigLoaderModule);

BackendConfigLoaderModule *BackendConfigLoaderModule::fgBackendConfigLoaderModule = 0;
Anything BackendConfigLoaderModule::backendConfigurations = MetaThing(Storage::Global());

BackendConfigLoaderModule::BackendConfigLoaderModule(const char *name)
	: WDModule(name)
{
}

BackendConfigLoaderModule::~BackendConfigLoaderModule()
{
}

bool BackendConfigLoaderModule::Init(const ROAnything config)
{
	StartTrace(BackendConfigLoaderModule.Init);
	bool retCode = true;
	ROAnything roaModuleConfig, roaConfigDir;

	if ( config.LookupPath(roaModuleConfig, "BackendConfigLoaderModule") && roaModuleConfig.LookupPath(roaConfigDir, "BackendConfigDir") ) {
		TraceAny(roaModuleConfig, "BackendConfigLoaderConfig:");
		SystemLog::WriteToStderr("\tReading Backend Configuration Files");
		String path( roaConfigDir.AsString() );
		if (!System::IsAbsolutePath(path)) {
			String cwd( System::GetRootDir() );
			path = cwd << "/" << path;
			System::ResolvePath(path);
		}
		Anything dirlist = System::DirFileList(path, "any");
		for (int i = 0, sz=dirlist.GetSize() ; retCode && i < sz; ++i) {
			SystemLog::WriteToStderr(".");
			String backendName( dirlist[i].AsString() );
			Anything backendConfig;
			if (!System::LoadConfigFile(backendConfig, backendName)) {
				retCode = false;
			}
			backendConfigurations[backendName] = backendConfig;
		}
		TraceAny(backendConfigurations, "Backend Configurations:");
	} else {
		retCode = false;
	}

	retCode = retCode && RegisterBackends();

	if (retCode) {
		fgBackendConfigLoaderModule = this;
		SystemLog::WriteToStderr(" done\n");
	} else {
		fgBackendConfigLoaderModule = 0L;
		SystemLog::WriteToStderr(" failed\n");
	}
	return retCode;
}

bool BackendConfigLoaderModule::Finis()
{
	StartTrace(BackendConfigLoaderModule.Finis);

	SystemLog::WriteToStderr("\tTerminating BackendConfigLoader Module");
	fgBackendConfigLoaderModule = 0L;
	SystemLog::WriteToStderr(" done\n");

	return true;
}

ROAnything BackendConfigLoaderModule::GetBackendConfig(const String &backendName)
{
	StartTrace(BackendConfigLoaderModule.GetBackendConfig);
	TraceAny(backendConfigurations, "Backend Configurations:");
	return backendConfigurations[backendName];
}

ROAnything BackendConfigLoaderModule::GetBackendConfig()
{
	return backendConfigurations;
}

Anything BackendConfigLoaderModule::GetBackendList()
{
	StartTrace(BackendConfigLoaderModule.GetBackendList);
	Anything backendList;
	for (int i = 0, sz=backendConfigurations.GetSize(); i < sz; ++i) {
		backendList.Append(backendConfigurations.SlotName(i));
	}
	TraceAny(backendList, "List of all Backends:");
	return backendList;
}

bool BackendConfigLoaderModule::RegisterBackend(const String& backendName)
{
	StartTrace(BackendConfigLoaderModule.RegisterBackend);
	Trace("Registering backend:" << backendName);
	bool ret=true;
	ROAnything roaBackendConfig( GetBackendConfig(backendName) ), roaObjectConfig;
	if ( roaBackendConfig.LookupPath(roaObjectConfig, ParameterMapper::gpcConfigPath) ) {
		HierarchyInstallerWithConfig ip(ParameterMapper::gpcCategory, roaBackendConfig);
		ret = RegisterableObject::Install(roaObjectConfig, ParameterMapper::gpcCategory, &ip) && ret;
	}
	if ( roaBackendConfig.LookupPath(roaObjectConfig, ResultMapper::gpcConfigPath) ) {
		HierarchyInstallerWithConfig ip(ResultMapper::gpcCategory, roaBackendConfig);
		ret = RegisterableObject::Install(roaObjectConfig, ResultMapper::gpcCategory, &ip) && ret;
	}
	if ( roaBackendConfig.LookupPath(roaObjectConfig, DataAccessImpl::gpcConfigPath) ) {
		HierarchyInstallerWithConfig ip(DataAccessImpl::gpcCategory, roaBackendConfig);
		ret = RegisterableObject::Install(roaObjectConfig, DataAccessImpl::gpcCategory, &ip) && ret;
	}
	if ( roaBackendConfig.LookupPath(roaObjectConfig, ServiceHandler::gpcConfigPath) ) {
		// here we need to pass an unusual slot delim because we want a.b.c to be found as a single piece and not a path
		HierarchyInstallerWithConfig ip(ServiceHandler::gpcCategory, roaBackendConfig, '#');
		ret = RegisterableObject::Install(roaObjectConfig, ServiceHandler::gpcCategory, &ip) && ret;
	}
	return ret;
}

bool BackendConfigLoaderModule::RegisterBackends()
{
	StartTrace(BackendConfigLoaderModule.RegisterBackends);
	Anything backendList = GetBackendList();
	TraceAny(backendList, "Backends to register:");
	bool ret = true;
	for (int i = 0, sz=backendList.GetSize(); ret && i < sz; ++i) {
		ret = RegisterBackend(backendList[i].AsString()) && ret;
	}
	return ret;
}
