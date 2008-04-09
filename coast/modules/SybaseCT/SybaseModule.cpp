/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

//--- interface include --------------------------------------------------------
#include "SybaseModule.h"

//--- standard modules used ----------------------------------------------------
#include "SysLog.h"
#include "Dbg.h"
#include "SybCTnewDAImpl.h"

//---- SybaseModule ---------------------------------------------------------------
RegisterModule(SybaseModule);

SybaseModule::SybaseModule(const char *name)
	: WDModule(name)
	, fHasNewDAImpls(false)
{
	StartTrace(SybaseModule.SybaseModule);
}

SybaseModule::~SybaseModule()
{
	StartTrace(SybaseModule.~SybaseModule);
	Finis();
}

bool SybaseModule::Init(const ROAnything config)
{
	StartTrace(SybaseModule.Init);
	ROAnything myCfg;
	if (config.LookupPath(myCfg, "SybaseModule")) {
		TraceAny(myCfg, "SybaseModuleConfig");
		// initialize WorkerPools for the listed servers
		if ( myCfg.IsDefined("SybCTnewDAImpl") ) {
			// initialize SybCTnewDAImpls
			Trace("initializing SybCTnewDAImpl");
			fHasNewDAImpls = SybCTnewDAImpl::Init(config);
		}
	}
	return true;
}

bool SybaseModule::Finis()
{
	StartTrace(SybaseModule.Finis);
	if ( fHasNewDAImpls && SybCTnewDAImpl::fgInitialized ) {
		// de-initialize SybCTnewDAImpls
		SybCTnewDAImpl::Finis();
	}
	return true;
}
