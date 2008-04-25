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

BackendConfigLoaderModule::BackendConfigLoaderModule(const char *name)
	: WDModule(name)

{
}

BackendConfigLoaderModule::~BackendConfigLoaderModule()
{
}

bool BackendConfigLoaderModule::Init(const Anything &config)
{
	bool retCode = true;
	Anything BackendConfigLoaderConfig;
	StartTrace(BackendConfigLoaderModule.Init);

	if ( config.LookupPath(BackendConfigLoaderConfig, "BackendConfigLoaderModule") && BackendConfigLoaderConfig.IsDefined("BackendConfigDir") && BackendConfigLoaderConfig["BackendConfigDir"].GetSize() ) {
		TraceAny(BackendConfigLoaderConfig, "BackendConfigLoaderConfig:");
		SysLog::WriteToStderr("\tReading Backend Configuration Files");
		Anything dirlist = System::DirFileList(BackendConfigLoaderConfig["BackendConfigDir"].AsCharPtr(), "any");
		for (int i = 0 ; i < dirlist.GetSize(); i++) {
			SysLog::WriteToStderr(dirlist[i].AsString());
		}
		for (long i = 0; i < 10; i++) {
			SysLog::WriteToStderr(".", 1);
		}
		if (true) {
			SysLog::WriteToStderr(" done\n");
		} else {
			SysLog::WriteToStderr(" failed\n");
		}
		/*		Anything servers(BackendConfigLoaderConfig["Servers"]);
				long numOfServers= servers.GetSize();
				String strLogDir, strRotateDir;
				strLogDir = BackendConfigLoaderConfig["LogDir"].AsCharPtr("log");
				strRotateDir = strLogDir;
				strRotateDir << System::Sep() << "rotate";
				if ( BackendConfigLoaderConfig.IsDefined("RotateDir") )
				{
					strRotateDir = BackendConfigLoaderConfig["RotateDir"].AsCharPtr();
				}
				Trace("raw LogDir [" << strLogDir << "]");
				Trace("raw RotateDir [" << strRotateDir << "]");
				// generate for each server its respective logchannels
				for(long i= 0; i < numOfServers; i++)
				{
					const char *servername= servers.SlotName(i);
					Server *s;
					if ( servername && (s= Server::FindServer(servername)) )
					{
						Anything config;
						config["Channels"] = servers[i];
						config["LogDir"] = strLogDir;
						config["RotateDir"] = strRotateDir;
						retCode= retCode && MakeChannels(servername, config);
					}
					SysLog::WriteToStderr(".", 1);
				}
				TraceAny(fLogConnections, "LogConnections: ");
				if (retCode && StartLogRotator(BackendConfigLoaderConfig["RotateTime"].AsCharPtr("24:00")))
				{
					fgBackendConfigLoaderModule= this;
					fROLogConnections= fLogConnections;
					SysLog::WriteToStderr(" done\n");
				}
				else
				{
					fgBackendConfigLoaderModule= 0L;
					SysLog::WriteToStderr(" failed\n");
				}
			}
			else
			{
				SysLog::WriteToStderr(" done\n");*/
	}
	return retCode;
}

bool BackendConfigLoaderModule::Finis()
{
	StartTrace(BackendConfigLoaderModule.Finis);

	return true;
}

