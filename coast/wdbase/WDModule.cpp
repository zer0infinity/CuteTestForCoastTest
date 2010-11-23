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
#include "SystemLog.h"
#include "Registry.h"
#include "Dbg.h"

//--- c-library modules used ---------------------------------------------------

//--- WDModule iterators
class WDModuleCaller;

//:abstracting iteration over a module list calling some operation on each module
class WDModuleIterator
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

class ConfiguredWDMIterator: public WDModuleIterator
{
public:
	ConfiguredWDMIterator(WDModuleCaller *wdmc, const ROAnything roaModules, bool forward = true);
	virtual ~ConfiguredWDMIterator() { }

protected:
	virtual bool HasMore();
	virtual WDModule *Next();

	ROAnything fModules;
	long fSize;
	long fIndex;
};

class RegistryWDMIterator: public WDModuleIterator
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
class WDModuleCaller
{
public:
	WDModuleCaller(const ROAnything roaConfig);
	virtual ~WDModuleCaller() { }
	virtual bool Call(WDModule *wdm);
	virtual void SetModules(const ROAnything roaModules) {
		fModules = roaModules;
	}

protected:
	virtual void HandleError(WDModule *wdm);
	virtual bool CheckMandatory();
	virtual bool SetModuleName(WDModule *wdm);

	virtual bool DoCall(WDModule *wdm) = 0;
	virtual const char *CallName() = 0;

	ROAnything fModules;
	ROAnything fConfig;
	String fModuleName; // only set temporary
};

class WDInit : public WDModuleCaller
{
public:
	WDInit(const ROAnything roaConfig) : WDModuleCaller(roaConfig) { }
	~WDInit() { }

	virtual bool DoCall(WDModule *wdm);
	virtual const char *CallName() {
		return "Initialization";
	}
};

class WDTerminate : public WDModuleCaller
{
public:
	WDTerminate(const ROAnything roaConfig) : WDModuleCaller(roaConfig) { }
	~WDTerminate() { }

protected:
	virtual bool DoCall(WDModule *wdm);
	virtual const char *CallName() {
		return "\tTermination";
	}
	virtual bool DoCallInner(WDModule *wdm);
};

class WDResetInstall : public WDModuleCaller
{
public:
	WDResetInstall(const ROAnything roaConfig) : WDModuleCaller(roaConfig) { }
	~WDResetInstall() { }

	virtual bool DoCall(WDModule *wdm);
	virtual const char *CallName() {
		return "\tResetInitialization";
	}
};

class WDResetTerminate : public WDTerminate
{
public:
	WDResetTerminate(const ROAnything roaConfig) : WDTerminate(roaConfig) { }
	~WDResetTerminate() { }

	virtual bool DoCallInner(WDModule *wdm);
	virtual const char *CallName() {
		return "\tResetTermination";
	}
};

//---- WDModule -----------------------------------------------------------
RegCacheImpl(WDModule);	// FindWDModule()
//---- static API first ---------------------------------------------------

int WDModule::Install(const ROAnything roaConfig)
{
	StartTrace(WDModule.Install);
	ROAnything roaModules;
	WDInit wdi(roaConfig);
	if ( roaConfig.LookupPath(roaModules, "Modules") ) {
		return ConfiguredWDMIterator(&wdi, roaModules).DoForEach();
	}
	return RegistryWDMIterator(&wdi).DoForEach();
}

int WDModule::Terminate(const ROAnything roaConfig)
{
	StartTrace(WDModule.Terminate);
	SystemLog::WriteToStderr("\tTerminating modules:\n");
	ROAnything roaModules;
	WDTerminate wdt(roaConfig);

	if ( roaConfig.LookupPath(roaModules, "Modules") ) {
		return ConfiguredWDMIterator(&wdt, roaModules, false).DoForEach();
	}
	SystemLog::WriteToStderr("\t\tNO /Modules configured: Terminating ALL REGISTERED MODULES!\n");
	return RegistryWDMIterator(&wdt, false).DoForEach();
}

int WDModule::ResetInstall(const ROAnything roaConfig)
{
	StartTrace(WDModule.ResetInstall);
	SystemLog::WriteToStderr("\tInstalling modules after reset:\n");
	ROAnything roaModules;
	WDResetInstall wdt(roaConfig);
	int result = 0;
	if ( roaConfig.LookupPath(roaModules, "Modules") ) {
		result = ConfiguredWDMIterator(&wdt, roaModules).DoForEach();
	} else {
		SystemLog::WriteToStderr("\t\tNO /Modules configured: ResetInstal all registered modules\n");
		result = RegistryWDMIterator(&wdt).DoForEach();
	}
	SystemLog::WriteToStderr("\tInstallation of modules after reset: DONE\n");
	return result;
}

int WDModule::ResetTerminate(const ROAnything roaConfig)
{
	StartTrace(WDModule.ResetTerminate);
	SystemLog::WriteToStderr("\tTerminating modules for reset:\n");
	ROAnything roaModules;
	WDResetTerminate wdt(roaConfig);

	if ( roaConfig.LookupPath(roaModules, "Modules") ) {
		return ConfiguredWDMIterator(&wdt, roaModules, false).DoForEach();
	}
	SystemLog::WriteToStderr("\t\tNO /Modules configured: Terminating ALL REGISTERED MODULES for reset\n");
	return RegistryWDMIterator(&wdt, false).DoForEach();
}

int WDModule::Reset(const ROAnything roaOldconfig, const ROAnything roaConfig)
{
	StartTrace(WDModule.Reset);
	if ( ResetTerminate(roaOldconfig) == 0 ) {
		return ResetInstall(roaConfig);
	}
	return -1;
}

//--- object api ------------------------------------
WDModule::WDModule() : NotCloned("WDModule")
{
}

WDModule::WDModule(const char *name)
	: NotCloned(name)
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

bool WDModule::ResetInit(const ROAnything config)
{
	StartTrace(WDModule.ResetInit);
	return Init(config);
}

bool WDModule::ResetFinis(const ROAnything )
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
ConfiguredWDMIterator::ConfiguredWDMIterator(WDModuleCaller *wdmc, const ROAnything roaModules, bool forward)
	: WDModuleIterator(wdmc, forward)
	, fModules(roaModules)
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
	(fForward) ? ++fIndex : --fIndex;
	if ( !wdm ) {
		SystemLog::WriteToStderr(moduleName << " not found.\n");
	}
	return wdm;
}

//--- RegistryWDMIterator ---------
RegistryWDMIterator::RegistryWDMIterator(WDModuleCaller *wdmc, bool forward)
	: WDModuleIterator(wdmc, forward)
	, fIter(Registry::GetRegistry("WDModule"), forward)
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
WDModuleCaller::WDModuleCaller(const ROAnything roaConfig)
	: fConfig(roaConfig)
{}

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
		return fModules[fModuleName]["Mandatory"].AsBool(false);
	}
	return false;
}

void WDModuleCaller::HandleError(WDModule *wdm)
{
	// call failed, send a message for the poor config debugger
	if (wdm) {
		SystemLog::WriteToStderr(String(CallName()) << " of " << fModuleName << " failed\n");
	}
}
bool WDInit::DoCall(WDModule *wdm)
{
	return wdm->Init(fConfig);
}

bool WDTerminate::DoCall(WDModule *wdm)
{
	SystemLog::WriteToStderr("\tTerminating ");
	SystemLog::WriteToStderr(wdm->GetName());
	bool ret = DoCallInner(wdm);
	if ( !wdm->IsStatic() ) {
		Registry::GetRegistry("WDModule")->UnregisterRegisterableObject(fModuleName);
		delete wdm;
	}
	SystemLog::WriteToStderr( ( ret ? " done\n" : " failed\n" ) );
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
