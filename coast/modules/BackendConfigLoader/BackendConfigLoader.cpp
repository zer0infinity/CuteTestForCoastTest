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
#include "Registry.h"

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
	ROAnything BackendConfigLoaderConfig;

	if ( config.LookupPath(BackendConfigLoaderConfig, "BackendConfigLoaderModule") && BackendConfigLoaderConfig.IsDefined("BackendConfigDir") && BackendConfigLoaderConfig["BackendConfigDir"].GetSize() ) {
		TraceAny(BackendConfigLoaderConfig, "BackendConfigLoaderConfig:");
		SystemLog::WriteToStderr("\tReading Backend Configuration Files");
		String path = BackendConfigLoaderConfig["BackendConfigDir"].AsCharPtr();
		if (!System::IsAbsolutePath(path)) {
			String cwd;
			cwd = System::GetRootDir();
			path = cwd << "/" << path;
			System::ResolvePath(path);
		}
		Anything dirlist = System::DirFileList(path, "any");
		for (int i = 0 ; i < dirlist.GetSize(); i++) {
			SystemLog::WriteToStderr(".");
			String backendName = dirlist[i].AsString();
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

	RegisterBackends();

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

ROAnything BackendConfigLoaderModule::GetBackendList()
{
	StartTrace(BackendConfigLoaderModule.GetBackendList);
	Anything backendList;
	for (int i = 0; i < backendConfigurations.GetSize(); i++) {
		backendList.Append(backendConfigurations.SlotName(i));
	}
	TraceAny(backendList, "List of all Backends:");
	return backendList;
}

bool BackendConfigLoaderModule::RegisterBackend(String backendName)
{
	StartTrace(BackendConfigLoaderModule.RegisterBackend);
	Trace("Registering backend:" << backendName);
	Anything config;
	config["DataAccessImpls"]["DataAccessImpl"]["HTTPDAImpl"] = backendName;
	config["Input"]["Mapper"] = backendName;
	config["Output"]["Mapper"] = backendName;
	HierarchyInstaller hi("DataAccessImpl");
	bool ret = true;
	ret = RegisterableObject::Install(config["DataAccessImpls"], "DataAccessImpl", &hi) && ret;

	AliasInstaller ai1("ParameterMapper");
	ret = RegisterableObject::Install(config["Input"], "ParameterMapper", &ai1) && ret;

	AliasInstaller ai2("ResultMapper");
	ret = RegisterableObject::Install(config["Output"], "ResultMapper", &ai2) && ret;

	Server *server = (Server *)Registry::GetRegistry("Application")->Find("Server");
	if (backendConfigurations[backendName].IsDefined("VHost2ServiceMap")) {
		Anything namedObjectConfig = server->GetNamedObjectConfig().DeepClone();
		Anything backendVHosts = backendConfigurations[backendName]["VHost2ServiceMap"];
		for (int i = 0; i < backendVHosts.GetSize(); i++) {
			String key = backendVHosts.SlotName(i);
			namedObjectConfig["VHost2ServiceMap"][key] = backendVHosts[key];
		}
		TraceAny(namedObjectConfig, "namedObjectConfig:");
		server->SetConfig("Application", "Server", namedObjectConfig);
	}
	if (backendConfigurations[backendName].IsDefined("URIPrefix2ServiceMap")) {
		Anything namedObjectConfig = server->GetNamedObjectConfig().DeepClone();
		Anything uriPrefixes = backendConfigurations[backendName]["URIPrefix2ServiceMap"];
		for (int i = 0; i < uriPrefixes.GetSize(); i++) {
			String key = uriPrefixes.SlotName(i);
			namedObjectConfig["URIPrefix2ServiceMap"][key] = uriPrefixes[key];
		}
		TraceAny(namedObjectConfig, "namedObjectConfig:");
		server->SetConfig("Application", "Server", namedObjectConfig);
	}

	if (backendConfigurations[backendName].IsDefined("ServiceActionMapping")) {
		ConfNamedObject *role = (ConfNamedObject *)Registry::GetRegistry("Role")->Find("CGRole");
		Anything namedObjectConfig = role->GetNamedObjectConfig().DeepClone();
		Anything serviceActionMapping = backendConfigurations[backendName]["ServiceActionMapping"];
		for (int i = 0; i < serviceActionMapping.GetSize(); i++) {
			String key = serviceActionMapping.SlotName(i);
			namedObjectConfig["DefaultCustomerAction"]["SwitchRenderer"]["Case"][key] = serviceActionMapping[key];
		}
		TraceAny(namedObjectConfig, "namedObjectConfig:");
		role->SetConfig("Role", "CGRole", namedObjectConfig);
	}
	return ret;
}

bool BackendConfigLoaderModule::RegisterBackends()
{
	StartTrace(BackendConfigLoaderModule.RegisterBackends);
	ROAnything backendList = GetBackendList();
	TraceAny(backendList, "Backends to register:");
	bool ret = true;
	for (int i = 0; i < backendList.GetSize(); i++) {
		String backendName = backendList[i].AsString();
		ret = RegisterBackend(backendName) && ret;
	}
	return ret;
}
