/*
 * Copyright (c) 2009, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

//--- interface include --------------------------------------------------------
#include "OracleModule.h"

//--- standard modules used ----------------------------------------------------
#include "SystemLog.h"
#include "Dbg.h"

//---- OracleModule ---------------------------------------------------------------
RegisterModule(OracleModule);

OracleModule::OracleModule(const char *name)
	: WDModule(name)
{
	StartTrace(OracleModule.OracleModule);
}

OracleModule::~OracleModule()
{
	StartTrace(OracleModule.~OracleModule);
	Finis();
}

bool OracleModule::Init(const ROAnything config)
{
	StartTrace(OracleModule.Init);
	ROAnything myCfg;
	if (config.LookupPath(myCfg, "OracleModule")) {
		TraceAny(myCfg, "OracleModuleConfig");
		// initialize ConnectionPool
		if ( !fpConnectionPool.get() ) {
			fpConnectionPool = ConnectionPoolPtr(new Coast::Oracle::ConnectionPool("OracleConnectionPool"));
		}
		ROAnything roaPoolConfig( myCfg["ConnectionPool"] );
		TraceAny(roaPoolConfig, "initializing ConnectionPool with config");
		fpConnectionPool->Init(roaPoolConfig);
	}
	return true;
}

Coast::Oracle::ConnectionPool *OracleModule::GetConnectionPool()
{
	StatTrace(OracleModule.GetConnectionPool, "poolptr: &" << (long)fpConnectionPool.get(), Storage::Current());
	return fpConnectionPool.get();
}

bool OracleModule::Finis()
{
	StartTrace(OracleModule.Finis);
	if ( fpConnectionPool.get() ) {
		Trace("de-initialize ConnectionPool");
		fpConnectionPool->Finis();
		fpConnectionPool.reset();
	}
	return true;
}
