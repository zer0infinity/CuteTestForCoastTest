/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

//--- interface include --------------------------------------------------------
#include "LDAPCachePolicyModule.h"

//--- standard modules used ----------------------------------------------------
#include "Dbg.h"
#include "System.h"
#include "SysLog.h"
#include "DataAccess.h"
#include "Action.h"

//--- c-modules used -----------------------------------------------------------

//---- LdapCachePolicyModule -----------------------------------------------------------
RegisterModule(LdapCachePolicyModule);

LdapCachePolicyModule *LdapCachePolicyModule::fgLdapCachePolicyModule = 0;

LdapCachePolicyModule::LdapCachePolicyModule(const char *name) : WDModule(name)
{
	StartTrace(LdapCachePolicyModule.LdapCachePolicyModule);
}

LdapCachePolicyModule::~LdapCachePolicyModule()
{
	StartTrace(LdapCachePolicyModule.~LdapCachePolicyModule);
}

bool LdapCachePolicyModule::Init(const Anything &config)
{
	StartTrace(LdapCachePolicyModule.Init);
	Anything ldapCachePolicyModuleConfig;
	config.LookupPath(ldapCachePolicyModuleConfig, "LdapCachePolicyModule");
	TraceAny(ldapCachePolicyModuleConfig, "LdapCachePolicyModule:");

	Anything dataAccesses(config["LdapCachePolicyModule"]["LdapDataAccess"]);
	Anything dataAccessActions(config["LdapCachePolicyModule"]["LdapDataAccessAction"]);
	if ( dataAccesses.GetSize() 		== 0 &&
		 dataAccessActions.GetSize()	== 0 ) {
		SysLog::WriteToStderr("\tLdapCachePolicyModule::Init can't read needed configuration data.\n");
		return false;
	}
	if ( InitialLoad(dataAccesses, LdapCachePolicyModule::dataaccess) 	== false ||
		 InitialLoad(dataAccessActions, LdapCachePolicyModule::action) 	== false ) {
		return false;
	}
	String failedDataAccesses;
	CheckContractIsFulfilled(failedDataAccesses, dataAccesses);
	CheckContractIsFulfilled(failedDataAccesses, dataAccessActions);
	if (failedDataAccesses.Length() != 0 ) {
		SysLog::WriteToStderr(String("\tLdapCachePolicyModule::LDAP Query: ") << failedDataAccesses <<
							  String(" returned no data.\n"));
		return false;
	}
	SysLog::WriteToStderr("\tLdapCachePolicyModule done\n");
	return true;
}

bool LdapCachePolicyModule::InitialLoad(const Anything &dataAccesses, LdapCachePolicyModule::EDataAccessType daType)
{
	StartTrace(LdapCachePolicyModule.InitialLoad);
	CacheHandler *cache = CacheHandler::Get();
	bool ret(true);
	if (cache) {
		LdapDataAccessLoader ldl;
		LdapActionLoader lal;
		Anything tmp;
		for (int i = 0; i < dataAccesses.GetSize(); ++i) {
			String toDo(dataAccesses[i].AsString());
			if ( daType == dataaccess ) {
				Trace("Loading ldl with: " << toDo);
				cache->Load("LdapGetter", toDo, &ldl);
			}
			if ( daType == action ) {
				Trace("Loading lal with: " << toDo);
				cache->Load("LdapGetter", toDo, &lal);
			}
		}
	} else {
		SysLog::WriteToStderr("\tLdapCachePolicyModule::InitialLoad: NoCacheHandlerFound\n");
		ret = false;
	}
	return ret;
}

bool LdapCachePolicyModule::CheckContractIsFulfilled(String &failedDataAccesses, const Anything &dataAccesses)
{
	StartTrace(LdapCachePolicyModule.CheckContractIsFulfilled);
	bool ret(true);
	for (int i = 0; i < dataAccesses.GetSize(); ++i) {
		String daToDo(dataAccesses[i].AsString());
		Trace("Checking with: " << daToDo);

		ROAnything result(LdapCacheGetter::GetAll(daToDo));
		// broke contract: specified LDAP-query must return data
		if (result.IsNull()) {
			failedDataAccesses.Append(daToDo);
			failedDataAccesses.Append(" ");
			ret = false;
		}
	}
	return ret;
}

bool LdapCachePolicyModule::Finis()
{
	StartTrace(LdapCachePolicyModule.Finis);

	SysLog::WriteToStderr("\tTerminating LdapCachePolicyModule done\n");
	return true;
}

//--- LdapDataAccessLoader -----------------------------------------------
LdapDataAccessLoader::LdapDataAccessLoader() { }
LdapDataAccessLoader::~LdapDataAccessLoader() { }
Anything LdapDataAccessLoader::Load(const char *ldapDa)
{
	StartTrace(LdapDataAccessLoader.Load);
	Anything theResult;
	if ( String(ldapDa).Length() ) {
		Context ctx;
		if ( DataAccess(ldapDa).StdExec(ctx) ) {
			String strResultSlot = ctx.Lookup("ResultMapper.DestinationSlot", "Mapper");
			ROAnything roaResult;
			if ( ctx.Lookup(strResultSlot, roaResult ) ) {
				TraceAny(roaResult, "Results for [" << ldapDa << "]");
				if ( roaResult["Info"]["LdapSearchFoundEntryButNoData"].AsLong() == 0 ) {
					theResult = roaResult["LDAPResult"].DeepClone();
				}
			}
		} else {
			String msg;
			msg << "\tLdapCachePolicyModule::Load Unable to exec LDAP query for: " << ldapDa << "\n";
			SysLog::WriteToStderr(msg);
		}
	}
	return theResult;
}

//--- LdapDataAccessLoader -----------------------------------------------
LdapActionLoader::LdapActionLoader() { }
LdapActionLoader::~LdapActionLoader() { }

Anything LdapActionLoader::Load(const char *ldapDaAction)
{
	StartTrace(LdapActionLoader.Load);
	Anything theResult;
	if ( String(ldapDaAction).Length() ) {
		Context ctx;
		String transition;
		Anything config;
		// Default constructs an action config containing the name of the LdapDataAccess to execute
		// This may be overridden by the action implementing the DataAccess(es).
		config[ldapDaAction]["DataAccess"] = ldapDaAction;
		if ( Action::ExecAction(transition, ctx, config) ) {
			String strResultSlot = ctx.Lookup("ResultMapper.DestinationSlot", "Mapper");
			ROAnything roaResult;
			if ( ctx.Lookup(strResultSlot, roaResult ) ) {
				TraceAny(roaResult, "Results for [" << ldapDaAction << "]");
				if ( roaResult["Info"]["LdapSearchFoundEntryButNoData"].AsLong() == 0 ) {
					theResult = roaResult["LDAPResult"].DeepClone();
				}
			}
		} else {
			String msg;
			msg << "\tLdapCachePolicyModule::Load Unable to exec LDAP query for: " << ldapDaAction << "\n";
			SysLog::WriteToStderr(msg);
		}
	}
	return theResult;
}

//---- LdapCacheGetter ---------------------------------------------------------
LdapCacheGetter::LdapCacheGetter(const String &dataAccess)
	: fDA(dataAccess)
{
	StartTrace("LdapCacheGetter.LdapCacheGetter");
}

LdapCacheGetter::~LdapCacheGetter()
{
	StartTrace(LdapCacheGetter.~LdapCacheGetter);
}

bool LdapCacheGetter::DoLookup(const char *key, ROAnything &result, char delim, char indexdelim) const
{
	StartTrace(LdapCacheGetter.DoLookup);

	return Get(result, fDA, key, delim, indexdelim);
}

ROAnything LdapCacheGetter::GetAll(const String &dataAccess)
{
	StartTrace1(LdapCacheGetter.GetAll, dataAccess);

	CacheHandler *cache = CacheHandler::Get();
	return cache->Get("LdapGetter", dataAccess);
}

bool LdapCacheGetter::Get(ROAnything &result, const String &dataAccess, const String &key, char sepS, char sepI)
{
	StartTrace1(LdapCacheGetter.Get, key);

	bool ret = GetAll(dataAccess).LookupPath(result, key, sepS, sepI);
	TraceAny(result, "Result:");
	return ret;
}
