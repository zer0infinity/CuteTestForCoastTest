/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#include "LDAPDAICachePolicyModule.h"
#include "DataAccess.h"
#include "Action.h"
#include "Context.h"

//---- LDAPDAICachePolicyModule -----------------------------------------------------------
RegisterModule(LDAPDAICachePolicyModule);

LDAPDAICachePolicyModule::LDAPDAICachePolicyModule(const char *name) : WDModule(name)
{
	StartTrace(LDAPDAICachePolicyModule.LDAPDAICachePolicyModule);
}

LDAPDAICachePolicyModule::~LDAPDAICachePolicyModule()
{
	StartTrace(LDAPDAICachePolicyModule.~LDAPDAICachePolicyModule);
}

bool LDAPDAICachePolicyModule::Init(const ROAnything config)
{
	StartTrace(LDAPDAICachePolicyModule.Init);
	ROAnything LDAPDAICachePolicyModuleConfig;
	config.LookupPath(LDAPDAICachePolicyModuleConfig, "LDAPDAICachePolicyModule");
	TraceAny(LDAPDAICachePolicyModuleConfig, "LDAPDAICachePolicyModule:");

	ROAnything dataAccesses(config["LDAPDAICachePolicyModule"]["LDAPDAIDataAccess"]);
	ROAnything dataAccessActions(config["LDAPDAICachePolicyModule"]["LDAPDAIDataAccessAction"]);
	if ( dataAccesses.GetSize() == 0 && dataAccessActions.GetSize() == 0 ) {
		SystemLog::WriteToStderr("\tLDAPDAICachePolicyModule::Init can't read needed configuration data.\n");
		return false;
	}
	if ( InitialLoad(dataAccesses, LDAPDAICachePolicyModule::dataaccess, config.DeepClone()) 	== false ||
		 InitialLoad(dataAccessActions, LDAPDAICachePolicyModule::action, config.DeepClone()) 	== false ) {
		return false;
	}
	String failedDataAccesses;
	CheckContractIsFulfilled(failedDataAccesses, dataAccesses);
	CheckContractIsFulfilled(failedDataAccesses, dataAccessActions);
	if (failedDataAccesses.Length() != 0 ) {
		SystemLog::WriteToStderr(String("\tLDAPDAICachePolicyModule::LDAP Query: ") << failedDataAccesses <<
								 String(" returned no data.\n"));
		return false;
	}
	SystemLog::WriteToStderr("\tLDAPDAICachePolicyModule done\n");
	return true;
}

bool LDAPDAICachePolicyModule::InitialLoad(const ROAnything dataAccesses, LDAPDAICachePolicyModule::EDataAccessType daType, const Anything &config )
{
	StartTrace(LDAPDAICachePolicyModule.InitialLoad);
	LDAPDAIDataAcccessLoader ldl(config);
	LDAPDAIActionLoader lal(config);
	Anything tmp;
	for (int i = 0; i < dataAccesses.GetSize(); i++) {
		String toDo(dataAccesses[i].AsString());
		if ( daType == dataaccess ) {
			Trace("Loading ldl with: " << toDo);
			CacheHandler::instance().Load("LdapDAIGetter", toDo, &ldl);
		}
		if ( daType == action ) {
			Trace("Loading lal with: " << toDo);
			CacheHandler::instance().Load("LdapDAIGetter", toDo, &lal);
		}
	}
	return true;
}

bool LDAPDAICachePolicyModule::CheckContractIsFulfilled(String &failedDataAccesses, const ROAnything dataAccesses)
{
	StartTrace(LDAPDAICachePolicyModule.CheckContractIsFulfilled);
	bool ret(true);
	for (int i = 0; i < dataAccesses.GetSize(); i++) {
		String daToDo(dataAccesses[i].AsString());
		Trace("Checking with: " << daToDo);

		ROAnything result(LDAPDAICacheGetter::GetAll(daToDo));
		// broke contract: specified LDAP-query must return data
		if (result.IsNull()) {
			failedDataAccesses.Append(daToDo);
			failedDataAccesses.Append(" ");
			ret = false;
		}
	}
	return ret;
}

bool LDAPDAICachePolicyModule::Finis()
{
	StartTrace(LDAPDAICachePolicyModule.Finis);
	return true;
}

//--- LDAPDAIDataAcccessLoader -----------------------------------------------
LDAPDAIDataAcccessLoader::LDAPDAIDataAcccessLoader() { }
LDAPDAIDataAcccessLoader::LDAPDAIDataAcccessLoader(Anything config) : fConfig(config) { }
LDAPDAIDataAcccessLoader::~LDAPDAIDataAcccessLoader() { }
Anything LDAPDAIDataAcccessLoader::Load(const char *ldapDa)
{
	StartTrace1(LDAPDAIDataAcccessLoader.Load, "da name to execute [" << NotNull(ldapDa) << "]");
	Anything theResult(Coast::Storage::Global());
	Context ctx;
	Context::PushPopEntry<Anything> aEntry(ctx, "LdapLoader", fConfig);

	if ( String(ldapDa).Length() ) {
		DataAccess da(ldapDa);
		bool retCode = da.StdExec(ctx);
		Anything tmpStore(ctx.GetTmpStore());
		TraceAny(tmpStore, "da execution " << (retCode?"successful":"failed"));
		if (retCode && tmpStore["LDAPResult"][ldapDa]["NumberOfEntries"].AsLong() > 0) {
			theResult = tmpStore["LDAPResult"][ldapDa]["Entries"];
		} else {
			String msg;
			msg << "\tLDAPDAICachePolicyModule::Load Unable to exec LDAP query for: " << ldapDa << "\n";
			SystemLog::WriteToStderr(msg);
		}
	}

	TraceAny(theResult, "LDAP-Result for " << ldapDa << " cache.");
	return theResult;
}

//--- LDAPDAIDataAcccessLoader -----------------------------------------------
LDAPDAIActionLoader::LDAPDAIActionLoader() { }
LDAPDAIActionLoader::LDAPDAIActionLoader(Anything config) : fConfig(config) { }
LDAPDAIActionLoader::~LDAPDAIActionLoader() { }

Anything LDAPDAIActionLoader::Load(const char *ldapDaAction)
{
	StartTrace(LDAPDAIActionLoader.Load);
	Anything theResult(Coast::Storage::Global());
	Context ctx;
	Context::PushPopEntry<Anything> aEntry(ctx, "LdapLoader", fConfig);
	if ( String(ldapDaAction).Length() ) {
		Anything tmpStore = ctx.GetTmpStore();
		String transition;
		Anything config;
		// Default constructs an action config containing the name of the LDAPDAIDataAccess to execute
		// This may be overridden by the action implementing the DataAccess(es).
		config[ldapDaAction]["DataAccess"] = ldapDaAction;
		bool retCode = Action::ExecAction(transition, ctx, config);
		Trace("Result of DataAccess " << ldapDaAction << " is: " << retCode);
		// Called Action *must* return results in slot ["LDAPResult"][ldapDaAction]
		TraceAny(tmpStore["LDAPResult"][ldapDaAction], "Returned: " << ldapDaAction);
		if (retCode && tmpStore["LDAPResult"][ldapDaAction]["NumberOfEntries"].AsLong() > 0) {
			theResult = tmpStore["LDAPResult"][ldapDaAction]["Entries"];
			TraceAny(theResult, "Result of Load()");
		} else {
			String msg;
			msg << "\tLDAPDAICachePolicyModule::Load Unable to exec LDAP query for: " << ldapDaAction << "\n";
			SystemLog::WriteToStderr(msg);
		}
	}
	return (theResult);
}

LDAPDAICacheGetter::LDAPDAICacheGetter(const String &dataAccess) :
		fDA(dataAccess) {
	StartTrace("LDAPDAICacheGetter.LDAPDAICacheGetter");
}

LDAPDAICacheGetter::~LDAPDAICacheGetter() {
	StartTrace(LDAPDAICacheGetter.~LDAPDAICacheGetter);
}

bool LDAPDAICacheGetter::DoLookup(const char *key, ROAnything &result, char delim, char indexdelim) const {
	StartTrace(LDAPDAICacheGetter.DoLookup);

	return Get(result, fDA, key, delim, indexdelim);
}

ROAnything LDAPDAICacheGetter::GetAll(const String &dataAccess) {
	StartTrace1(LDAPDAICacheGetter.GetAll, dataAccess);
	return CacheHandler::instance().Get("LdapDAIGetter", dataAccess);
}

bool LDAPDAICacheGetter::Get(ROAnything &result, const String &dataAccess, const String &key, char sepS, char sepI) {
	StartTrace1(LDAPDAICacheGetter.Get, key);
	bool ret = GetAll(dataAccess).LookupPath(result, key, sepS, sepI);
	TraceAny(result, "Result:");
	return ret;
}
