/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

//--- interface include --------------------------------------------------------
#include "WDModule.h"

//--- standard modules used ----------------------------------------------------
#include "SysLog.h"
#include "Registry.h"
#include "Dbg.h"

//--- c-library modules used ---------------------------------------------------

//--- WDModule iterators
class WDModuleCaller;

//:abstracting iteration over a module list calling some operation on each module
class EXPORTDECL_WDBASE WDModuleIterator
{
public:
	WDModuleIterator(WDModuleCaller *wdmc, bool forward = true) : fCaller(wdmc), fForward(forward) { }
	virtual ~WDModuleIterator() { }
	virtual int DoForEach();

protected:
	virtual bool HasMore() = 0;
	virtual WDModule *Next() = 0;

	WDModuleCaller *fCaller;
	bool fForward;
};

class EXPORTDECL_WDBASE ConfiguredWDMIterator: public WDModuleIterator
{
public:
	ConfiguredWDMIterator(WDModuleCaller *wdmc, const Anything &modules, bool forward = true);
	virtual ~ConfiguredWDMIterator() { }

protected:
	virtual bool HasMore();
	virtual WDModule *Next();

	Anything fModules;
	long fSize;
	long fIndex;
};

class EXPORTDECL_WDBASE RegistryWDMIterator: public WDModuleIterator
{
public:
	RegistryWDMIterator(WDModuleCaller *wdmc, bool forward = true);
	virtual ~RegistryWDMIterator() { }

protected:
	virtual bool HasMore();
	virtual WDModule *Next();

	RegistryIterator fIter;
};

//---- WDModuleCaller ----------------------------------------------

//:abstracting calling some operation on a module
class EXPORTDECL_WDBASE WDModuleCaller
{
public:
	WDModuleCaller(const Anything &config);
	virtual ~WDModuleCaller() { }
	virtual bool Call(WDModule *wdm);
	virtual void SetModules(const Anything &modules) {
		fModules = modules;
	}

protected:
	virtual void HandleError(WDModule *wdm);
	virtual bool CheckMandatory();
	virtual bool SetModuleName(WDModule *wdm);

	virtual bool DoCall(WDModule *wdm) = 0;
	virtual const char *CallName() = 0;

	Anything fModules;
	Anything fConfig;
	String fModuleName; // only set temporary
};

class EXPORTDECL_WDBASE WDInit : public WDModuleCaller
{
public:
	WDInit(const Anything &config) : WDModuleCaller(config) { }
	~WDInit() { }

	virtual bool DoCall(WDModule *wdm);
	virtual const char *CallName() {
		return "Initialization";
	}
};

class EXPORTDECL_WDBASE WDTerminate : public WDModuleCaller
{
public:
	WDTerminate(const Anything &config) : WDModuleCaller(config) { }
	~WDTerminate() { }

protected:
	virtual bool DoCall(WDModule *wdm);
	virtual const char *CallName() {
		return "\tTermination";
	}
	virtual bool DoCallInner(WDModule *wdm);
};

class EXPORTDECL_WDBASE WDResetInstall : public WDModuleCaller
{
public:
	WDResetInstall(const Anything &config) : WDModuleCaller(config) { }
	~WDResetInstall() { }

	virtual bool DoCall(WDModule *wdm);
	virtual const char *CallName() {
		return "\tResetInitialization";
	}
};

class EXPORTDECL_WDBASE WDResetTerminate : public WDTerminate
{
public:
	WDResetTerminate(const Anything &config) : WDTerminate(config) { }
	~WDResetTerminate() { }

	virtual bool DoCallInner(WDModule *wdm);
	virtual const char *CallName() {
		return "\tResetTermination";
	}
};

//---- WDModule -----------------------------------------------------------
RegCacheImpl(WDModule);	// FindWDModule()
//---- static API first ---------------------------------------------------

int WDModule::Install(const Anything &config)
{
	StartTrace(WDModule.Install);
	Anything modules;
	WDInit wdi(config);
	if ( config.LookupPath(modules, "Modules") ) {
		return ConfiguredWDMIterator(&wdi, modules).DoForEach();
	}
	return RegistryWDMIterator(&wdi).DoForEach();
}

int WDModule::Terminate(const Anything &config)
{
	StartTrace(WDModule.Terminate);
	SysLog::WriteToStderr("\tTerminating modules:\n");
	Anything modules;
	WDTerminate wdt(config);

	if ( config.LookupPath(modules, "Modules") ) {
		return ConfiguredWDMIterator(&wdt, modules, false).DoForEach();
	}
	SysLog::WriteToStderr("\t\tNO /Modules configured: Terminating ALL REGISTERED MODULES!\n");
	return RegistryWDMIterator(&wdt, false).DoForEach();
}

int WDModule::ResetInstall(const Anything &config)
{
	StartTrace(WDModule.ResetInstall);
	SysLog::WriteToStderr("\tInstalling modules after reset:\n");
	Anything modules;
	WDResetInstall wdt(config);
	int result = 0;
	if ( config.LookupPath(modules, "Modules") ) {
		result = ConfiguredWDMIterator(&wdt, modules).DoForEach();
	} else {
		SysLog::WriteToStderr("\t\tNO /Modules configured: ResetInstal all registered modules\n");
		result = RegistryWDMIterator(&wdt).DoForEach();
	}
	SysLog::WriteToStderr("\tInstallation of modules after reset: DONE\n");
	return result;
}

int WDModule::ResetTerminate(const Anything &config)
{
	StartTrace(WDModule.ResetTerminate);
	SysLog::WriteToStderr("\tTerminating modules for reset:\n");
	Anything modules;
	WDResetTerminate wdt(config);

	if ( config.LookupPath(modules, "Modules") ) {
		return ConfiguredWDMIterator(&wdt, modules, false).DoForEach();
	}
	SysLog::WriteToStderr("\t\tNO /Modules configured: Terminating ALL REGISTERED MODULES for reset\n");
	return RegistryWDMIterator(&wdt, false).DoForEach();
}

int WDModule::Reset(const Anything &oldconfig, const Anything &config)
{
	StartTrace(WDModule.Reset);
	SysLog::WriteToStderr("\tReset modules:\n");
	if (ResetTerminate(oldconfig) == 0) {
		return ResetInstall(config);
	}
	return -1;
}

//--- object api ------------------------------------
WDModule::WDModule() : NotCloned("WDModule")
{
}

WDModule::WDModule(const char *name) : NotCloned(name)
{
}

WDModule::~WDModule()
{
}

bool WDModule::StdFinis(const char *category, const char *title)
{
	StartTrace(WDModule.StdFinis);
	bool success;
	AliasTerminator at(category);
	success = RegisterableObject::Terminate(category, &at);
	return success;
}

bool WDModule::ResetInit(const Anything &config)
{
	StartTrace(WDModule.ResetInit);
	return Init(config);
}

bool WDModule::ResetFinis(const Anything &)
{
	StartTrace(WDModule.ResetFinis);
	return Finis();
}

//--- WDModuleIterators ------------------------------------------------
int WDModuleIterator::DoForEach()
{
	while ( HasMore() ) {
		if (!fCaller->Call(Next())) {
			return -1;
		}
	} // while
	return 0;
}

//--- ConfiguredWDMIterator
ConfiguredWDMIterator::ConfiguredWDMIterator(WDModuleCaller *wdmc, const Anything &modules, bool forward)
	: WDModuleIterator(wdmc, forward), fModules(modules)
{
	fCaller->SetModules(fModules);
	fSize = fModules.GetSize();
	fIndex = (fForward) ? 0 : fSize - 1;
}

bool ConfiguredWDMIterator::HasMore()
{
	return (( fForward ) ? (fIndex < fSize) : (fIndex >= 0));
}

WDModule *ConfiguredWDMIterator::Next()
{
	String moduleName(fModules[fIndex][0L].AsCharPtr("NoModule"));
	WDModule *wdm = WDModule::FindWDModule(moduleName);
	(fForward) ? fIndex++ : fIndex--;
	if ( !wdm ) {
		SysLog::WriteToStderr(moduleName << " not found.\n");
	}
	return wdm;
}

//--- RegistryWDMIterator ---------
RegistryWDMIterator::RegistryWDMIterator(WDModuleCaller *wdmc, bool forward)
	: WDModuleIterator(wdmc, forward), fIter(Registry::GetRegistry("WDModule"), forward)
{
}

bool RegistryWDMIterator::HasMore()
{
	return fIter.HasMore();
}

WDModule *RegistryWDMIterator::Next()
{
	String moduleName;
	return (WDModule *)fIter.Next(moduleName);
}

//--- WDModuleCallers ------------------------------------------------
WDModuleCaller::WDModuleCaller(const Anything &config)
	: fConfig(config)
{ }

bool WDModuleCaller::Call(WDModule *wdm)
{
	if (wdm && SetModuleName(wdm) && DoCall(wdm)) {
		return true;
	}

	HandleError(wdm);

	if (!wdm || CheckMandatory()) {
		return false;
	}
	return true;
}

bool WDModuleCaller::SetModuleName(WDModule *wdm)
{
	return wdm->GetName(fModuleName);
}

bool WDModuleCaller::CheckMandatory()
{
	if (fModuleName.Length() > 0) {
		return (fModules[fModuleName].IsDefined("Mandatory") && fModules[fModuleName]["Mandatory"].AsBool(true));
	}
	return false;
}

void WDModuleCaller::HandleError(WDModule *wdm)
{
	// call failed, send a message for the poor config debugger
	if (wdm) {
		SysLog::WriteToStderr(String(CallName()) << " of " << fModuleName << " failed\n");
	}
}
bool WDInit::DoCall(WDModule *wdm)
{
	return wdm->Init(fConfig);
}

bool WDTerminate::DoCall(WDModule *wdm)
{
	bool ret = DoCallInner(wdm);
	if ( !wdm->IsStatic() ) {
		Registry::GetRegistry("WDModule")->UnregisterRegisterableObject(fModuleName);
		delete wdm;
	}
	return ret;
}

bool WDTerminate::DoCallInner(WDModule *wdm)
{
	return wdm->Finis();
}

bool WDResetInstall::DoCall(WDModule *wdm)
{
	return wdm->ResetInit(fConfig);
}

bool WDResetTerminate::DoCallInner(WDModule *wdm)
{
	return wdm->ResetFinis(fConfig);
}
