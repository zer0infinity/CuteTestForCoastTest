
//--- interface include --------------------------------------------------------
#include "OracleModule.h"

//--- standard modules used ----------------------------------------------------
#include "O8ConnectionManager.h"
#include "SysLog.h"
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
		// initialize WorkerPools for the listed servers
		Trace("initializing O8ConnectionManager");
	}
	return true;
}

bool OracleModule::Finis()
{
	StartTrace(OracleModule.Finis);
	Trace("de-initialize O8ConnectionManager");
	return true;
}
